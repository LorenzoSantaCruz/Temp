// Copyright Epic Games, Inc. All Rights Reserved.

#include "ArsInstancedActorsIndex.h"
#include "ArsInstancedActorsManager.h"
#include "ArsInstancedActorsData.h"


FArsInstancedActorsInstanceHandle::FArsInstancedActorsInstanceHandle(UArsInstancedActorsData& InInstancedActorData, FArsInstancedActorsInstanceIndex InIndex)
	: InstancedActorData(&InInstancedActorData)
	, Index(InIndex)
{
}

FString FArsInstancedActorsInstanceIndex::GetDebugName() const
{
	return FString::Printf(TEXT("%d"), Index);
}

bool FArsInstancedActorsInstanceHandle::IsValid() const 
{
	return InstancedActorData.IsValid() && Index.IsValid();
}

UArsInstancedActorsData& FArsInstancedActorsInstanceHandle::GetInstanceActorDataChecked() const 
{ 
	UArsInstancedActorsData* RawInstancedActorData = InstancedActorData.Get();
	check(RawInstancedActorData);
	return *InstancedActorData; 
}

AArsInstancedActorsManager* FArsInstancedActorsInstanceHandle::GetManager() const
{
	if (UArsInstancedActorsData* RawInstancedActorData = InstancedActorData.Get())
	{
		return RawInstancedActorData->GetManager();
	}

	return nullptr;
}

AArsInstancedActorsManager& FArsInstancedActorsInstanceHandle::GetManagerChecked() const
{
	return GetInstanceActorDataChecked().GetManagerChecked();
}

FString FArsInstancedActorsInstanceHandle::GetDebugName() const
{
	UArsInstancedActorsData* RawInstancedActorData = InstancedActorData.Get();
	return FString::Printf(TEXT("%s : %s"), RawInstancedActorData ? *RawInstancedActorData->GetDebugName() : TEXT("null"), *Index.GetDebugName());
}

uint32 GetTypeHash(const FArsInstancedActorsInstanceHandle& Handle)
{
	uint32 Hash = 0;
	if (UArsInstancedActorsData* RawInstancedActorData = Handle.InstancedActorData.Get())
	{
		Hash = GetTypeHash(RawInstancedActorData);
	}
	Hash = HashCombine(Hash, Handle.GetIndex());

	return Hash;
}
