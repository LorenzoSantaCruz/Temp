// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ArsMechanicaAPI.h"

#include "MassProcessor.h"
#include "ArsInstancedActorsTypes.h"
#include "StructUtils/SharedStruct.h"
#include "ArsInstancedActorsStationaryLODBatchProcessor.generated.h"


UCLASS()
class ARSMECHANICA_API UArsInstancedActorsStationaryLODBatchProcessor : public UMassProcessor
{
	GENERATED_BODY()
	
public:
	UArsInstancedActorsStationaryLODBatchProcessor();

protected:
	virtual bool ShouldAllowQueryBasedPruning(const bool bRuntimeMode = true) const override { return false; }
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery LODChangingEntityQuery;
	FMassEntityQuery DirtyVisualizationEntityQuery;

	UPROPERTY(EditDefaultsOnly, Category="Mass", config)
	double DelayPerBulkLOD[(int)EArsInstancedActorsBulkLOD::MAX];
};
