// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameFeatureAction_ConfigureArsInstancedActors.h"
#include "ArsInstancedActorsSettings.h"
#include "ArsInstancedActorsSubsystem.h"
#include "GameFeaturesSubsystem.h"


void UGameFeatureAction_ConfigureArsInstancedActors::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	Super::OnGameFeatureActivating(Context);

	GetMutableDefault<UArsInstancedActorsProjectSettings>()->RegisterConfigOverride(*this, ConfigOverride);
}

void UGameFeatureAction_ConfigureArsInstancedActors::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	GetMutableDefault<UArsInstancedActorsProjectSettings>()->UnregisterConfigOverride(*this);

	Super::OnGameFeatureDeactivating(Context);
}
