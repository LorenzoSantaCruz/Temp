// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ArsMechanicaAPI.h"

#include "ArsInstancedActorsIndex.h"
#include "MassActorSpawnerSubsystem.h"
#include "ServerArsInstancedActorsSpawnerSubsystem.generated.h"



struct FMassEntityManager;
class UArsInstancedActorsComponent;
struct FArsInstancedActorsInstanceHandle;
struct FActorSpawnParameters;

/**
 * Dedicated UMassActorSpawnerSubsystem subclass handling server-side Actor spawning for InstancedActor.
 * The main responsibility is ArsInstancedActors-specific setup of newly spawned actors, including configuring
 * UArsInstancedActorsComponent instanced a newly spawned actor hosts. 
 */
UCLASS(MinimalAPI)
class ARSMECHANICA_API UServerArsInstancedActorsSpawnerSubsystem : public UMassActorSpawnerSubsystem
{
	GENERATED_BODY()

public:
	
	/**
	 * Called by UArsInstancedActorsComponent::InitializeComponent to provide an opportunity, before BeginPlay, to
	 * catch Instanced Actors we're spawning, to set their FMassEntityHandle association.
	 */
	void OnInstancedActorComponentInitialize(UArsInstancedActorsComponent& InstancedActorComponent) const;

protected:

	//~ Begin USubsystem Overrides
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual bool ReleaseActorToPool(AActor* Actor) override;
	//~ Begin USubsystem Overrides
	
	//~ Begin UMassActorSpawnerSubsystem Overrides
	virtual ESpawnRequestStatus SpawnActor(FConstStructView SpawnRequestView, TObjectPtr<AActor>& OutSpawnedActor, FActorSpawnParameters& InOutSpawnParameters) const override;
    //~ End UMassActorSpawnerSubsystem Overrides

	TSharedPtr<FMassEntityManager> EntityManager;

	// Set during SpawnActor and cleared once complete, to 'catch' UInstancedActorComponents initializing during the actor 
	// spawn, matching their Owner to TransientActorBeingSpawned in OnInstancedActorComponentInitialize, to test if the component 
	// was 'spawned by Instanced Actors'.
	// If so, we call UArsInstancedActorsComponent::InitializeComponentForInstance and pass along TransientActorSpawningEntity
	// as the Mass entity 'owning' this spawned actor.
	UPROPERTY(Transient)
	mutable TObjectPtr<AActor> TransientActorBeingSpawned = nullptr;

	UPROPERTY(Transient)
	mutable FArsInstancedActorsInstanceHandle TransientActorSpawningInstance;
};

