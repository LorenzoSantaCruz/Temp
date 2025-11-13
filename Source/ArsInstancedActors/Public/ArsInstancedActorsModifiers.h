// Copyright (c) 2024 Lorenzo Santa Cruz. All rights reserved.

#pragma once

#include "ArsMechanicaAPI.h"

#include "UObject/Object.h"
#include "ArsInstancedActorsManager.h"
#include "ArsInstancedActorsData.h"
#include "ArsInstancedActorsSettingsTypes.h"
#include "ArsInstancedActorsSubsystem.h"
#include "ArsInstancedActorsModifiers.generated.h"



struct FArsInstancedActorsInstanceHandle;
struct FArsInstancedActorsIterationContext;

/**
 * Base class for 'modifier' operations to run against Instanced Actors within AArsInstancedActorsManager's
 * 
 * Used by UArsInstancedActorsModifierVolumeComponent's to modify instances within their volumes. 
 * 
 * Subclasses must implement at least ModifyInstance but can also override ModifyAllInstances to provide a 
 * whole-manager fast path.
 *
 * @see UArsInstancedActorsModifierVolumeComponent
 */
UCLASS(MinimalAPI, Abstract)
class ARSMECHANICA_API UArsInstancedActorsModifierBase : public UObject
{
	GENERATED_BODY()
public:

	/** Optional tag query to filter instances to modify */
	UPROPERTY(EditAnywhere, Category = ArsInstancedActors)
	FGameplayTagQuery InstanceTagsQuery;

	/**
	 * If true, this modifier will wait to be called on Managers only after Mass
	 * entities have been spawned for all instances.
	 *
	 * @see bRequiresSpawnedEntities
	 */
	bool DoesRequireSpawnedEntities() const { return bRequiresSpawnedEntities; }

	/** 
	 * Callback to modify all instances in Manager, providing a 'fast path' opportunity for modifiers to perform whole-manager operations.
	 * By default this simply calls ModifyInstance for all instances.
	 * 
	 * @param Manager			The whole manager to modify. If bRequiresSpawnedEntities = false, this Manager may or may not have spawned 
	 * 							entities yet. @see bRequiresSpawnedEntities
	 * @param InterationContext Provides useful functionality while iterating instances like safe instance deletion
	 * 
	 * @see AArsInstancedActorsManager::ForEachInstance
	 */
	virtual void ModifyAllInstances(AArsInstancedActorsManager& Manager, FArsInstancedActorsIterationContext& IterationContext);

	/** 
	 * Per-instance callback to modify single instances.
	 * 
	 * Called by ModifyAllInstances & ModifyAllInstancesInBounds in their default implementations.
	 *
	 * @param InstanceHandle	Handle to the instance to Modify.
	 * @param InstanceTransform If entities have been spawned, this will be taken from the Mass transform fragment, else from 
	 * 							UArsInstancedActorsData::InstanceTransforms.
	 * @param InterationContext Provides useful functionality while iterating instances like safe instance deletion.
	 * 
	 * @return Return true to continue modification of subsequent instances, false to break iteration.
	 */
	virtual bool ModifyInstance(const FArsInstancedActorsInstanceHandle& InstanceHandle, const FTransform& InstanceTransform, FArsInstancedActorsIterationContext& IterationContext) PURE_VIRTUAL(UArsInstancedActorsModifierBase::ModifyInstance, return false;)
	
