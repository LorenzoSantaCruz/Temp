// Copyright (c) 2024 Lorenzo Santa Cruz. All rights reserved.

#pragma once

#include "ArsMechanicaAPI.h"

#include "MassRepresentationActorManagement.h"
#include "ArsInstancedActorsRepresentationActorManagement.generated.h"


enum class EUpdateTransformFlags : int32;
enum class ETeleportType : uint8;
struct FMassEntityView;

UCLASS(MinimalAPI)
class ARSMECHANICA_API UArsInstancedActorsRepresentationActorManagement : public UMassRepresentationActorManagement
{
	GENERATED_BODY()

public:
	AActor* FindOrInstantlySpawnActor(UMassRepresentationSubsystem& RepresentationSubsystem, FMassEntityManager& EntityManager, FMassEntityView& EntityView);

	// Called by UArsInstancedActorsData::UnlinkActor to cleanup actor delegate callbacks
	virtual void OnActorUnlinked(AActor& Actor);

protected:
	//~ Begin UMassRepresentationActorManagement Overrides
	virtual EMassActorSpawnRequestAction OnPostActorSpawn(const FMassActorSpawnRequestHandle& SpawnRequestHandle
		, FConstStructView SpawnRequest, TSharedRef<FMassEntityManager> EntityManager) const override;

	// @todo make this behavior configurable
	/** Overriding to make sure ticking doesn't get enabled on spawned actors - we don't want that for handled actors */
	virtual void SetActorEnabled(const EMassActorEnabledType EnabledType, AActor& Actor, const int32 EntityIdx
		, FMassCommandBuffer& CommandBuffer) const override;

	/** Overridden to skip transform updates for replicated actors on clients */
	virtual void TeleportActor(const FTransform& Transform, AActor& Actor, FMassCommandBuffer& CommandBuffer) const;
	//~ End UMassRepresentationActorManagement Overrides

	void OnSpawnedActorDestroyed(AActor& DestroyedActor, FMassEntityHandle EntityHandle) const;
	void OnSpawnedActorMoved(USceneComponent* MovedActorRootComponent, EUpdateTransformFlags TransformUpdateFlags, ETeleportType TeleportType, FMassEntityHandle EntityHandle) const;
};
