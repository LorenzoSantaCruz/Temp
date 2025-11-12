// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFeatureAction.h"
#include "ArsInstancedActorsSettings.h"
#include "GameFeatureAction_ConfigureArsInstancedActors.generated.h"


/** 
 * GameplayFeature Action carrying overrides to ArsInstancedActors settings
 */
UCLASS(meta = (DisplayName = "Configure ArsInstancedActors"))
class UGameFeatureAction_ConfigureArsInstancedActors : public UGameFeatureAction
{
	GENERATED_BODY()

public:	
	//~ Begin UGameFeatureAction interface
	virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
	//~ End UGameFeatureAction interface

private:

	UPROPERTY(EditAnywhere, Category=ArsInstancedActors)
	FArsInstancedActorsConfig ConfigOverride;
};

