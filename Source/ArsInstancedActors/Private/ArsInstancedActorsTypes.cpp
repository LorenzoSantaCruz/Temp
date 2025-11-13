// Copyright (c) 2024 Lorenzo Santa Cruz. All rights reserved.

#include "ArsInstancedActorsTypes.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "ArsInstancedActorsSettings.h"
#include "ServerArsInstancedActorsSpawnerSubsystem.h"
#include "ClientArsInstancedActorsSpawnerSubsystem.h"

DEFINE_LOG_CATEGORY(LogArsInstancedActors);

namespace UE::ArsInstancedActors::Utils
{
	TSubclassOf<UMassActorSpawnerSubsystem> DetermineActorSpawnerSubsystemClass(const UWorld& World)
	{
		// @todo Add support for non-replay NM_Standalone where we should use UServerArsInstancedActorsSpawnerSubsystem for 
		// authoritative actor spawning.
		if (World.GetNetMode() == NM_Client)
		{
			return GET_ARSINSTANCEDACTORS_CONFIG_VALUE(GetClientActorSpawnerSubsystemClass());
		}
		return GET_ARSINSTANCEDACTORS_CONFIG_VALUE(GetServerActorSpawnerSubsystemClass());
	}

	UServerArsInstancedActorsSpawnerSubsystem* GetServerArsInstancedActorsSpawnerSubsystem(const UWorld& World)
	{
		TSubclassOf<UMassActorSpawnerSubsystem> SpawnerSubsystemClass = GET_ARSINSTANCEDACTORS_CONFIG_VALUE(GetServerActorSpawnerSubsystemClass());
		check(SpawnerSubsystemClass);
		return Cast<UServerArsInstancedActorsSpawnerSubsystem>(World.GetSubsystemBase(SpawnerSubsystemClass));
	}

	UClientArsInstancedActorsSpawnerSubsystem* GetClientArsInstancedActorsSpawnerSubsystem(const UWorld& World)
	{
		TSubclassOf<UMassActorSpawnerSubsystem> SpawnerSubsystemClass = GET_ARSINSTANCEDACTORS_CONFIG_VALUE(GetClientActorSpawnerSubsystemClass());
		check(SpawnerSubsystemClass);
		return Cast<UClientArsInstancedActorsSpawnerSubsystem>(World.GetSubsystemBase(SpawnerSubsystemClass));
	}

	UMassActorSpawnerSubsystem* GetActorSpawnerSubsystem(const UWorld& World)
	{
		if (World.GetNetMode() == NM_Client)
		{
			return GetClientArsInstancedActorsSpawnerSubsystem(World);
		}
		return GetServerArsInstancedActorsSpawnerSubsystem(World);
	}

	UArsInstancedActorsSubsystem* GetArsInstancedActorsSubsystem(const UWorld& World)
	{
		TSubclassOf<UArsInstancedActorsSubsystem> ArsInstancedActorsSubsystemClass = GET_ARSINSTANCEDACTORS_CONFIG_VALUE(GetArsInstancedActorsSubsystemClass());
		check(ArsInstancedActorsSubsystemClass);

		UArsInstancedActorsSubsystem* InstancedActorSubsystem = Cast<UArsInstancedActorsSubsystem>(World.GetSubsystemBase(ArsInstancedActorsSubsystemClass));

		return InstancedActorSubsystem;
	}
} // namespace UE::ArsInstancedActors::Utils

//-----------------------------------------------------------------------------
// FArsInstancedActorsTagSet
//-----------------------------------------------------------------------------
FArsInstancedActorsTagSet::FArsInstancedActorsTagSet(const FGameplayTagContainer& InTags)
{
	TArray<FGameplayTag> SortedTags;
	InTags.GetGameplayTagArray(SortedTags);
	SortedTags.Sort();

	Tags = FGameplayTagContainer::CreateFromArray(SortedTags);

	CalcHash();
}

void FArsInstancedActorsTagSet::CalcHash()
{
	uint32 NewHash = 0;
	for (const FGameplayTag& Tag : Tags.GetGameplayTagArray())
	{
		NewHash = HashCombine(NewHash, GetTypeHash(Tag));
	}
	
	Hash = NewHash;
}

//-----------------------------------------------------------------------------
// FStaticMeshInstanceVisualizationDesc
//-----------------------------------------------------------------------------
FArsInstancedActorsVisualizationDesc::FArsInstancedActorsVisualizationDesc(const FArsInstancedActorsSoftVisualizationDesc& SoftVisualizationDesc)
{
	for (const FSoftISMComponentDescriptor& SoftISMComponentDescriptor : SoftVisualizationDesc.ISMComponentDescriptors)
	{
		// Calls FISMComponentDescriptor(FSoftISMComponentDescriptor&) which will LoadSynchronous any soft paths
		ISMComponentDescriptors.Emplace(SoftISMComponentDescriptor);
	}
}

