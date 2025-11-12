// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ArsInstancedActorsDebug.h"
#include "ArsInstancedActorsManager.h"
#include "GameplayTagContainer.h"
#include "HierarchicalHashGrid2D.h"
#include "Subsystems/WorldSubsystem.h"
#include "StructUtils/SharedStruct.h"
#include "UObject/ObjectKey.h"
#include "ArsInstancedActorsSubsystem.generated.h"


class AArsInstancedActorsManager;
class UActorPartitionSubsystem;
class UDataRegistrySubsystem;
class UArsInstancedActorsModifierVolumeComponent;
class ULevel;
struct FArsInstancedActorsInstanceHandle;
struct FArsInstancedActorsManagerHandle;
struct FArsInstancedActorsModifierVolumeHandle;
class UArsInstancedActorsProjectSettings;

namespace UE::ArsInstancedActors
{
struct FExemplarActorData;
} // UE::ArsInstancedActors

/**
 * Instanced Actor subsystem used to spawn AArsInstancedActorsManager's and populate their instance data.
 * It also keeps track of all InstancedActorDatas and can be queried for them
 * @see AArsInstancedActorsManager
 */
UCLASS(MinimalAPI, BlueprintType)
class UArsInstancedActorsSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	ARSINSTANCEDACTORS_API UArsInstancedActorsSubsystem();

	ARSINSTANCEDACTORS_API static UArsInstancedActorsSubsystem* Get(UObject* WorldContextObject);

	ARSINSTANCEDACTORS_API static UArsInstancedActorsSubsystem& GetChecked(UObject* WorldContextObject);

#if WITH_EDITOR
	/** 
	* Adds an instance of ActorClass at InstanceTransform location by spawning or reusing a AArsInstancedActorsManager at InstanceTransform's grid cell location.
	* @param AdditionalInstanceTags Gameplay tags that will get appended to class-based FArsInstancedActorsSettings::GameplayTags
	* @see UArsInstancedActorsProjectSettings::GridSize
	*/	
	ARSINSTANCEDACTORS_API FArsInstancedActorsInstanceHandle InstanceActor(TSubclassOf<AActor> ActorClass, FTransform InstanceTransform, ULevel* Level
		, const FGameplayTagContainer& AdditionalInstanceTags = FGameplayTagContainer());

	/** 
	* Adds an instance of ActorClass at InstanceTransform location by spawning or reusing a AArsInstancedActorsManager at InstanceTransform's grid cell location.
	* * @param AdditionalInstanceTags Gameplay tags that will get appended to class-based FArsInstancedActorsSettings::GameplayTags
	* @see UArsInstancedActorsProjectSettings::GridSize
	*/	
	UFUNCTION(BlueprintCallable, Category = ArsInstancedActors)
	ARSINSTANCEDACTORS_API FArsInstancedActorsInstanceHandle InstanceActor(TSubclassOf<AActor> ActorClass, FTransform InstanceTransform, ULevel* Level
		, const FGameplayTagContainer& AdditionalInstanceTags, TSubclassOf<AArsInstancedActorsManager> ManagerClass);

	/**
	 * Removes all instance data for InstanceHandle.	
	 *
	 * This simply adds this instance to a FreeList which incurs extra cost to process at runtime before instance spawning. 
	 *
	 * @param InstanceHandle         Instance to remove
	 * @param bDestroyManagerIfEmpty If true, and InstanceHandle is the last instance in its manager, destroy the 
	 *                               now-empty manager. This implicitly clears the FreeList which is stored per-manager. 
	 *                               Any subsequent instance adds will then create a fresh manager.
	 * @see IA.CompactInstances console command
     */
	UFUNCTION(BlueprintCallable, Category = ArsInstancedActors)
	ARSINSTANCEDACTORS_API bool RemoveActorInstance(const FArsInstancedActorsInstanceHandle& InstanceHandle, bool bDestroyManagerIfEmpty = true);
