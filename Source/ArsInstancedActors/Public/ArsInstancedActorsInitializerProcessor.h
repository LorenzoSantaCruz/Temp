// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ArsMechanicaAPI.h"

#include "MassProcessor.h"
#include "ArsInstancedActorsInitializerProcessor.generated.h"


class UArsInstancedActorsData;

USTRUCT()
struct FArsInstancedActorsMassSpawnData
{
	GENERATED_BODY()

	TWeakObjectPtr<UArsInstancedActorsData> InstanceData;
};

/** Initializes the fragments of all entities that fit the query specified in ConfigureQueries, which are all considered Instanced Actors. */
UCLASS()
class ARSMECHANICA_API UArsInstancedActorsInitializerProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UArsInstancedActorsInitializerProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
