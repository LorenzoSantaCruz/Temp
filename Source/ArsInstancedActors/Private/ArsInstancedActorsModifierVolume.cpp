// Copyright (c) 2024 Lorenzo Santa Cruz. All rights reserved.

#include "ArsInstancedActorsModifierVolume.h"
#include "ArsInstancedActorsModifierVolumeComponent.h"


//-----------------------------------------------------------------------------
// AArsInstancedActorsModifierVolume
//-----------------------------------------------------------------------------
AArsInstancedActorsModifierVolume::AArsInstancedActorsModifierVolume(const FObjectInitializer& ObjectInitializer)
{
	ModifierVolumeComponent = CreateDefaultSubobject<UArsInstancedActorsModifierVolumeComponent>(TEXT("ModifierVolume"));
	ModifierVolumeComponent->Extent = FVector(50.0f);
	ModifierVolumeComponent->Radius = 50.0f;
	RootComponent = ModifierVolumeComponent;
}

//-----------------------------------------------------------------------------
// AArsInstancedActorsRemovalModifierVolume
//-----------------------------------------------------------------------------
AArsInstancedActorsRemovalModifierVolume::AArsInstancedActorsRemovalModifierVolume(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<URemoveInstancesModifierVolumeComponent>(TEXT("ModifierVolume")))
{
	ModifierVolumeComponent->Color = FColor::Red;
}
