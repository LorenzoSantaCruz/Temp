// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MassStationaryDistanceVisualizationTrait.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "ArsInstancedActorsVisualizationTrait.generated.h"


class UArsInstancedActorsData;

/** 
 * Subclass of UMassStationaryVisualizationTrait which forces required settings for instanced actor entities and overrides
 * FMassRepresentationFragment.StaticMeshDescHandle to use a custom registered Visualization which reuses InstanceData's 
 * ISMComponents via UMassRepresentationSubsystem::AddVisualDescWithISMComponent.
 * Note that the trait is marked to not show up in class selection drop-downs. The reason is that this trait is supposed
 * to be used internally by ArsInstancedActors and is never expected to be a part of a user-authored entity config.
 */
UCLASS(MinimalAPI, HideDropdown)
class UArsInstancedActorsVisualizationTrait : public UMassStationaryDistanceVisualizationTrait
{
	GENERATED_BODY()

public:

	ARSINSTANCEDACTORS_API UArsInstancedActorsVisualizationTrait(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	ARSINSTANCEDACTORS_API virtual void InitializeFromInstanceData(UArsInstancedActorsData& InInstanceData);

	//~ Begin UArsInstancedActorsVisualizationTrait Overrides
	ARSINSTANCEDACTORS_API virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
	//~ End UArsInstancedActorsVisualizationTrait Overrides

protected:
	
	UPROPERTY(Transient)
	TWeakObjectPtr<UArsInstancedActorsData> InstanceData;
};