#endif // WITH_EDITOR

	ARSINSTANCEDACTORS_API void ForEachManager(const FBox& QueryBounds, TFunctionRef<bool(AArsInstancedActorsManager&)> InOperation
		, TSubclassOf<AArsInstancedActorsManager> ManagerClass = AArsInstancedActorsManager::StaticClass()) const;
	ARSINSTANCEDACTORS_API void ForEachModifierVolume(const FBox& QueryBounds, TFunctionRef<bool(UArsInstancedActorsModifierVolumeComponent&)> InOperation) const;
	ARSINSTANCEDACTORS_API void ForEachInstance(const FBox& QueryBounds, TFunctionRef<bool(const FArsInstancedActorsInstanceHandle&
		, const FTransform&, FArsInstancedActorsIterationContext&)> InOperation) const;

	/** 
	 * Checks whether there are any instanced actors representing ActorClass or its subclasses inside QueryBounds.
	 * The check doesn't differentiate between hydrated and dehydrated actors (i.e. whether there's an actor instance 
	 * associated with the instance or not).
	 * @param bTestActorsIfSpawned if true then when an instance is found to overlap given bounds, and it has an actor
	 *	spawned associated with it, then the actor itself will be tested against the bounds for more precise test.
	 * @param AllowedLODs if provided will be used to filter out InstancedActorData that are at LOD not matching the flags in AllowedLODs
	 */
	ARSINSTANCEDACTORS_API bool HasInstancesOfClass(const FBox& QueryBounds, TSubclassOf<AActor> ActorClass, const bool bTestActorsIfSpawned = false
		, const EArsInstancedActorsBulkLODMask AllowedLODs = EArsInstancedActorsBulkLODMask::All) const;

	ARSINSTANCEDACTORS_API FArsInstancedActorsManagerHandle AddManager(AArsInstancedActorsManager& Manager);
	void RemoveManager(FArsInstancedActorsManagerHandle ManagerHandle);

	FArsInstancedActorsModifierVolumeHandle AddModifierVolume(UArsInstancedActorsModifierVolumeComponent& ModifierVolume);
	void RemoveModifierVolume(FArsInstancedActorsModifierVolumeHandle ModifierVolumeHandle);

	/** Adds ManagerHandle to PendingManagersToSpawnEntities for later processing in Tick -> ExecutePendingDeferredSpawnEntitiesRequests */
	void RequestDeferredSpawnEntities(FArsInstancedActorsManagerHandle ManagerHandle);

	/**
	 * Removes ManagerHandle from PendingManagersToSpawnEntities if present 
	 * @return	true if ManagerHandle was present in PendingManagersToSpawnEntities and subsequently removed. False otherwise (list is empty
	 *			or didn't contain ManagerHandle)
	 */
	bool CancelDeferredSpawnEntitiesRequest(FArsInstancedActorsManagerHandle ManagerHandle);

	/**
	 * Calls AArsInstancedActorsManager::InitializeModifyAndSpawnEntities for all pending managers in PendingManagersToSpawnEntities
	 * added via RequestDeferredSpawnEntities.
	 * @param	StopAfterSeconds	If < INFINITY, requests processing will stop after this time, leaving remaining requests for the next 
	 *								ExecutePendingDeferredSpawnEntitiesRequests to continue.
	 */
	bool ExecutePendingDeferredSpawnEntitiesRequests(double StopAfterSeconds = INFINITY);

	/** Return true if any deferred spawn entities requests are pending execution by the next ExecutePendingDeferredSpawnEntitiesRequests */
	ARSINSTANCEDACTORS_API bool HasPendingDeferredSpawnEntitiesRequests() const;

	/**
	 * Retrieves existing or spawns a new ActorClass for introspecting exemplary instance data.
	 *
	 * Actors are spawned into ExemplarActorWorld, a separated 'inactive' UWorld, to ensure no conflict or modifications in
	 * the main game world.
	 *
	 * These 'exemplar' actors are fully constructed, including BP construction scripts up to (but not including) BeginPlay.
	 */
	ARSINSTANCEDACTORS_API TSharedRef<UE::ArsInstancedActors::FExemplarActorData> GetOrCreateExemplarActor(TSubclassOf<AActor> ActorClass);
	
	/**
	 * Removes exemplar actor class from the map
	 */
	ARSINSTANCEDACTORS_API void UnregisterExemplarActorClass(TSubclassOf<AActor> ActorClass);

	/** 
	 * Compiles and caches finalized settings for ActorClass based off FArsInstancedActorsClassSettingsBase found in 
	 * UArsInstancedActorsProjectSettings::ActorClassSettingsRegistryType data registry, for ActorClass and it's 
	 * inherited super classes.
	 *
	 * Note: FArsInstancedActorsClassSettingsBase are indexed by class FName (*not* the full path) in the data registry, 
	 *       for quick lookup in CompileSettingsForActorClass. This means unique class names must be used for 
	 *       per-class settings.
	 */
	FSharedStruct GetOrCompileSettingsForActorClass(TSubclassOf<AActor> ActorClass);

	/**
	 * Returns true if ActorClass has a matching FArsInstancedActorsClassSettingsBase entry in 
	 * UArsInstancedActorsProjectSettings::ActorClassSettingsRegistryType data registry.
	 * Note: This relies on the registry being loaded at the time of calling i.e: in editor the registry must be
	 *       set to preload in editor.
	 * @param bIncludeSuperClasses	If true and ActorClass doesn't have a direct entry in ActorClassSettingsRegistryType,
	 * 								ActorClass's super classes will be successively tried instead.
	 */
	bool DoesActorClassHaveRegisteredSettings(TSubclassOf<AActor> ActorClass, bool bIncludeSuperClasses = true);

	/**
	 * Adds InstanceHandle to a list of pending instances that require an explicit representation update e.g: to remove/add ISMCs when a
	 * replicated actor is spawned / despawned. This ensures their representation is updated even if the are currently in a non-detailed
	 * bulk LOD.
	 * @see UArsInstancedActorsStationaryLODBatchProcessor
	 */
	void MarkInstanceRepresentationDirty(FArsInstancedActorsInstanceHandle InstanceHandle);

	/**
	 * Copies the current list of instances requiring explicit representation updates (added via MarkInstanceRepresentationDirty) to
	 * OutInstances and clears the internal list. This 'consumes' these pending instances with the assumption they will then receive a
	 * representation update by the calling code.
	 * @see MarkInstanceRepresentationDirty, UArsInstancedActorsStationaryLODBatchProcessor
	 */
	ARSINSTANCEDACTORS_API void PopAllDirtyRepresentationInstances(TArray<FArsInstancedActorsInstanceHandle>& OutInstances);

	ARSINSTANCEDACTORS_API virtual FArsInstancedActorsVisualizationDesc CreateVisualDescriptionFromActor(const AActor& ExemplarActor) const;
	/*
	* Called when an additional/alternate VisualizationDesc is registered. Override to make custom modifications to the visual representation
	*/
	ARSINSTANCEDACTORS_API virtual void ModifyVisualDescriptionForActor(const TNotNull<AActor*> ExemplarActor, FArsInstancedActorsVisualizationDesc& InOutVisualization) const {}

	//~ Begin UTickableWorldSubsystem Overrides
	ARSINSTANCEDACTORS_API virtual void Tick(float DeltaTime) override;
	ARSINSTANCEDACTORS_API virtual TStatId GetStatId() const override;
	//~ End UTickableWorldSubsystem Overrides

	//~ Begin USubsystem Overrides
	ARSINSTANCEDACTORS_API virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	ARSINSTANCEDACTORS_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	ARSINSTANCEDACTORS_API virtual void Deinitialize() override;
	//~ End USubsystem Overrides

	struct FNextTickSharedFragment
	{
		FSharedStruct SharedStruct;
		double NextTickTime = 0;

		bool operator<(const FNextTickSharedFragment& Other) const
		{
			return NextTickTime < Other.NextTickTime;
		}
	};

	TArray<UArsInstancedActorsSubsystem::FNextTickSharedFragment>& GetTickableSharedFragments();
	void UpdateAndResetTickTime(TConstStructView<FArsInstancedActorsDataSharedFragment> ArsInstancedActorsDataSharedFragment);

	TSubclassOf<AArsInstancedActorsManager> GetArsInstancedActorsManagerClass() const 
	{ 
		return ArsInstancedActorsManagerClass; 
	}

