// Copyright (c) 2024 Lorenzo Santa Cruz. All rights reserved.

#include "ArsInstancedActorsVisualizationProcessor.h"
#include "MassStationaryISMSwitcherProcessor.h"
#include "ArsInstancedActorsTypes.h"


UArsInstancedActorsVisualizationProcessor::UArsInstancedActorsVisualizationProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::AllNetModes);
	// This processor needs to be executed before UMassStationaryISMSwitcherProcessors since that's the processor
	// responsible for executing what UArsInstancedActorsVisualizationProcessor calculates.
	// Missing this dependency would result in client-side one-frame representation absence when switching
	// from actor representation back to ISM.
	ExecutionOrder.ExecuteBefore.Add(UMassStationaryISMSwitcherProcessor::StaticClass()->GetFName());

	UpdateParams.bTestCollisionAvailibilityForActorVisualization = false;
}

void UArsInstancedActorsVisualizationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) 
{
	Super::ConfigureQueries(EntityManager);

	EntityQuery.ClearTagRequirements(FMassTagBitSet(*FMassVisualizationProcessorTag::StaticStruct()));
	EntityQuery.AddTagRequirement<FArsInstancedActorsVisualizationProcessorTag>(EMassFragmentPresence::All);
}