// Copyright (c) 2024 Lorenzo Santa Cruz. All rights reserved.

#pragma once

#include "ArsMechanicaAPI.h"

#include "ArsInstancedActorsDebug.h"
#if WITH_ARSINSTANCEDACTORS_DEBUG
#include "MassEntityQuery.h"
#endif // WITH_ARSINSTANCEDACTORS_DEBUG
#include "MassProcessor.h"
#include "ArsInstancedActorsDebugProcessor.generated.h"


UCLASS()
class ARSMECHANICA_API UArsInstancedActorsDebugProcessor : public UMassProcessor
{
	GENERATED_BODY()

protected:
	UArsInstancedActorsDebugProcessor();

	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;

	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

#if WITH_ARSINSTANCEDACTORS_DEBUG
	/**
	 *  The query used when drawing instanced actors at Detailed batch LOD 
	 *  (see UE::ArsInstancedActors::Debug::bDebugDrawDetailedCurrentRepresentation)
	 */
	FMassEntityQuery DetailedLODEntityQuery;
	
	/** The query used when "debug all entities" is enabled (see UE::ArsInstancedActors::Debug::bDebugDrawAllEntities)*/
	FMassEntityQuery DebugAllEntityQuery;
#endif // WITH_ARSINSTANCEDACTORS_DEBUG
};
