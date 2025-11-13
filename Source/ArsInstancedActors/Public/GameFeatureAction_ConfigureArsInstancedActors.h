// Copyright (c) 2024 Lorenzo Santa Cruz. All rights reserved.

#pragma once

#include "ArsMechanicaAPI.h"

#include "GameFeatureAction.h"
#include "ArsInstancedActorsSettings.h"
#include "GameFeatureAction_ConfigureArsInstancedActors.generated.h"


/** 
 * GameplayFeature Action carrying overrides to ArsInstancedActors settings
 */
UCLASS(meta = (DisplayName = "Configure ArsInstancedActors"))
class ARSMECHANICA_API UGameFeatureAction_ConfigureArsInstancedActors : public UGameFeatureAction
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