FArsInstancedActorsVisualizationDesc FArsInstancedActorsVisualizationDesc::FromActor(const AActor& ExemplarActor
	, const FAdditionalSetupStepsFunction& AdditionalSetupSteps)
{
	return FromActor(ExemplarActor, [&AdditionalSetupSteps](const AActor& ExemplarActor, FArsInstancedActorsVisualizationDesc& OutVisualization)
	{
		if (OutVisualization.ISMComponentDescriptors.Num() > 0)
		{
			AdditionalSetupSteps(ExemplarActor, OutVisualization.ISMComponentDescriptors[0], OutVisualization);
		}
	});
}

FArsInstancedActorsVisualizationDesc FArsInstancedActorsVisualizationDesc::FromActor(const AActor& ExemplarActor,
	const FVisualizationDescSetupFunction& AdditionalSetupSteps)
{
	FArsInstancedActorsVisualizationDesc Visualization;
	USceneComponent* RootComponent = ExemplarActor.GetRootComponent();
	
	ExemplarActor.ForEachComponent<UStaticMeshComponent>(/*bIncludeFromChildActors=*/false, [&Visualization, RootComponent](UStaticMeshComponent* SourceStaticMeshComponent)
	{
		if (!SourceStaticMeshComponent->IsVisible())
		{
			return;
		}

		const UStaticMesh* StaticMesh = SourceStaticMeshComponent->GetStaticMesh();
		if (!IsValid(StaticMesh))
		{
			// No mesh = no visualization
			return;
		}

		FISMComponentDescriptor& ISMComponentDescriptor = Visualization.ISMComponentDescriptors.AddDefaulted_GetRef();
		ISMComponentDescriptor.InitFrom(SourceStaticMeshComponent);

		// LocalTransform means local to the Actor/Entity, so we need to compute based on the UStaticMeshComponent's relative transform accordingly
		// (in case this UStaticMeshComponent was a child of another UStaticMeshComponent within the Actor hierarchy)
		if (SourceStaticMeshComponent != RootComponent)
		{
			ISMComponentDescriptor.LocalTransform = SourceStaticMeshComponent->GetComponentToWorld();
		}
	});

	AdditionalSetupSteps(ExemplarActor, Visualization);

	return MoveTemp(Visualization);
}

FStaticMeshInstanceVisualizationDesc FArsInstancedActorsVisualizationDesc::ToMassVisualizationDesc() const
{
	FStaticMeshInstanceVisualizationDesc OutMassVisualizationDesc;

	for (const FISMComponentDescriptor& ISMComponentDescriptor : ISMComponentDescriptors)
	{
		if (!ensure(IsValid(ISMComponentDescriptor.StaticMesh)))
		{
			continue;
		}

		FMassStaticMeshInstanceVisualizationMeshDesc& MeshDesc = OutMassVisualizationDesc.Meshes.AddDefaulted_GetRef();
		MeshDesc.Mesh = ISMComponentDescriptor.StaticMesh;
		MeshDesc.LocalTransform = ISMComponentDescriptor.LocalTransform;
		MeshDesc.bCastShadows = ISMComponentDescriptor.bCastShadow;
		MeshDesc.Mobility = EComponentMobility::Stationary;
		MeshDesc.MaterialOverrides = ISMComponentDescriptor.OverrideMaterials;
		MeshDesc.ISMComponentClass = UInstancedStaticMeshComponent::StaticClass();
	}

	OutMassVisualizationDesc.CustomDataFloats = CustomDataFloats;

	return MoveTemp(OutMassVisualizationDesc);
}

//-----------------------------------------------------------------------------
// FArsInstancedActorsSoftVisualizationDesc
//-----------------------------------------------------------------------------
FArsInstancedActorsSoftVisualizationDesc::FArsInstancedActorsSoftVisualizationDesc(const FArsInstancedActorsVisualizationDesc& VisualizationDesc)
{
	for (const FISMComponentDescriptor& ISMComponentDescriptor : VisualizationDesc.ISMComponentDescriptors)
	{
		ISMComponentDescriptors.Emplace(ISMComponentDescriptor);
	}
}

void FArsInstancedActorsSoftVisualizationDesc::GetAssetsToLoad(TArray<FSoftObjectPath>& OutAssetsToLoad) const
{
	for (const FSoftISMComponentDescriptor& ISMComponentDescriptor : ISMComponentDescriptors)
	{
		if (ISMComponentDescriptor.StaticMesh.IsPending())
		{
			OutAssetsToLoad.Add(ISMComponentDescriptor.StaticMesh.ToSoftObjectPath());
		}
		for (const TSoftObjectPtr<UMaterialInterface>& OverrideMaterial : ISMComponentDescriptor.OverrideMaterials)
		{
			if (OverrideMaterial.IsPending())
			{
				OutAssetsToLoad.Add(OverrideMaterial.ToSoftObjectPath());
			}
		}
		if (ISMComponentDescriptor.OverlayMaterial.IsPending())
		{
			OutAssetsToLoad.Add(ISMComponentDescriptor.OverlayMaterial.ToSoftObjectPath());
		}
		for (const TSoftObjectPtr<URuntimeVirtualTexture>& RuntimeVirtualTexture : ISMComponentDescriptor.RuntimeVirtualTextures)
		{
			if (RuntimeVirtualTexture.IsPending())
			{
				OutAssetsToLoad.Add(RuntimeVirtualTexture.ToSoftObjectPath());
			}
		}
	}
}
