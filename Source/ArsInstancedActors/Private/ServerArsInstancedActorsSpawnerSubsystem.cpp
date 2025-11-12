// Copyright Epic Games, Inc. All Rights Reserved.


#include "ServerArsInstancedActorsSpawnerSubsystem.h"
#include "ArsInstancedActorsComponent.h"
#include "ArsInstancedActorsData.h"
#include "Engine/World.h"
#include "MassEntitySubsystem.h"
#include "ArsInstancedActorsSettings.h"

//-----------------------------------------------------------------------------
// UServerArsInstancedActorsSpawnerSubsystem
//-----------------------------------------------------------------------------
bool UServerArsInstancedActorsSpawnerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer))
	{
		return false;
	}

	// @todo Add support for non-replay NM_Standalone where we should use UServerArsInstancedActorsSpawnerSubsystem for 
	// authoritative actor spawning.
	UWorld* World = Cast<UWorld>(Outer);
	return (World != nullptr && World->GetNetMode() != NM_Client);
}

bool UServerArsInstancedActorsSpawnerSubsystem::ReleaseActorToPool(AActor* Actor)
{
	return Super::ReleaseActorToPool(Actor);
}

void UServerArsInstancedActorsSpawnerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UMassEntitySubsystem* EntitySubsystem = Collection.InitializeDependency<UMassEntitySubsystem>();
	check(EntitySubsystem);
	EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
}

void UServerArsInstancedActorsSpawnerSubsystem::Deinitialize()
{
	EntityManager.Reset();

	Super::Deinitialize();
}

ESpawnRequestStatus UServerArsInstancedActorsSpawnerSubsystem::SpawnActor(FConstStructView SpawnRequestView, TObjectPtr<AActor>& OutSpawnedActor, FActorSpawnParameters& InOutSpawnParameters) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UServerArsInstancedActorsSpawnerSubsystem::SpawnActor);

	UWorld* World = GetWorld();
	check(World);
	check(World->GetNetMode() != NM_Client);

	const FMassActorSpawnRequest& SpawnRequest = SpawnRequestView.Get<const FMassActorSpawnRequest>();
	UArsInstancedActorsData* InstanceData = UArsInstancedActorsData::GetInstanceDataForEntity(*EntityManager, SpawnRequest.MassAgent);
	check(InstanceData);
	const FArsInstancedActorsInstanceIndex InstanceIndex = InstanceData->GetInstanceIndexForEntity(SpawnRequest.MassAgent);
	const FArsInstancedActorsInstanceHandle InstanceHandle(*InstanceData, InstanceIndex);

	// Record currently spawning IA instance for OnInstancedActorComponentInitialize to check
	TransientActorSpawningInstance = InstanceHandle;
	ON_SCOPE_EXIT 
	{
		TransientActorBeingSpawned = nullptr;
		TransientActorSpawningInstance.Reset();
	};

	InOutSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	// we’re going to call FinishSpawning only if the input parameters don’t indicate that the caller wants to handle it themselves.
	const bool bCallFinishSpawning = (InOutSpawnParameters.bDeferConstruction == false);
	// we always defer construction to have a chance to configure the UArsInstancedActorsComponent instances
	// before their InitializeComponent gets called. From the callers point of view nothing changes.
	InOutSpawnParameters.bDeferConstruction = true;

	OutSpawnedActor = World->SpawnActor<AActor>(SpawnRequest.Template, SpawnRequest.Transform, InOutSpawnParameters);
	if (ensureMsgf(OutSpawnedActor, TEXT("Failed to spawn actor of class %s"), *GetNameSafe(SpawnRequest.Template.Get())))
	{
		// @todo this is a temporary solution, the whole idea is yucky and needs to be reimplemented.
		// Before this addition TransientActorBeingSpawned was only being set in Juno's custom 
		// InOutSpawnParameters.CustomPreSpawnInitalization delegate
		TransientActorBeingSpawned = OutSpawnedActor;

		// Add an UArsInstancedActorsComponent if one isn't present and ensure replication is enabled to replicate the InstanceHandle 
		// to clients for Mass entity matchup in UArsInstancedActorsComponent::OnRep_InstanceHandle
		UArsInstancedActorsComponent* InstancedActorComponent = OutSpawnedActor->GetComponentByClass<UArsInstancedActorsComponent>();
		if (InstancedActorComponent)
		{
			// If the component is set to replicate by default, we assume AddComponentTypesAllowListedForReplication has 
			// already been performed.
			if (!InstancedActorComponent->GetIsReplicated())
			{
				InstancedActorComponent->SetIsReplicated(true);
			}
		}
		else
		{
			// No existing UArsInstancedActorsComponent class or subclass, add a new UArsInstancedActorsComponent
			InstancedActorComponent = NewObject<UArsInstancedActorsComponent>(OutSpawnedActor);
			if (OutSpawnedActor->GetIsReplicated() == false)
			{
				OutSpawnedActor->SetReplicates(true);
			}
			InstancedActorComponent->SetIsReplicated(true);
			InstancedActorComponent->RegisterComponent();
		}

		if (bCallFinishSpawning)
		{
			OutSpawnedActor->FinishSpawning(SpawnRequest.Transform);
		}
	}
	
	return IsValid(OutSpawnedActor) ? ESpawnRequestStatus::Succeeded : ESpawnRequestStatus::Failed;
}

void UServerArsInstancedActorsSpawnerSubsystem::OnInstancedActorComponentInitialize(UArsInstancedActorsComponent& InstancedActorComponent) const
{
	// Does this component belong to an actor we're in the middle of spawning in UServerArsInstancedActorsSpawnerSubsystem::SpawnActor?
	//
	// Note: This may not always be the case, as OnInstancedActorComponentInitialize is called by UArsInstancedActorsComponent::InitializeComponent 
	// regardless of whether the actor was spawned by Instanced Actors or not, as we can't yet know if it was (this callback is in place to
	// *attempt* to find that out). Actors using UInstancedActorComponents aren't 'required' to be spawned with Instanced Actors, rather: the
	// components are expected to provide functionality without Mass and simply provide *additional* ability to continue their functionality once
	// the actor is 'dehydrated' into the lower LOD representation in Mass.
	if (InstancedActorComponent.GetOwner() == TransientActorBeingSpawned)
	{
		// Pass the IA instance responsible for spawning this actor. Importantly the UArsInstancedActorsComponent will now have a link
		// to Mass before / by the time it receives BeginPlay.
		check(TransientActorSpawningInstance.IsValid());
		InstancedActorComponent.InitializeComponentForInstance(TransientActorSpawningInstance);
	}
}