protected:
	/** 
	 * Fetches all registered FArsInstancedActorsDataSharedFragment from the EntityManager and adds the missing ones to SortedSharedFragments
	 * @param ArsInstancedActorsDataSharedFragment optionally the function can check if given shared fragment is amongst 
	 *	the newly added fragments
	 * @param returns whether ArsInstancedActorsDataSharedFragment has been found, or `true` if that param is not provided. 
	 */
	bool RegisterNewSharedFragmentsInternal(TConstStructView<FArsInstancedActorsDataSharedFragment> ArsInstancedActorsDataSharedFragment = TConstStructView<FArsInstancedActorsDataSharedFragment>());

	/** The container storing a sorted queue of FSharedStruct instances, ordered by the NextTickTime */
	TArray<FNextTickSharedFragment> SortedSharedFragments;

	TSharedPtr<FMassEntityManager> EntityManager;

	UPROPERTY(Transient)
	TObjectPtr<const UArsInstancedActorsProjectSettings> ProjectSettings;

	UPROPERTY(Transient)
	TObjectPtr<const UDataRegistrySubsystem> DataRegistrySubsystem;

	UPROPERTY(Transient)
	TObjectPtr<UActorPartitionSubsystem> ActorPartitionSubsystem;

	UPROPERTY()
	TSubclassOf<AArsInstancedActorsManager> ArsInstancedActorsManagerClass;

	// Spatially indexed managers. TSparseArray used for stable indices which can be spatially indexed by THierarchicalHashGrid2D
	// @todo Managers should be indexable by cell coord hashes within their levels, we could leverage this for more efficient spatial
	//       indexing by keeping a cellcoord hash map per tile
	TSparseArray<TWeakObjectPtr<AArsInstancedActorsManager>> Managers;
	using FManagersHashGridType = THierarchicalHashGrid2D</*Levels*/3, /*LevelRatio*/4, /*ItemIDType*/FArsInstancedActorsManagerHandle>;
	FManagersHashGridType ManagersHashGrid;

	// Spatially indexed modifier volumes. TSparseArray used for stable indices which can be spatially indexed by THierarchicalHashGrid2D
	TSparseArray<TWeakObjectPtr<UArsInstancedActorsModifierVolumeComponent>> ModifierVolumes;
	using FModifierVolumesHashGridType = THierarchicalHashGrid2D</*Levels*/3, /*LevelRatio*/4, /*ItemIDType*/FArsInstancedActorsModifierVolumeHandle>;
	FModifierVolumesHashGridType ModifierVolumesHashGrid;

	// FIFO queue of Managers pending deferred entity spawning in Tick. Enqueued in RequestDeferredSpawnEntities
	TArray<FArsInstancedActorsManagerHandle> PendingManagersToSpawnEntities;

	// Instances whose representation is explicitly dirty, e.g: due to actor spawn / despawn replication, requiring immediate representation 
	// processing even out of 'detailed' representation processing range.
	TArray<FArsInstancedActorsInstanceHandle> DirtyRepresentationInstances;