	/** 
	 * Callback to modify all instances in Manager, whose location falls within Bounds. 
	 * Prior to entity spawning in BeginPlay, this iterates valid UArsInstancedActorsData::InstanceTransforms. Once entities have 
	 * spawned, UArsInstancedActorsData::Entities are iterated.
	 * 
	 * By default this simply calls ModifyInstance for all instances.
	 * 
	 * @param Bounds 			A world space FBox or FSphere to test instance locations against using Bounds.IsInside(InstanceLocation)
	 * @param Manager			The whole manager to modify. If bRequiresSpawnedEntities = false, this Manager may or may not have spawned entities yet. @see bRequiresSpawnedEntities
	 * @param InterationContext Provides useful functionality while iterating instances like safe instance deletion
	 * @see AArsInstancedActorsManager::ForEachInstance
	 */
	template<typename TBoundsType>
	void ModifyAllInstancesInBounds(const TBoundsType& Bounds, AArsInstancedActorsManager& Manager, FArsInstancedActorsIterationContext& IterationContext)
	{
		UE::ArsInstancedActors::EBoundsTestType ArsInstancedActorsDataBoundsTestType{UE::ArsInstancedActors::EBoundsTestType::Default};
		
		Manager.ForEachInstance([this, &Bounds, &ArsInstancedActorsDataBoundsTestType](const FArsInstancedActorsInstanceHandle& InstanceHandle, const FTransform& InstanceTransform, FArsInstancedActorsIterationContext& IterationContext)
		{
			if (UE::ArsInstancedActors::PassesBoundsTest(Bounds, ArsInstancedActorsDataBoundsTestType, InstanceHandle, InstanceTransform))
			{
				return ModifyInstance(InstanceHandle, InstanceTransform, IterationContext);
			}
			return true;
		}, 
		IterationContext,
		/*Predicate*/TOptional<AArsInstancedActorsManager::FInstancedActorDataPredicateFunc>([this, &ArsInstancedActorsDataBoundsTestType](const UArsInstancedActorsData& InstancedActorData)
		{			
			// Allow settings to stop modifiers affect this instance type.
			const FArsInstancedActorsSettings* Settings = InstancedActorData.GetSettingsPtr<const FArsInstancedActorsSettings>();
			if (Settings && Settings->bOverride_bIgnoreModifierVolumes && Settings->bIgnoreModifierVolumes)
			{
				return false;
			}

			if (InstanceTagsQuery.IsEmpty() || InstancedActorData.GetCombinedTags().MatchesQuery(InstanceTagsQuery))
			{
				// modify the EBoundsTestType for this set of Entities that are associated with this IAD, if the settings require it
				ArsInstancedActorsDataBoundsTestType = (Settings != nullptr && Settings->bModifierVolumeCheckFullyEnclosed)
					? UE::ArsInstancedActors::EBoundsTestType::Enclosed
					: UE::ArsInstancedActors::EBoundsTestType::Intersect;

				return true;
			}

			return false;
		}));
	}

protected:

	/**
	 * If true, this modifier will wait to be called on Managers only after Mass
	 * entities have been spawned for all instances.
	 * 
	 * If false, this modifier may be called on managers prior to entity spawning (i.e: Manager.HasSpawnedEntities() = false)
	 * where operations like RuntimeRemoveEntities are cheaper to run pre-entity spawning. However, as latently spawned UArsInstancedActorsModifierVolumeComponent's
	 * may be matched up with Managers that have already spawned entities, modifiers with bRequiresSpawnedEntities = false must also
	 * support execution on Managers with HasSpawnedEntities() = true and should check Managers state accordingly in Modify
	 * callbacks. Modification is tracked per manager though and is guaranteed to only run once per manager, so modifiers that are
	 * run pre-entity spawning will not try and run again post-spawn.
	 * 
	 * @see AArsInstancedActorsManager::TryRunPendingModifiers
	 */
	bool bRequiresSpawnedEntities = true;
};

/** 
 * Modifier which removes all affected instances using AArsInstancedActorsManager::RuntimeRemoveInstances for individual instances.
 * For whole-manager modification this simply destroys the Manager.
 */
UCLASS(MinimalAPI)
class ARSMECHANICA_API URemoveArsInstancedActorsModifier : public UArsInstancedActorsModifierBase
{
	GENERATED_BODY()
public:
	URemoveArsInstancedActorsModifier();

	virtual void ModifyAllInstances(AArsInstancedActorsManager& Manager, FArsInstancedActorsIterationContext& IterationContext) override;
	virtual bool ModifyInstance(const FArsInstancedActorsInstanceHandle& InstanceHandle, const FTransform& InstanceTransform, FArsInstancedActorsIterationContext& IterationContext) override;
};

