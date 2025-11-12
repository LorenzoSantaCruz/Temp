// Copyright Epic Games, Inc. All Rights Reserved.

#include "ClientArsInstancedActorsSpawnerSubsystem.h"
#include "ArsInstancedActorsComponent.h"
#include "ArsInstancedActorsDebug.h"
#include "MassEntitySubsystem.h"
#include "Engine/Level.h"
#include "ArsInstancedActorsSettings.h"


//-----------------------------------------------------------------------------
// UClientArsInstancedActorsSpawnerSubsystem
//-----------------------------------------------------------------------------
bool UClientArsInstancedActorsSpawnerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer))
	{
		return false;
	}

	// @todo Add support for non-replay NM_Standalone where we should use UServerArsInstancedActorsSpawnerSubsystem for 
	// authoritative actor spawning.
	UWorld* World = Cast<UWorld>(Outer);
	return (World != nullptr && World->GetNetMode() == NM_Client);
}

ESpawnRequestStatus UClientArsInstancedActorsSpawnerSubsystem::SpawnActor(FConstStructView SpawnRequestView, TObjectPtr<AActor>& OutSpawnedActor, FActorSpawnParameters& InOutSpawnParameters) const
{
	// UArsInstancedActorsVisualizationTrait should be setting all LOD representations to StaticMeshInstance to avoid
	// ever attempting to natively spawn an actor.
	// 
	// Instead we rely on UArsInstancedActorsComponent's, added dynamically to all actors on spawn in UServerArsInstancedActorsSpawnerSubsystem::SpawnActor, 
	// to replicate over and 'inject' the Actor into Mass in UArsInstancedActorsComponent::OnRep_InstanceHandle.
	//
	// UArsInstancedActorsVisualizationTrait also sets bForceActorRepresentationWhileAvailable on clients so that once the replicated 
	// actor is registered with Mass, we switch into Actor representation and stay there until Actor destruction is replicate,
	// whereupon we switch back to whatever the natural wanted representation is (ISMC)
	ensureMsgf(false, TEXT("UClientArsInstancedActorsSpawnerSubsystem::SpawnActor unexpectedly called on client where we shouldn't ever be trying to spawn new actors."));

	return ESpawnRequestStatus::Pending;
}

bool UClientArsInstancedActorsSpawnerSubsystem::ReleaseActorToPool(AActor* Actor)
{
	// As we set bForceActorRepresentationWhileAvailable on clients (see above), we should never be attempting to 
	// explicitly destroy actors from Mass. Rather, actor destruction should be happening via replication from server,
	// relying on UArsInstancedActorsComponent::EndPlay to clean up the actor reference in Mass.
	ensureMsgf(false, TEXT("UClientArsInstancedActorsSpawnerSubsystem::ReleaseActorToPool unexpectedly called on client where we shouldn't ever be trying to destroy actors."));

	return true;
}
