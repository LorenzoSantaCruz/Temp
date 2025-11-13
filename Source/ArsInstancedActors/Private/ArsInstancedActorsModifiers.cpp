// Copyright (c) 2024 Lorenzo Santa Cruz. All rights reserved.

#include "ArsInstancedActorsModifiers.h"
#include "ArsInstancedActorsDebug.h"
#include "ArsInstancedActorsIndex.h"
#include "ArsInstancedActorsIteration.h"
#include "ArsInstancedActorsManager.h"
#include "ArsInstancedActorsData.h"
#include "DrawDebugHelpers.h"
#include "Math/Box.h"
#include "Math/Sphere.h"


//-----------------------------------------------------------------------------
// UArsInstancedActorsModifierBase
//-----------------------------------------------------------------------------
void UArsInstancedActorsModifierBase::ModifyAllInstances(AArsInstancedActorsManager& Manager, FArsInstancedActorsIterationContext& IterationContext)
{
	Manager.ForEachInstance([this](const FArsInstancedActorsInstanceHandle& InstanceHandle, const FTransform& InstanceTransform, FArsInstancedActorsIterationContext& IterationContext)
	{
		return ModifyInstance(InstanceHandle, InstanceTransform, IterationContext);
	}, 
	IterationContext,
	/*Predicate*/InstanceTagsQuery.IsEmpty() ? TOptional<AArsInstancedActorsManager::FInstancedActorDataPredicateFunc>() : TOptional<AArsInstancedActorsManager::FInstancedActorDataPredicateFunc>([this](const UArsInstancedActorsData& InstancedActorData)
	{
		return InstancedActorData.GetCombinedTags().MatchesQuery(InstanceTagsQuery);
	}));
}

//-----------------------------------------------------------------------------
// URemoveArsInstancedActorsModifier
//-----------------------------------------------------------------------------
URemoveArsInstancedActorsModifier::URemoveArsInstancedActorsModifier()
{
	// Safe * more efficient to perform deletions before entity spawning
	bRequiresSpawnedEntities = false;
}

void URemoveArsInstancedActorsModifier::ModifyAllInstances(AArsInstancedActorsManager& Manager, FArsInstancedActorsIterationContext& IterationContext)
{
#if WITH_ARSINSTANCEDACTORS_DEBUG
	// Debug draw removed instances
	UE::ArsInstancedActors::Debug::DebugDrawAllInstanceLocations(UE::ArsInstancedActors::Debug::CVars::DebugModifiers, ELogVerbosity::Verbose, Manager, FColor::Red, /*LogOwner*/&Manager, "LogArsInstancedActors");
#endif

	if (InstanceTagsQuery.IsEmpty())
	{
		IterationContext.RemoveAllInstancesDeferred(Manager);
	}
	else
	{
		for (const TObjectPtr<UArsInstancedActorsData>& InstanceData : Manager.GetAllInstanceData())
		{
			check(IsValid(InstanceData));

			if (InstanceData->GetCombinedTags().MatchesQuery(InstanceTagsQuery))
			{
				IterationContext.RemoveAllInstancesDeferred(*InstanceData);
			}
		}
	}
}

bool URemoveArsInstancedActorsModifier::ModifyInstance(const FArsInstancedActorsInstanceHandle& InstanceHandle, const FTransform& InstanceTransform, FArsInstancedActorsIterationContext& IterationContext)
{
#if WITH_ARSINSTANCEDACTORS_DEBUG
	// Debug draw removed instances
	UE::ArsInstancedActors::Debug::DebugDrawLocation(UE::ArsInstancedActors::Debug::CVars::DebugModifiers, InstanceHandle.GetManager()->GetWorld(), /*LogOwner*/InstanceHandle.GetManager(), LogArsInstancedActors, ELogVerbosity::Verbose, InstanceTransform.GetLocation(), /*Size*/30.0f, FColor::Red, TEXT("%d"), InstanceHandle.GetIndex());
#endif

	IterationContext.RemoveInstanceDeferred(InstanceHandle);

	return true;
}
