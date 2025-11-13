// Copyright (c) 2024 Lorenzo Santa Cruz. All rights reserved.

#include "ArsInstancedActorsVisualizationTrait.h"
#include "ArsInstancedActorsData.h"
#include "ArsInstancedActorsManager.h"
#include "ArsInstancedActorsRepresentationActorManagement.h"
#include "ArsInstancedActorsRepresentationSubsystem.h"
#include "ArsInstancedActorsSettingsTypes.h"
#include "ArsInstancedActorsTypes.h"
#include "ArsInstancedActorsVisualizationProcessor.h"
#include "MassEntityTemplateRegistry.h"
#include "MassCommonFragments.h"
#include "MassActorSubsystem.h"


UArsInstancedActorsVisualizationTrait::UArsInstancedActorsVisualizationTrait(const FObjectInitializer& ObjectInitializer)
{
	Params.RepresentationActorManagementClass = UArsInstancedActorsRepresentationActorManagement::StaticClass();
	Params.LODRepresentation[EMassLOD::High] = EMassRepresentationType::HighResSpawnedActor;
	Params.LODRepresentation[EMassLOD::Medium] = EMassRepresentationType::HighResSpawnedActor;
	// @todo Re-enable this with proper handling of Enable / Disable 'kept' actors, in 
	// UArsInstancedActorsRepresentationActorManagement::SetActorEnabled, including replication to client.
	Params.bKeepLowResActors = false;

	bAllowServerSideVisualization = true;
	RepresentationSubsystemClass = UArsInstancedActorsRepresentationSubsystem::StaticClass();

	// Avoids registering the Static Mesh Descriptor during BuildTemplate, as it's already added
	// during UArsInstancedActorsManagers InitializeModifySpawn flow.
	bRegisterStaticMeshDesc = false;
}

void UArsInstancedActorsVisualizationTrait::InitializeFromInstanceData(UArsInstancedActorsData& InInstanceData)
{
	InstanceData = &InInstanceData;

	HighResTemplateActor = InstanceData->ActorClass;

	const bool bIsClient = InInstanceData.GetManagerChecked().IsNetMode(NM_Client);
	if (bIsClient)
	{
		// Don't attempt to spawn actors natively on clients. Instead, rely on bForceActorRepresentationForExternalActors to 
		// switch to Actor representation once replicated actors are set explicitly in UArsInstancedActorsData::SetReplicatedActor 
		// and maintain this until the actor is destroyed by the server, whereupon we'll fall back to StaticMeshInstance
		// again.
		Params.LODRepresentation[EMassLOD::High] = EMassRepresentationType::StaticMeshInstance;
		Params.LODRepresentation[EMassLOD::Medium] = EMassRepresentationType::StaticMeshInstance;
		Params.bForceActorRepresentationForExternalActors = true;
	}

	const FArsInstancedActorsSettings& Settings = InstanceData->GetSettings<const FArsInstancedActorsSettings>();
	
	StaticMeshInstanceDesc = InstanceData->GetDefaultVisualizationChecked().VisualizationDesc.ToMassVisualizationDesc();

	if (StaticMeshInstanceDesc.Meshes.IsEmpty())
	{
		ensure(InstanceData->GetDefaultVisualizationChecked().ISMComponents.IsEmpty());
		Params.LODRepresentation[EMassLOD::Low] = EMassRepresentationType::None;

		if (bIsClient)
		{
			// Don't attempt to switch to ISMC representation on clients for no mesh classes (which would otherwise crash)
			// Let the server spawn these actors and replicate them to clients.
			Params.LODRepresentation[EMassLOD::High] = EMassRepresentationType::None;
			Params.LODRepresentation[EMassLOD::Medium] = EMassRepresentationType::None;
		}
	}

	double SettingsMaxInstanceDistance = FArsInstancedActorsSettings::DefaultMaxInstanceDistance;
	float LODDistanceScale = 1.0f;
	double GlobalMaxInstanceDistanceScale = 1.0;
	Settings.ComputeLODDistanceData(SettingsMaxInstanceDistance, GlobalMaxInstanceDistanceScale, LODDistanceScale);

	LODParams.LODDistance[EMassLOD::High] = 0.f;
	LODParams.LODDistance[EMassLOD::Medium] = Settings.MaxActorDistance;
	LODParams.LODDistance[EMassLOD::Low] = Settings.MaxActorDistance;
	LODParams.LODDistance[EMassLOD::Off] = SettingsMaxInstanceDistance * GlobalMaxInstanceDistanceScale;
}

void UArsInstancedActorsVisualizationTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	check(InstanceData.IsValid() || BuildContext.IsInspectingData());

	Super::BuildTemplate(BuildContext, World);

	// we need IAs to be processed by a dedicated visualization processor, configured a bit differently than the default one.
	BuildContext.RemoveTag<FMassVisualizationProcessorTag>();

	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);

	BuildContext.AddFragment<FTransformFragment>();
	BuildContext.AddFragment<FMassActorFragment>();

	FArsInstancedActorsDataSharedFragment ManagerSharedFragment;
	ManagerSharedFragment.InstanceData = InstanceData;
	FSharedStruct SubsystemFragment = EntityManager.GetOrCreateSharedFragment(ManagerSharedFragment);

	FArsInstancedActorsDataSharedFragment* AsShared = SubsystemFragment.GetPtr<FArsInstancedActorsDataSharedFragment>();
	if (ensure(AsShared))
	{
		// this can happen when we unload data and then stream it back again - we end up with the same path object, but different pointer.
		// The old instance should be garbage now
		if (AsShared->InstanceData != InstanceData)
		{
			ensure(AsShared->InstanceData.IsValid() == false);
			AsShared->InstanceData = InstanceData;
		}
		
		// we also need to make sure the bulk LOD is reset since the shared fragment can survive the death of the original 
		// InstanceData while preserving the "runtime" value, which will mess up newly spawned entities.
		AsShared->BulkLOD = EArsInstancedActorsBulkLOD::MAX;

		if (BuildContext.IsInspectingData() == false)
		{
			InstanceData->SetSharedInstancedActorDataStruct(SubsystemFragment);
		}
	}
	// not adding SubsystemFragment do BuildContext on purpose, we temporarily use shared fragments to store IAD information.
	// To be moved to InstancedActorSubsystem in the future

	// ActorInstanceFragment will get initialized by UArsInstancedActorsInitializerProcessor
	BuildContext.AddFragment_GetRef<FMassActorInstanceFragment>();

	FArsInstancedActorsFragment& InstancedActorFragment = BuildContext.AddFragment_GetRef<FArsInstancedActorsFragment>();
	InstancedActorFragment.InstanceData = InstanceData;

	if (BuildContext.IsInspectingData() == false)
	{
		// @todo Implement version of AddVisualDescWithISMComponent that supports multiple ISMCs and use that here
		if (ensure(InstanceData.IsValid()))
		{
			FMassRepresentationFragment* RepresentationFragment = BuildContext.GetFragment<FMassRepresentationFragment>();
			if (ensureMsgf(RepresentationFragment, TEXT("Configuration error, we always expect to have a FMassRepresentationFragment instance at this point")))
			{
				RepresentationFragment->StaticMeshDescHandle = InstanceData->GetDefaultVisualizationChecked().MassStaticMeshDescHandle;

				if (RepresentationFragment->LowResTemplateActorIndex == INDEX_NONE)
				{
					// if there's no "low res actor" we reuse the high-res one, otherwise we risk the visualization actor getting 
					// removed when switching from EMassLOD::High down to EMassLOD::Medium
					RepresentationFragment->LowResTemplateActorIndex = RepresentationFragment->HighResTemplateActorIndex;
				}
			}
		}
	}
}
