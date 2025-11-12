// Copyright Epic Games, Inc. All Rights Reserved.

#include "ArsInstancedActorsIteration.h"
#include "ArsInstancedActorsIndex.h"
#include "ArsInstancedActorsManager.h"
#include "ArsInstancedActorsData.h"


FArsInstancedActorsIterationContext::~FArsInstancedActorsIterationContext()
{
	ensureMsgf(InstancesToRemove.IsEmpty(), TEXT("FArsInstancedActorsIterationContext destructing with dangling / un-flushed InstancesToRemove. FlushDeferredActions must be called once iteration completes, to perform deferred actions."));
	ensureMsgf(RemoveAllInstancesIAMs.IsEmpty(), TEXT("FArsInstancedActorsIterationContext destructing with dangling / un-flushed RemoveAllInstancesIAMs. FlushDeferredActions must be called once iteration completes, to perform deferred actions."));
}

void FArsInstancedActorsIterationContext::RemoveInstanceDeferred(const FArsInstancedActorsInstanceHandle& InstanceHandle)
{
	if (ensure(InstanceHandle.IsValid()))
	{
		InstancesToRemove.FindOrAdd(InstanceHandle.GetInstanceActorData()).Add(InstanceHandle.GetInstanceIndex());
	}
}

void FArsInstancedActorsIterationContext::RemoveAllInstancesDeferred(UArsInstancedActorsData& InstanceData)
{
	RemoveAllInstancesIADs.Add(&InstanceData);
}

void FArsInstancedActorsIterationContext::RemoveAllInstancesDeferred(AArsInstancedActorsManager& Manager)
{
	RemoveAllInstancesIAMs.Add(&Manager);
}

void FArsInstancedActorsIterationContext::FlushDeferredActions()
{
	for (auto& InstancesToRemoveItem : InstancesToRemove)
	{
		TObjectPtr<UArsInstancedActorsData> InstanceData = InstancesToRemoveItem.Key;
		if (ensure(IsValid(InstanceData)))
		{
			InstanceData->RuntimeRemoveInstances(MakeArrayView(InstancesToRemoveItem.Value));
		}
	}
	InstancesToRemove.Empty();

	for (TObjectPtr<UArsInstancedActorsData> InstanceData : RemoveAllInstancesIADs)
	{
		InstanceData->RuntimeRemoveAllInstances();
	}
	RemoveAllInstancesIADs.Empty();

	for (TObjectPtr<AArsInstancedActorsManager> Manager : RemoveAllInstancesIAMs)
	{
		Manager->RuntimeRemoveAllInstances();
	}
	RemoveAllInstancesIAMs.Empty();
}
