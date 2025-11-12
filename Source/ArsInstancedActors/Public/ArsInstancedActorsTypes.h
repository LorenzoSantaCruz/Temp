// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ArsMechanicaAPI.h"

#include "Logging/LogMacros.h"
#include "GameplayTagContainer.h"
#include "ISMPartition/ISMComponentDescriptor.h"
#include "MassEntityTypes.h"
#include "MassRepresentationTypes.h"
#include "ArsInstancedActorsIndex.h"
#include "ArsInstancedActorsTypes.generated.h"



DECLARE_LOG_CATEGORY_EXTERN(LogArsInstancedActors, Log, All)

struct FStaticMeshInstanceVisualizationDesc;
struct FStreamableHandle;
struct FMassISMCSharedData;
class UArsInstancedActorsData;
class UWorld;
class UServerArsInstancedActorsSpawnerSubsystem;
class UClientArsInstancedActorsSpawnerSubsystem;
class UMassActorSpawnerSubsystem;
class UArsInstancedActorsSubsystem;

enum class EArsInstancedActorsBulkLOD : uint8
{
	Detailed, // this will make Mass calculate LOD individually for every instance
	Medium,
	Low,
	Off,
	MAX
};

enum class EArsInstancedActorsBulkLODMask : uint8
{
	None = 0,

	Detailed = 1 << int(EArsInstancedActorsBulkLOD::Detailed),
	Medium = 1 << int(EArsInstancedActorsBulkLOD::Medium),
	Low = 1 << int(EArsInstancedActorsBulkLOD::Low),
	Off = 1 << int(EArsInstancedActorsBulkLOD::Off),

	NotDetailed = Medium | Low | Off,
	All = 0xFF
};

enum class EArsInstancedActorsFragmentFlags : uint8
{
    None = 0,

    Replicated = 1 << 0,
    Persisted = 1 << 1,

    All = 0xFF,
};
ENUM_CLASS_FLAGS(EArsInstancedActorsFragmentFlags);

namespace UE::ArsInstancedActors::Utils
{
	TSubclassOf<UMassActorSpawnerSubsystem> DetermineActorSpawnerSubsystemClass(const UWorld& World);
	UServerArsInstancedActorsSpawnerSubsystem* GetServerArsInstancedActorsSpawnerSubsystem(const UWorld& World);
	UClientArsInstancedActorsSpawnerSubsystem* GetClientArsInstancedActorsSpawnerSubsystem(const UWorld& World);
	/** 
	 * Calls either GetServerArsInstancedActorsSpawnerSubsystem or GetClientArsInstancedActorsSpawnerSubsystem, depending on 
	 * given UWorld's net mode.
	 */
	UMassActorSpawnerSubsystem* GetActorSpawnerSubsystem(const UWorld& World);
	UArsInstancedActorsSubsystem* GetArsInstancedActorsSubsystem(const UWorld& World);
}

// FArsInstancedActorsTagSet -> FArsInstancedActorsTagSet
/** An immutable hashed tag container used to categorize / partition instances */
USTRUCT(BlueprintType)
struct FArsInstancedActorsTagSet
{
	GENERATED_BODY()

	FArsInstancedActorsTagSet() = default;
	FArsInstancedActorsTagSet(const FGameplayTagContainer& InTags);

	bool IsEmpty() const { return Tags.IsEmpty(); }

	const FGameplayTagContainer& GetTags() const { return Tags; }
	
	void CalcHash();

	bool operator==(const FArsInstancedActorsTagSet& OtherTagSet) const
	{
		if (Hash.IsSet() && OtherTagSet.Hash.IsSet() && Hash.GetValue() != OtherTagSet.Hash.GetValue())
		{
			return false;
		}

		if (Tags.Num() != OtherTagSet.Tags.Num())
		{
			return false;
		}

		// Tags member will have already been constructed from a sorted TArray
		for (int32 i = 0; i < Tags.Num(); ++i)
		{
			if (Tags.GetGameplayTagArray()[i] != OtherTagSet.Tags.GetGameplayTagArray()[i])
			{
				return false;
			}
		}

		return true;
	}
	
private:

	UPROPERTY(VisibleAnywhere, Category=ArsInstancedActors)
	FGameplayTagContainer Tags;

	UPROPERTY(Transient)
	TOptional<uint32> Hash;
};


/**
 * ISMC descriptions for instances 'visualization', allowing instances to define multiple
 * potential visualizations / ISMC sets: e.g: 'with berries', 'without berries'.
 */
USTRUCT()
struct FArsInstancedActorsVisualizationDesc
{
	GENERATED_BODY()

	FArsInstancedActorsVisualizationDesc() = default;
	explicit FArsInstancedActorsVisualizationDesc(const FArsInstancedActorsSoftVisualizationDesc& SoftVisualizationDesc);

