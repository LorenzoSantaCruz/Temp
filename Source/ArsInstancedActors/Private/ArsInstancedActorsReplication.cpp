// Copyright (c) 2024 Lorenzo Santa Cruz. All rights reserved.

#include "ArsInstancedActorsReplication.h"
#include "ArsInstancedActorsData.h"


void FArsInstancedActorsDeltaList::Initialize(UArsInstancedActorsData& InOwnerInstancedActorData)
{
	const uintptr_t ThisStart = (uintptr_t)this;
	const uintptr_t ThisEnd = ThisStart + sizeof(FArsInstancedActorsDeltaList);
	const uintptr_t OwnerStart = (uintptr_t)&InOwnerInstancedActorData;
	const uintptr_t OwnerEnd = OwnerStart + InOwnerInstancedActorData.GetClass()->GetStructureSize();

	if (OwnerStart <= ThisStart && ThisEnd <= OwnerEnd)
	{
		InstancedActorData = &InOwnerInstancedActorData;
	}
	else
	{
		static constexpr TCHAR MessageFormat[] = TEXT("InOwnerInstancedActorData is required to be the instance containing `this` as a member. Ignoring.");
		checkf(false, MessageFormat);
		UE_LOG(LogArsInstancedActors, Error, MessageFormat);
	}
}

FArsInstancedActorsDelta& FArsInstancedActorsDeltaList::FindOrAddInstanceDelta(FArsInstancedActorsInstanceIndex InstanceIndex)
{
	uint16& DeltaIndex = InstanceIndexToDeltaIndex.FindOrAdd(InstanceIndex, (uint16)INDEX_NONE);
	if (DeltaIndex == (uint16)INDEX_NONE)
	{
		const int32 NewDeltaIndex = InstanceDeltas.Emplace(InstanceIndex);
		checkf(NewDeltaIndex < std::numeric_limits<uint16>::max(), TEXT("Reach limit of supported deltas"));
		DeltaIndex = NewDeltaIndex;
	}

	check(InstanceDeltas.IsValidIndex(DeltaIndex));
	return InstanceDeltas[DeltaIndex];
}

void FArsInstancedActorsDeltaList::RemoveInstanceDelta(uint16 DeltaIndex)
{
	check(InstanceDeltas.IsValidIndex(DeltaIndex));
	FArsInstancedActorsDelta& InstanceDelta = InstanceDeltas[DeltaIndex];

	uint16* DeltaIndexPtr = InstanceIndexToDeltaIndex.Find(InstanceDelta.GetInstanceIndex());
	if (ensureMsgf(DeltaIndexPtr && *DeltaIndexPtr == DeltaIndex, TEXT("Expecting the instance to exist and match the delta index")))
	{
		InstanceIndexToDeltaIndex.Remove(InstanceDelta.GetInstanceIndex());

		InstanceDeltas.RemoveAtSwap(DeltaIndex);
		MarkArrayDirty();

		// Fix up swapped-in delta's index lookup
		if (InstanceDeltas.IsValidIndex(DeltaIndex))
		{
			FArsInstancedActorsDelta& SwappedInInstanceDelta = InstanceDeltas[DeltaIndex];
			uint16* SwappedInInstanceDeltaIndex = InstanceIndexToDeltaIndex.Find(SwappedInInstanceDelta.GetInstanceIndex());
			if (ensureMsgf(SwappedInInstanceDeltaIndex, TEXT("InstanceIndexToDeltaIndex and InstanceDeltas have gotten out of sync! Couldn't find delta index for swapped in instance delta %s (RemoveAtSwap'd index: %u)"), *SwappedInInstanceDelta.GetInstanceIndex().GetDebugName(), DeltaIndex))
			{
				*SwappedInInstanceDeltaIndex = DeltaIndex;
			}
		}
	}
}

bool FArsInstancedActorsDeltaList::NetDeltaSerialize(FNetDeltaSerializeInfo& NetDeltaParams)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FArsInstancedActorsDelta, FArsInstancedActorsDeltaList>(InstanceDeltas, NetDeltaParams, *this);
}

// @todo consider merging the data in these two call backs (also PostReplicatedChange()) see CallPostReplicatedReceiveOrNot()
void FArsInstancedActorsDeltaList::PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize)
{
	if (ensure(InstancedActorData) && AddedIndices.Num() > 0)
	{
		InstancedActorData->OnRep_InstanceDeltas(TConstArrayView<int32>(AddedIndices));
	}
}

void FArsInstancedActorsDeltaList::PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize)
{
	if (ensure(InstancedActorData) && ChangedIndices.Num() > 0)
	{
		InstancedActorData->OnRep_InstanceDeltas(TConstArrayView<int32>(ChangedIndices));
	}
}

void FArsInstancedActorsDeltaList::PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize)
{
	if (ensure(InstancedActorData) && RemovedIndices.Num() > 0)
	{
		InstancedActorData->OnRep_PreRemoveInstanceDeltas(TConstArrayView<int32>(RemovedIndices));
	}
}

void FArsInstancedActorsDeltaList::SetInstanceDestroyed(FArsInstancedActorsInstanceIndex InstanceIndex)
{
	FArsInstancedActorsDelta& InstanceDelta = FindOrAddInstanceDelta(InstanceIndex);
	if (ensure(!InstanceDelta.IsDestroyed()))
	{
		InstanceDelta.SetDestroyed(true);
		++NumDestroyedInstanceDeltas;
		MarkItemDirty(InstanceDelta);
	}
}

