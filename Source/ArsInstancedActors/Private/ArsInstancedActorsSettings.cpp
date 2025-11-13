// Copyright (c) 2024 Lorenzo Santa Cruz. All rights reserved.

#include "ArsInstancedActorsSettings.h"
#include "ClientArsInstancedActorsSpawnerSubsystem.h"
#include "ServerArsInstancedActorsSpawnerSubsystem.h"
#include "ArsInstancedActorsSubsystem.h"
#include "ArsInstancedActorsVisualizationTrait.h"


//-----------------------------------------------------------------------------
// UArsInstancedActorsProjectSettings
//-----------------------------------------------------------------------------
UArsInstancedActorsProjectSettings::UArsInstancedActorsProjectSettings()
{
	DefaultConfig.ServerActorSpawnerSubsystemClass = UServerArsInstancedActorsSpawnerSubsystem::StaticClass();
	DefaultConfig.ClientActorSpawnerSubsystemClass = UClientArsInstancedActorsSpawnerSubsystem::StaticClass();
	DefaultConfig.ArsInstancedActorsSubsystemClass = UArsInstancedActorsSubsystem::StaticClass();
	DefaultConfig.StationaryVisualizationTraitClass = UArsInstancedActorsVisualizationTrait::StaticClass();

	CompiledActiveConfig = DefaultConfig;
}

void UArsInstancedActorsProjectSettings::PostInitProperties()
{
	Super::PostInitProperties();
	CompiledActiveConfig = DefaultConfig;
}

#if WITH_EDITOR
void UArsInstancedActorsProjectSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	if (ClassConfigOverrides.IsEmpty())
	{
		CompiledActiveConfig = DefaultConfig;
	}
	else
	{
		CompileSettings();
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

TSubclassOf<UMassActorSpawnerSubsystem> UArsInstancedActorsProjectSettings::GetServerActorSpawnerSubsystemClass() const 
{ 
	return CompiledActiveConfig.ServerActorSpawnerSubsystemClass;
}

TSubclassOf<UMassActorSpawnerSubsystem> UArsInstancedActorsProjectSettings::GetClientActorSpawnerSubsystemClass() const 
{
	return CompiledActiveConfig.ClientActorSpawnerSubsystemClass;
}

TSubclassOf<UArsInstancedActorsSubsystem> UArsInstancedActorsProjectSettings::GetArsInstancedActorsSubsystemClass() const 
{
	return CompiledActiveConfig.ArsInstancedActorsSubsystemClass;
}

TSubclassOf<UMassStationaryDistanceVisualizationTrait> UArsInstancedActorsProjectSettings::GetStationaryVisualizationTraitClass() const
{
	return CompiledActiveConfig.StationaryVisualizationTraitClass;
}

void UArsInstancedActorsProjectSettings::RegisterConfigOverride(UObject& Owner, const FArsInstancedActorsConfig& Config)
{
	FClassConfigOverrideEntry* Entry = ClassConfigOverrides.FindByPredicate([OwnerKey = &Owner](const FClassConfigOverrideEntry& Entry)
		{ 
			return Entry.Owner == OwnerKey;
		});
	if (Entry)
	{
		Entry->ConfigOverride = Config;
	}
	else
	{
		ClassConfigOverrides.Add({ &Owner, Config });
	}
	CompileSettings();
}

void UArsInstancedActorsProjectSettings::UnregisterConfigOverride(UObject& Owner)
{
	const int32 NumRemoved = ClassConfigOverrides.RemoveAll([OwnerKey = &Owner](const FClassConfigOverrideEntry& Entry)
		{
			return Entry.Owner == OwnerKey;
		});
	
	if (NumRemoved)
	{
		CompileSettings();
	}
}

/** 
 * A helper macro for applying overrides to the specified Property. Note that we iterate starting from the latest
 * override and quit as soon as a valid override is found.
 */
#define APPLY_OVERRIDE(Config, Property) \
	for (int32 OverrideIndex = ClassConfigOverrides.Num() - 1; OverrideIndex >=0; --OverrideIndex) \
	{ \
		const FClassConfigOverrideEntry& Entry = ClassConfigOverrides[OverrideIndex]; \
		if (Entry.ConfigOverride.Property) \
		{ \
			Config.Property = Entry.ConfigOverride.Property; \
			break; \
		} \
	} \

void UArsInstancedActorsProjectSettings::CompileSettings()
{
	CompiledActiveConfig = DefaultConfig;

	APPLY_OVERRIDE(CompiledActiveConfig, ServerActorSpawnerSubsystemClass);
	APPLY_OVERRIDE(CompiledActiveConfig, ClientActorSpawnerSubsystemClass);
	APPLY_OVERRIDE(CompiledActiveConfig, ArsInstancedActorsSubsystemClass);
	APPLY_OVERRIDE(CompiledActiveConfig, StationaryVisualizationTraitClass);

	OnSettingsUpdated.Broadcast();
}

#undef APPLY_OVERRIDE
