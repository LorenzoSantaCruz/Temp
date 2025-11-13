// Copyright (c) 2024 Lorenzo Santa Cruz. All rights reserved.

#include "ArsInstancedActorsComponent.h"
#include "ArsInstancedActorsManager.h"
#include "ArsInstancedActorsData.h"
#include "ServerArsInstancedActorsSpawnerSubsystem.h"
#include "ArsInstancedActorsSettings.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"


UArsInstancedActorsComponent::UArsInstancedActorsComponent()
{
	bWantsInitializeComponent = true;
}

void UArsInstancedActorsComponent::OnServerPreSpawnInitForInstance(FArsInstancedActorsInstanceHandle InInstanceHandle)
{
	InstanceHandle = InInstanceHandle;
}

void UArsInstancedActorsComponent::InitializeComponentForInstance(FArsInstancedActorsInstanceHandle InInstanceHandle)
{
	InstanceHandle = InInstanceHandle;
}

void UArsInstancedActorsComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UArsInstancedActorsComponent, InstanceHandle, COND_InitialOnly, REPNOTIFY_OnChanged);
}

void UArsInstancedActorsComponent::OnRep_InstanceHandle()
{
	// Note: The client may not have loaded InstanceHandle.InstancedActorData yet, resulting in an invalid InstanceHandle. Once the client
	// 		 completes the load however, we'll get another OnRep_InstanceHandle with the fixed up InstancedActorData pointer.
	if (InstanceHandle.IsValid())
	{
		check(GetOwner());
		InstanceHandle.GetInstanceActorDataChecked().SetReplicatedActor(InstanceHandle.GetInstanceIndex(), *GetOwner());
	}
}

void UArsInstancedActorsComponent::InitializeComponent()
{
	Super::InitializeComponent();

	// @todo Add support for non-replay NM_Standalone where we should call OnInstancedActorComponentInitialize
	UWorld* World = GetWorld();
	if (World && World->GetNetMode() != NM_Client)
	{
		UServerArsInstancedActorsSpawnerSubsystem* ServerInstancedActorSpawnerSubystem = UE::ArsInstancedActors::Utils::GetServerArsInstancedActorsSpawnerSubsystem(*World);
		if (IsValid(ServerInstancedActorSpawnerSubystem))
		{
			ServerInstancedActorSpawnerSubystem->OnInstancedActorComponentInitialize(*this);
		}
	}
}

void UArsInstancedActorsComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Deregister Actor from entity on clients
	if (GetNetMode() == NM_Client && InstanceHandle.IsValid())
	{
		AActor* Owner = GetOwner();
		if (ensure(Owner))
		{
			InstanceHandle.GetInstanceActorDataChecked().ClearReplicatedActor(InstanceHandle.GetInstanceIndex(), *Owner);
		}
	}

	// @todo Callback from UMassActorSpawnerSubsystem when if/when we're released
	InstanceHandle.Reset();

	Super::EndPlay(EndPlayReason);
}

FMassEntityHandle UArsInstancedActorsComponent::GetMassEntityHandle() const
{
	if (InstanceHandle.IsValid())
	{
		return InstanceHandle.GetInstanceActorDataChecked().GetEntity(InstanceHandle.GetInstanceIndex());
	}

	return FMassEntityHandle();
}

TSharedPtr<FMassEntityManager> UArsInstancedActorsComponent::GetMassEntityManager() const
{
	if (InstanceHandle.IsValid())
	{
		return InstanceHandle.GetManagerChecked().GetMassEntityManager();
	}

	return TSharedPtr<FMassEntityManager>();
}

FMassEntityManager& UArsInstancedActorsComponent::GetMassEntityManagerChecked() const
{
	return InstanceHandle.GetManagerChecked().GetMassEntityManagerChecked();
}