void FArsInstancedActorsDeltaList::RemoveDestroyedInstanceDelta(FArsInstancedActorsInstanceIndex InstanceIndex)
{
	uint16* DeltaIndexPtr = InstanceIndexToDeltaIndex.Find(InstanceIndex);
	if (DeltaIndexPtr != nullptr)
	{
		uint16 DeltaIndex = *DeltaIndexPtr;

		if (ensureMsgf(InstanceDeltas.IsValidIndex(DeltaIndex), TEXT("Expecting a valid delta index")))
		{
			FArsInstancedActorsDelta& InstanceDelta = InstanceDeltas[DeltaIndex];
			if (ensureMsgf(InstanceDelta.GetInstanceIndex() == InstanceIndex, TEXT("Expecting instance index to match")))
			{
				if (InstanceDelta.IsDestroyed())
				{
					InstanceDelta.SetDestroyed(false);

					if (!InstanceDelta.HasAnyDeltas())
					{
						RemoveInstanceDelta(DeltaIndex);
					}

					--NumDestroyedInstanceDeltas;
				}
			}
		}
	}
}

void FArsInstancedActorsDeltaList::SetCurrentLifecyclePhaseIndex(FArsInstancedActorsInstanceIndex InstanceIndex, uint8 InCurrentLifecyclePhaseIndex)
{
	FArsInstancedActorsDelta& InstanceDelta = FindOrAddInstanceDelta(InstanceIndex);
	if (InstanceDelta.GetCurrentLifecyclePhaseIndex() != InCurrentLifecyclePhaseIndex)
	{
		if (!InstanceDelta.HasCurrentLifecyclePhase())
		{
			++NumLifecyclePhaseDeltas;
		}
		InstanceDelta.SetCurrentLifecyclePhaseIndex(InCurrentLifecyclePhaseIndex);
		MarkItemDirty(InstanceDelta);
	}
}	

void FArsInstancedActorsDeltaList::RemoveLifecyclePhaseDelta(FArsInstancedActorsInstanceIndex InstanceIndex)
{
	uint16* DeltaIndexPtr = InstanceIndexToDeltaIndex.Find(InstanceIndex);
	if (DeltaIndexPtr != nullptr)
	{
		uint16 DeltaIndex = *DeltaIndexPtr;

		if (ensureMsgf(InstanceDeltas.IsValidIndex(DeltaIndex), TEXT("Expecting a valid delta index")))
		{
			FArsInstancedActorsDelta& InstanceDelta = InstanceDeltas[DeltaIndex];
			if (ensureMsgf(InstanceDelta.GetInstanceIndex() == InstanceIndex, TEXT("Expecting instance index to match")))
			{
				if (InstanceDelta.HasCurrentLifecyclePhase())
				{
					InstanceDelta.ResetLifecyclePhaseIndex();

					if (!InstanceDelta.HasAnyDeltas())
					{
						RemoveInstanceDelta(DeltaIndex);
					}
					else
					{
						MarkItemDirty(InstanceDelta);
					}

					--NumLifecyclePhaseDeltas;
				}
			}
		}
	}
}

#if WITH_SERVER_CODE
void FArsInstancedActorsDeltaList::SetCurrentLifecyclePhaseTimeElapsed(FArsInstancedActorsInstanceIndex InstanceIndex, FFloat16 InCurrentLifecyclePhaseTimeElapsed)
{
	FArsInstancedActorsDelta& InstanceDelta = FindOrAddInstanceDelta(InstanceIndex);
	if (InstanceDelta.GetCurrentLifecyclePhaseTimeElapsed() != InCurrentLifecyclePhaseTimeElapsed)
	{
		if (!InstanceDelta.HasCurrentLifecyclePhaseTimeElapsed())
		{
			++NumLifecyclePhaseTimeElapsedDeltas;
		}
		InstanceDelta.SetCurrentLifecyclePhaseTimeElapsed(InCurrentLifecyclePhaseTimeElapsed);
		// No need to MarkItemDirty(InstanceDelta) as this is not a replicated field
	}
}

void FArsInstancedActorsDeltaList::RemoveLifecyclePhaseTimeElapsedDelta(FArsInstancedActorsInstanceIndex InstanceIndex)
{
	uint16* DeltaIndexPtr = InstanceIndexToDeltaIndex.Find(InstanceIndex);
	if (DeltaIndexPtr != nullptr)
	{
		uint16 DeltaIndex = *DeltaIndexPtr;

		if (ensureMsgf(InstanceDeltas.IsValidIndex(DeltaIndex), TEXT("Expecting a valid delta index")))
		{
			FArsInstancedActorsDelta& InstanceDelta = InstanceDeltas[DeltaIndex];
			if (ensureMsgf(InstanceDelta.GetInstanceIndex() == InstanceIndex, TEXT("Expecting instance index to match")))
			{
				if (InstanceDelta.HasCurrentLifecyclePhaseTimeElapsed())
				{
					InstanceDelta.ResetLifecyclePhaseTimeElapsed();

					if (!InstanceDelta.HasAnyDeltas())
					{
						RemoveInstanceDelta(DeltaIndex);
					}

					--NumLifecyclePhaseTimeElapsedDeltas;
				}
			}
		}
	}
}
#endif // WITH_SERVER_CODE

void FArsInstancedActorsDeltaList::Reset(bool bMarkDirty)
{
	InstanceDeltas.Reset();
	InstanceIndexToDeltaIndex.Reset();

	NumDestroyedInstanceDeltas = 0;
	NumLifecyclePhaseDeltas = 0;
	NumLifecyclePhaseTimeElapsedDeltas = 0;

	if (bMarkDirty)
	{
		MarkArrayDirty();
	}
}
