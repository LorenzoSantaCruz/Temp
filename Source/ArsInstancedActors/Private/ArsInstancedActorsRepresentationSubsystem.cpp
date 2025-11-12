// Copyright Epic Games, Inc. All Rights Reserved.

#include "ArsInstancedActorsRepresentationSubsystem.h"
#include "ArsInstancedActorsSettings.h"


void UArsInstancedActorsRepresentationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	TSubclassOf<UMassActorSpawnerSubsystem> SpawnerSystemSubclass = UE::ArsInstancedActors::Utils::DetermineActorSpawnerSubsystemClass(GetWorldRef());
	if (SpawnerSystemSubclass)
	{
		ActorSpawnerSubsystem = Cast<UMassActorSpawnerSubsystem>(Collection.InitializeDependency(SpawnerSystemSubclass));

		ensureMsgf(ActorSpawnerSubsystem, TEXT("Trying to initialize dependency on class %s failed. Verify InstanedActors settings.")
			, *SpawnerSystemSubclass->GetName());
	}

	OnSettingsChangedHandle = GET_ARSINSTANCEDACTORS_CONFIG_VALUE(GetOnSettingsUpdated()).AddUObject(this, &UArsInstancedActorsRepresentationSubsystem::OnSettingsChanged);
}

void UArsInstancedActorsRepresentationSubsystem::Deinitialize()
{
	GET_ARSINSTANCEDACTORS_CONFIG_VALUE(GetOnSettingsUpdated()).Remove(OnSettingsChangedHandle);
	ActorSpawnerSubsystem = nullptr;

	Super::Deinitialize();
}

void UArsInstancedActorsRepresentationSubsystem::OnSettingsChanged()
{
	if (UWorld* World = GetWorld())
	{
		ActorSpawnerSubsystem = UE::ArsInstancedActors::Utils::GetActorSpawnerSubsystem(*World);
		UE_CLOG(ActorSpawnerSubsystem == nullptr, LogArsInstancedActors, Warning
			, TEXT("%s %hs failed to fetch ActorSpawnerSubsystem instance, class %s.")
			, *GetName(), __FUNCTION__, *GetNameSafe(UE::ArsInstancedActors::Utils::DetermineActorSpawnerSubsystemClass(*World)));
	}
}
