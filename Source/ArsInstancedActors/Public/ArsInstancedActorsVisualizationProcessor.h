// Copyright (c) 2024 Lorenzo Santa Cruz. All rights reserved.

#pragma once

#include "ArsMechanicaAPI.h"

#include "MassRepresentationProcessor.h"
#include "ArsInstancedActorsVisualizationProcessor.generated.h"


/**
 * Tag required by Instanced Actors Visualization Processor to process given archetype. Removing the tag allows to support
 * disabling of processing for individual entities of given archetype.
 */
USTRUCT()
struct FArsInstancedActorsVisualizationProcessorTag : public FMassTag
{
	GENERATED_BODY();
};

UCLASS()
class ARSMECHANICA_API UArsInstancedActorsVisualizationProcessor : public UMassVisualizationProcessor
{
	GENERATED_BODY()

protected:
	UArsInstancedActorsVisualizationProcessor();
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
};