#if WITH_ARSINSTANCEDACTORS_DEBUG
	TMap<TObjectKey<AArsInstancedActorsManager>, FBox> DebugManagerBounds;
	TMap<TObjectKey<UArsInstancedActorsModifierVolumeComponent>, FBox> DebugModifierVolumeBounds;
#endif

	// Cached finalized / flattened FArsInstancedActorsClassSettingsBase for GetOrCompileSettingsForActorClass requested ActorClass.
	// Built via CompileSettingsForActorClass and cached in GetOrCompileSettingsForActorClass
	TMap<TWeakObjectPtr<UClass>, FSharedStruct> PerActorClassSettings;

	// Called in GetOrCompileSettingsForActorClass to compile finalized settings for ActorClass based off 
	// FArsInstancedActorsClassSettingsBase found in UArsInstancedActorsProjectSettings::ActorClassSettingsRegistryType
	// data registry, for ActorClass and it's inherited super classes.
	FSharedStruct CompileSettingsForActorClass(TSubclassOf<AActor> ActorClass) const;

#if WITH_EDITOR
	void HandleRefreshSettings(IConsoleVariable* InCVar);
#endif

	// Inactive UWorld housing lazily create exemplar actors for instance actor classes
	// @see GetOrCreateExemplarActor
	UPROPERTY(Transient)
	TObjectPtr<UWorld> ExemplarActorWorld;

	// Lazily created exemplar actors for instance actor classes
	// @see GetOrCreateExemplarActor
	TMap<TObjectKey<const UClass>, TWeakPtr<UE::ArsInstancedActors::FExemplarActorData>> ExemplarActors;

	UPROPERTY(Transient)
	TObjectPtr<const UScriptStruct> SettingsType;
};