	/**
	 * Array of Instanced Static Mesh Component descriptors. An ISMC will be created for each of these, using the specified mesh, material,
	 * collision settings etc. Instanced Actors using this visualization will add an instance to each of these, allowing for composite mesh
	 * visualizations for a single actor instance e.g: a car with separate body and wheel meshes all instanced together.
	 */
	UPROPERTY(VisibleAnywhere, Category = ArsInstancedActors)
	TArray<FISMComponentDescriptor> ISMComponentDescriptors;

	UPROPERTY(VisibleAnywhere, Category = ArsInstancedActors)
	TArray<float> CustomDataFloats;

	using FAdditionalSetupStepsFunction = TFunctionRef<void(const AActor& /*ExemplarActor*/, FISMComponentDescriptor& /*NewISMComponentDescriptor*/, FArsInstancedActorsVisualizationDesc& /*OutVisualization*/)>;	
	using FVisualizationDescSetupFunction = TFunctionRef<void(const AActor& /*ExemplarActor*/, FArsInstancedActorsVisualizationDesc& /*OutVisualization*/)>;
	
	UE_DEPRECATED(5.6, "Using FAdditionalSetupStepsFunction which takes a FISMComponentDescriptor& parameter is deprecated. Use FVisualizationDescSetupFunction instead, since FArsInstancedActorsVisualizationDesc already contains the built TArray of FISMComponentDescriptor")
	static FArsInstancedActorsVisualizationDesc FromActor(const AActor& ExemplarActor, const FAdditionalSetupStepsFunction& AdditionalSetupSteps);
	
	/**
	 * Helper function to deduce appropriate instanced static mesh representation for an ActorClass exemplar actor.
	 * @param ExemplarActor A fully constructed exemplar actor, including BP construction scripts up to (but not including) BeginPlay.
	 * @param AdditionalSetupFunction function called once a new ISMComponentDescriptor is created, allowing custom code to 
	 *	perform additional set up steps. Note that this function will get called only if ExemplarActor has a 
	 *	StaticMeshComponent with a valid StaticMesh configured.
	 * @see UArsInstancedActorsSubsystem::GetOrCreateExemplarActor
	 */	
	static FArsInstancedActorsVisualizationDesc FromActor(const AActor& ExemplarActor, const FVisualizationDescSetupFunction& AdditionalSetupFunction = [](const AActor& /*ExemplarActor*/, FArsInstancedActorsVisualizationDesc& /*OutVisualization*/){});

	FStaticMeshInstanceVisualizationDesc ToMassVisualizationDesc() const;

	friend inline uint32 GetTypeHash(const FArsInstancedActorsVisualizationDesc& InDesc)
	{
		uint32 Hash = 0;
		for (const FISMComponentDescriptor& InstancedMesh : InDesc.ISMComponentDescriptors)
		{
			Hash = HashCombine(Hash, GetTypeHash(InstancedMesh));
		}
		return Hash;
	}
};


/**
 * Soft-ptr variant of FArsInstancedActorsVisualizationDesc for defining visualization assets to async load.
 */
USTRUCT()
struct FArsInstancedActorsSoftVisualizationDesc
{
	GENERATED_BODY()

	FArsInstancedActorsSoftVisualizationDesc() = default;
	explicit FArsInstancedActorsSoftVisualizationDesc(const FArsInstancedActorsVisualizationDesc& VisualizationDesc);

	/**
	 * Array of Instanced Static Mesh Component descriptors. An ISMC will be created for each of these, using the specified mesh, material,
	 * collision settings etc. Instanced Actors using this visualization will add an instance to each of these, allowing for composite mesh
	 * visualizations for a single actor instance e.g: a car with separate body and wheel meshes all instanced together.
	 */
	UPROPERTY(VisibleAnywhere, Category = ArsInstancedActors)
	TArray<FSoftISMComponentDescriptor> ISMComponentDescriptors;

	void GetAssetsToLoad(TArray<FSoftObjectPath>& OutAssetsToLoad) const;
};


/** Runtime ISMC tracking for a given 'visualization' (alternate ISMC set) for instances */
USTRUCT()
struct FArsInstancedActorsVisualizationInfo
{
	GENERATED_BODY()

	/**
	 * Returns true if this visualization was added view UArsInstancedActorsData::AddVisualizationAsync and streaming is still in-progress.
	 * Once streaming completes, Desc, ISMComponents and MassStaticMeshDescIndex will be valid and this returns false.
	 * Note: Until streaming completes, Desc, ISMComponents & MassStaticMeshDescIndex will all be defayult values / unset.
	 */
	FORCEINLINE bool IsAsyncLoading() const { return AssetLoadHandle.IsValid(); }

