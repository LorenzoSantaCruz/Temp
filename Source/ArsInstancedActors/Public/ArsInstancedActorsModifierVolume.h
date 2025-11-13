// Copyright (c) 2024 Lorenzo Santa Cruz. All rights reserved.

#pragma once

#include "ArsMechanicaAPI.h"

#include "GameFramework/Actor.h"
#include "UObject/UObjectGlobals.h"
#include "ArsInstancedActorsModifierVolume.generated.h"

class UArsInstancedActorsModifierVolumeComponent;

/**
 * A 3D volume with a list of Modifiers to execute against any Instanced Actor's found within the volume.
 * @see UArsInstancedActorsModifierVolumeComponent
 */
UCLASS(MinimalAPI)
class ARSMECHANICA_API AArsInstancedActorsModifierVolume : public AActor
{
	GENERATED_BODY()

public:

	AArsInstancedActorsModifierVolume(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	FORCEINLINE UArsInstancedActorsModifierVolumeComponent* GetModifierVolumeComponent() const
	{
		return ModifierVolumeComponent;
	}

protected:
	
	UPROPERTY(Category = ArsInstancedActorsModifierVolume, VisibleAnywhere, BlueprintReadOnly, meta = (ExposeFunctionCategories = "Instanced Actor Modifier Volume", AllowPrivateAccess = "true"))
	TObjectPtr<UArsInstancedActorsModifierVolumeComponent> ModifierVolumeComponent;
};

/**
 * A 3D volume that performs filtered removal of Instanced Actor's found within the volume.
 * @see URemoveInstancesModifierVolumeComponent
 */
UCLASS(MinimalAPI)
class ARSMECHANICA_API AArsInstancedActorsRemovalModifierVolume : public AArsInstancedActorsModifierVolume
{
	GENERATED_BODY()

public:

	AArsInstancedActorsRemovalModifierVolume(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
