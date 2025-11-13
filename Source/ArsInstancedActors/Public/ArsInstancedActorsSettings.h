// Copyright (c) 2024 Lorenzo Santa Cruz. All rights reserved.

#pragma once

#include "ArsMechanicaAPI.h"

#include "DataRegistryId.h"
#include "Engine/DeveloperSettings.h"
#include "UObject/SoftObjectPtr.h"
#include "MassActorSpawnerSubsystem.h"
#include "ArsInstancedActorsSubsystem.h"
#include "MassStationaryDistanceVisualizationTrait.h"
#include "ArsInstancedActorsSettings.generated.h"


#define GET_ARSINSTANCEDACTORS_CONFIG_VALUE(a) (GetMutableDefault<UArsInstancedActorsProjectSettings>()->a)

USTRUCT()
struct FArsInstancedActorsConfig
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, Category = Subsystems, NoClear, meta = (MetaClass = "/Script/MassActors.MassActorSpawnerSubsystem"))
	TSubclassOf<UMassActorSpawnerSubsystem> ServerActorSpawnerSubsystemClass; 

	UPROPERTY(Config, EditAnywhere, Category = Subsystems, NoClear, meta = (MetaClass = "/Script/MassActors.MassActorSpawnerSubsystem"))
	TSubclassOf<UMassActorSpawnerSubsystem> ClientActorSpawnerSubsystemClass;

	UPROPERTY(Config, EditAnywhere, Category = Subsystems, NoClear, meta = (MetaClass = "/Script/ArsInstancedActors.ArsInstancedActorsSubsystem"))
	TSubclassOf<UArsInstancedActorsSubsystem> ArsInstancedActorsSubsystemClass;

	UPROPERTY(Config, EditAnywhere, Category = Subsystems, NoClear, meta = (MetaClass = "/Script/MassRepresentation.MassStationaryDistanceVisualizationTrait"))
	TSubclassOf<UMassStationaryDistanceVisualizationTrait> StationaryVisualizationTraitClass;
};

USTRUCT()
struct FClassConfigOverrideEntry
{
	GENERATED_BODY()

	FObjectKey Owner;
	
	UPROPERTY()
	FArsInstancedActorsConfig ConfigOverride;
};

/** 
 * Configurable project settings for the Instanced Actors system.
 * @see FArsInstancedActorsClassSettingsBase and FArsInstancedActorsClassSettings for per-class specific runtime settings.
 * @see AArsInstancedActorsManager
 */
UCLASS(Config=ArsInstancedActors, defaultconfig, DisplayName = "Instanced Actors", MinimalAPI)
class ARSMECHANICA_API UArsInstancedActorsProjectSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE(FOnSettingsChanged);

	UArsInstancedActorsProjectSettings();

	TSubclassOf<UMassActorSpawnerSubsystem> GetServerActorSpawnerSubsystemClass() const;
	TSubclassOf<UMassActorSpawnerSubsystem> GetClientActorSpawnerSubsystemClass() const;
	TSubclassOf<UArsInstancedActorsSubsystem> GetArsInstancedActorsSubsystemClass() const;
	TSubclassOf<UMassStationaryDistanceVisualizationTrait> GetStationaryVisualizationTraitClass() const;

	void RegisterConfigOverride(UObject& Owner, const FArsInstancedActorsConfig& Config);
	void UnregisterConfigOverride(UObject& Owner);

	FOnSettingsChanged& GetOnSettingsUpdated() { return OnSettingsUpdated; }

protected:
	virtual void PostInitProperties() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	void CompileSettings();

public:
	/** 3D grid size (distance along side) for partitioned instanced actor managers */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, meta = (ClampMin="0", Units=cm), Category = Grid)
	int32 GridSize = 24480;

	/** Data Registry to gather 'named' FArsInstancedActorsSettings from during UArsInstancedActorsSubsystem init */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = ActorClassSettings)
	FDataRegistryType NamedSettingsRegistryType = "ArsInstancedActorsNamedSettings";

	/** Data Registry to gather per-class FArsInstancedActorsClassSettingsBase-based settings from during UArsInstancedActorsSubsystem init */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = ActorClassSettings)
	FDataRegistryType ActorClassSettingsRegistryType = "ArsInstancedActorsClassSettings";

	/**
	 * If specified, these named settings will be applied to the default settings used as the base settings set for all 
	 * others, with a lower precedence than any per-class overrides 
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = ActorClassSettings)
	FName DefaultBaseSettingsName = NAME_None;

	/** If specified, these named settings will be applied as a final set of overrides to all settings, overriding / taking precedence over all previous values */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = ActorClassSettings)
	FName EnforcedSettingsName = NAME_None;

	UPROPERTY(Config, EditAnywhere, Category = ActorClassSettings)
	FArsInstancedActorsConfig DefaultConfig;

protected:
	FOnSettingsChanged OnSettingsUpdated;

	/** Represents the current config combining DefaultConfig and all registered ClassConfigOverrides */
	UPROPERTY(Transient)
	FArsInstancedActorsConfig CompiledActiveConfig;
		
	UPROPERTY(Transient)
	TArray<FClassConfigOverrideEntry> ClassConfigOverrides;
};
