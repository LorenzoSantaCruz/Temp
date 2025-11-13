// Copyright (c) 2024 Lorenzo Santa Cruz. All rights reserved.

#pragma once

#include "ArsMechanicaAPI.h"

#include "ArsInstancedActorsIndex.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "ArsInstancedActorsReplication.generated.h"



struct FArsInstancedActorsDeltaList;
class UArsInstancedActorsData;

/** Per-instance delta's against the cooked instance data, for persistence and replication */
USTRUCT() 
struct FArsInstancedActorsDelta : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FArsInstancedActorsDelta(FArsInstancedActorsInstanceIndex InInstanceIndex)
		: InstanceIndex(InInstanceIndex)
	{}

	FArsInstancedActorsDelta() = default;

	/**
	 * Returns true if this delta actually contains any non-default 
	 * deltas / overrides to apply.
     */
	FORCEINLINE bool HasAnyDeltas() const
	{
		return IsDestroyed() 
			|| HasCurrentLifecyclePhase()
#if WITH_SERVER_CODE
			|| HasCurrentLifecyclePhaseTimeElapsed()
#endif
			;
	}

	FArsInstancedActorsInstanceIndex GetInstanceIndex() const { return InstanceIndex;  }

	bool IsDestroyed() const { return bDestroyed; }

	const bool HasCurrentLifecyclePhase() const { return CurrentLifecyclePhaseIndex != (uint8)INDEX_NONE; }
	uint8 GetCurrentLifecyclePhaseIndex() const { return CurrentLifecyclePhaseIndex; }

#if WITH_SERVER_CODE

	// mz@todo IA: move this section back
	const bool HasCurrentLifecyclePhaseTimeElapsed() const { return CurrentLifecyclePhaseTimeElapsed > 0.0f; }
	FFloat16 GetCurrentLifecyclePhaseTimeElapsed() const { return CurrentLifecyclePhaseTimeElapsed; }

#endif // WITH_SERVER_CODE

private:

	friend FArsInstancedActorsDeltaList;

	bool SetDestroyed(bool bInDestroyed) { return bDestroyed = bInDestroyed; }
	void SetCurrentLifecyclePhaseIndex(uint8 InCurrentLifecyclePhaseIndex) { CurrentLifecyclePhaseIndex = InCurrentLifecyclePhaseIndex; }
	void ResetLifecyclePhaseIndex() { CurrentLifecyclePhaseIndex = (uint8)INDEX_NONE; }

	UPROPERTY()
	FArsInstancedActorsInstanceIndex InstanceIndex;

	UPROPERTY()
	uint8 bDestroyed : 1 = false;

	UPROPERTY()
	uint8 CurrentLifecyclePhaseIndex = (uint8)INDEX_NONE;

#if WITH_SERVER_CODE
	void SetCurrentLifecyclePhaseTimeElapsed(FFloat16 InCurrentLifecyclePhaseTimeElapsed) { CurrentLifecyclePhaseTimeElapsed = InCurrentLifecyclePhaseTimeElapsed; }
	void ResetLifecyclePhaseTimeElapsed() { CurrentLifecyclePhaseTimeElapsed = -1.0f; }

	// Server-only (not replicated) time elapsed in current phase, saved & restored via persistence.
	// Unrequired by client code which only needs to know about discrete phase changes for visual updates.
	FFloat16 CurrentLifecyclePhaseTimeElapsed = -1.0f;
#endif // WITH_SERVER_CODE
};

USTRUCT()
struct FArsInstancedActorsDeltaList : public FFastArraySerializer
{
	GENERATED_BODY()

	void Initialize(UArsInstancedActorsData& InOwnerInstancedActorData);

	const TArray<FArsInstancedActorsDelta>& GetInstanceDeltas() const { return InstanceDeltas; }

	// Clear the InstanceDeltas list and resets InstancedActorData 
	// @param bMarkDirty If true, marks InstanceDeltas dirty for fast array replication
	void Reset(bool bMarkDirty = true);

	// Adds or modifies a FArsInstancedActorsDelta for InstanceIndex, marking the instance as destroyed and marks the 
	// delta as dirty for replication and application on clients.
	// This delta will also be persisted @see AArsInstancedActorsManager::SerializeInstancePersistenceData
	// Note: This does not request a persistence re-save
	void SetInstanceDestroyed(FArsInstancedActorsInstanceIndex InstanceIndex);

	void RemoveDestroyedInstanceDelta(FArsInstancedActorsInstanceIndex InstanceIndex);

	// Adds or modifies a FArsInstancedActorsDelta for InstanceIndex, specifying a new lifecycle phase to switch the instance
	// to, and marks the delta as dirty for replication and application on clients
	// Note: This does not request a persistence re-save
	void SetCurrentLifecyclePhaseIndex(FArsInstancedActorsInstanceIndex InstanceIndex, uint8 InCurrentLifecyclePhaseIndex);

	void RemoveLifecyclePhaseDelta(FArsInstancedActorsInstanceIndex InstanceIndex);

#if WITH_SERVER_CODE
	// Adds or modifies a FArsInstancedActorsDelta for InstanceIndex, specifying a new elapse time for the current lifecycle phase.
	// Note: This is a server-only delta and is NOT replicated to clients. It's simply stored in the delta list alongside the lifecycle 
	//		 phase index so they can be restored together from persistence and applied on the server in OnPersistentDataRestored -> ApplyInstanceDeltas
	void SetCurrentLifecyclePhaseTimeElapsed(FArsInstancedActorsInstanceIndex InstanceIndex, FFloat16 InCurrentLifecyclePhaseTimeElapsed);

	void RemoveLifecyclePhaseTimeElapsedDelta(FArsInstancedActorsInstanceIndex InstanceIndex);
#endif // WITH_SERVER_CODE

	const uint16 GetNumDestroyedInstanceDeltas() const { return NumDestroyedInstanceDeltas; }
	const uint16 GetNumLifecyclePhaseDeltas() const { return NumLifecyclePhaseDeltas; }
	const uint16 GetNumLifecyclePhaseTimeElapsedDeltas() const { return NumLifecyclePhaseTimeElapsedDeltas; }

	// UStruct overrides
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams);

	// FFastArraySerializer overrides
	void PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize);
	void PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize);

private:

	FArsInstancedActorsDelta& FindOrAddInstanceDelta(FArsInstancedActorsInstanceIndex InstanceIndex);
	void RemoveInstanceDelta(uint16 DeltaIndex);

	// Lookup the InstanceDeltas index from a FArsInstancedActorsInstanceIndex. 
	// Note: This is server only data, initialized in Initialize
	TMap<FArsInstancedActorsInstanceIndex, uint16> InstanceIndexToDeltaIndex;

	// Cached counts for persistence serialization
	uint16 NumDestroyedInstanceDeltas = 0;
	uint16 NumLifecyclePhaseDeltas = 0;
	uint16 NumLifecyclePhaseTimeElapsedDeltas = 0;

	UPROPERTY(Transient)
	TArray<FArsInstancedActorsDelta> InstanceDeltas; // FastArray of Instance replication data.

	// Raw pointer to the UArsInstancedActorsData this FArsInstancedActorsDeltaList instance is a member of
	UArsInstancedActorsData* InstancedActorData = nullptr;
};

template<>
struct TStructOpsTypeTraits< FArsInstancedActorsDeltaList > : public TStructOpsTypeTraitsBase2< FArsInstancedActorsDeltaList >
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

