// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ArsMechanicaAPI.h"

#include "MassProcessor.h"
#include "MassRepresentationTypes.h"
#include "ArsInstancedActorsVisualizationSwitcherProcessor.generated.h"


struct FMassRepresentationFragment;

/**
 * Executes on entities with FArsInstancedActorsMeshSwitchFragment's, processing them as `pending requests` to switch to
 * the specified NewStaticMeshDescHandle, then removing the fragments once complete
 */
UCLASS(MinimalAPI)
class ARSMECHANICA_API UArsInstancedActorsVisualizationSwitcherProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UArsInstancedActorsVisualizationSwitcherProcessor();

	// Switch EntityHandle to NewStaticMeshDescHandle by removing any current / previous ISMC instances for the current StaticMeshDescIHandle
	// and setting RepresentationFragment.PrevRepresentation = EMassRepresentationType::None to let the subsequent
	// UMassStationaryISMSwitcherProcessor see that a new isntance needs to be added for the now set NewStaticMeshDescHandle
	static void SwitchEntityMeshDesc(FMassInstancedStaticMeshInfoArrayView& ISMInfosView, FMassRepresentationFragment& RepresentationFragment, FMassEntityHandle EntityHandle, FStaticMeshInstanceVisualizationDescHandle NewStaticMeshDescHandle);

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