	/**
	 * Cached specification for this visualization, defining ISMCs to create.
	 * Note: For visualizations added via UArsInstancedActorsData::AddVisualizationAsync using an FArsInstancedActorsSoftVisualizationDesc
	 *       soft pointer descriptor: whilst IsAsyncLoading() = true, this will be default constructed. Once the async load completes, the
	 *       soft visualization decriptor will then be resolved to this hard pointer decriptor.
	 */
	UPROPERTY(VisibleAnywhere, Category = ArsInstancedActors)
	FArsInstancedActorsVisualizationDesc VisualizationDesc;

	/**
	 * Instanced Static Mesh Components created from VisualizationDesc.ISMComponentDescriptors specs.
	 *
	 * Note: If IsAsyncLoading() = true, this will be empty until the load completes and ISMCs are created.
	 */
	UPROPERTY(VisibleAnywhere, Category = ArsInstancedActors)
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> ISMComponents;

	/**
	 * Handle to registration of ISMComponents with UMassRepresentationSubsystem via
	 * UMassRepresentationSubsystem::AddVisualDescWithISMComponents.
	 *
	 * Note: If IsAsyncLoading() = true, this will be invalid until the load completes and ISMCs are created and registered.
	 */
	UPROPERTY(VisibleAnywhere, Category = ArsInstancedActors)
	FStaticMeshInstanceVisualizationDescHandle MassStaticMeshDescHandle;

	// If this visualization was added with UArsInstancedActorsData::AddVisualizationAsync, this will be set to the async streaming request 
	// until streaming is complete, whereupon this handle is cleared.
	TSharedPtr<FStreamableHandle> AssetLoadHandle;

	/** Used to track version of data used to create CollisionIndexToEntityIndexMap */
	mutable uint16 CachedTouchCounter = 0;
	/**
	 * Valid as long as Mass visualization data indicated by MassStaticMeshDescIndex has ComponentInstanceIdTouchCounter
	 * equal to CachedTouchCounter.
	 */
	mutable TArray<int32> CollisionIndexToEntityIndexMap;
};


USTRUCT()
struct FArsInstancedActorsMeshSwitchFragment : public FMassFragment
{
	GENERATED_BODY()

	// The pending Mass static mesh representation index we want to switch to.
	// @see UArsInstancedActorsVisualizationSwitcherProcessor
	UPROPERTY()
	FStaticMeshInstanceVisualizationDescHandle NewStaticMeshDescHandle;
};


USTRUCT(BlueprintType)
struct FArsInstancedActorsManagerHandle
{
	GENERATED_BODY()

public:

	FArsInstancedActorsManagerHandle() = default;
	FArsInstancedActorsManagerHandle(const int32 InManagerID) : ManagerID(InManagerID) {}

	FORCEINLINE bool IsValid() const
	{
		return ManagerID != INDEX_NONE;
	}

	FORCEINLINE void Reset()
	{
		ManagerID = INDEX_NONE;
	}

	int32 GetManagerID() const
	{
		return ManagerID;
	}

	bool operator==(const FArsInstancedActorsManagerHandle&) const = default;

private:

	UPROPERTY(Transient)
	int32 ManagerID = INDEX_NONE;
};

USTRUCT(BlueprintType)
struct FArsInstancedActorsModifierVolumeHandle
{
	GENERATED_BODY()

public:

	FArsInstancedActorsModifierVolumeHandle() = default;
	FArsInstancedActorsModifierVolumeHandle(const int32 InModifierVolumeID) : ModifierVolumeID(InModifierVolumeID) {}

	int32 GetModifierVolumeID() const
	{
		return ModifierVolumeID;
	}

	bool operator==(const FArsInstancedActorsModifierVolumeHandle&) const = default;

private:

	UPROPERTY(Transient)
	int32 ModifierVolumeID = INDEX_NONE;
};

/**
 * Note that we don't really need this type to be a shared fragment. It's used to create FSharedStructs pointing at
 * UArsInstancedActorsData and this data is fetched from MassEntityManager by UArsInstancedActorsStationaryLODBatchProcessor.
 * @todo This will be addressed in the future by refactoring where this data is stored and how it's used.
 */
USTRUCT()
struct FArsInstancedActorsDataSharedFragment : public FMassSharedFragment
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TWeakObjectPtr<UArsInstancedActorsData> InstanceData;

	EArsInstancedActorsBulkLOD BulkLOD = EArsInstancedActorsBulkLOD::MAX;

	double LastTickTime = 0.0;
};

USTRUCT()
struct FArsInstancedActorsFragment : public FMassFragment
{
	GENERATED_BODY()

	// InstancedActorData owning the given entity
	UPROPERTY(Transient)
	TWeakObjectPtr<UArsInstancedActorsData> InstanceData;

	// The fixed index of this 'instance' into InstanceData
	UPROPERTY()
	FArsInstancedActorsInstanceIndex InstanceIndex;
};

