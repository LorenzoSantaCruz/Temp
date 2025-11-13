// Copyright (c) 2024 Lorenzo Santa Cruz. All rights reserved.

#pragma once

#include "ArsMechanicaAPI.h"

#include "ArsInstancedActorsIndex.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ObjectPtr.h"
#include "ArsInstancedActorsIteration.generated.h"



class AArsInstancedActorsManager;
class UArsInstancedActorsData;
struct FArsInstancedActorsInstanceHandle;
struct FArsInstancedActorsInstanceIndex;

/**
 * Provides useful functionality while iterating instances like safe instance deletion.
 * @see AArsInstancedActorsManager::ForEachInstance
 */
USTRUCT()
struct FArsInstancedActorsIterationContext
{
	GENERATED_BODY()

	// Destructor to ensure no pending actions remain
	~FArsInstancedActorsIterationContext();

	/**
	 * Safely marks InstanceHandle for destruction at the end of iteration, to ensure iteration
	 * order isn't affected.
	 * Note: These deletions will NOT be be persisted as if a player had performed them, rather the deletions will
	 *       make it as if the items were never present.
	 * Note: This is safe to call before entity spawning as source instance data will simply be invalidated,
	 *       preventing later entity spawning.
	 */
	void RemoveInstanceDeferred(const FArsInstancedActorsInstanceHandle& InstanceHandle);

	/**
	 * Safely marks all instances in InstanceData for destruction at the end of iteration, to ensure iteration
	 * order isn't affected.
	 * Note: These deletions will NOT be be persisted as if a player had performed them, rather the deletions will
	 *       make it as if the items were never present.
	 * Note: This is safe to call before entity spawning as source instance data will simply be invalidated,
	 *       preventing later entity spawning.
	 */
	void RemoveAllInstancesDeferred(UArsInstancedActorsData& InstanceData);

	/**
	 * Safely marks all instances in Manager for destruction at the end of iteration, to ensure iteration
	 * order isn't affected.
	 * Note: These deletions will NOT be be persisted as if a player had performed them, rather the deletions will
	 *       make it as if the items were never present.
	 * Note: This is safe to call before entity spawning as source instance data will simply be invalidated,
	 *       preventing later entity spawning.
	 */
	void RemoveAllInstancesDeferred(AArsInstancedActorsManager& Manager);

	/** Perform deferred instance removals **/
	void FlushDeferredActions();

private:

	TMap<TObjectPtr<UArsInstancedActorsData>, TArray<FArsInstancedActorsInstanceIndex>> InstancesToRemove;
	TArray<TObjectPtr<UArsInstancedActorsData>> RemoveAllInstancesIADs;
	TArray<TObjectPtr<AArsInstancedActorsManager>> RemoveAllInstancesIAMs;
};

/** Subclass of FArsInstancedActorsIterationContext that calls FlushDeferredActions in it's destructor */
struct FScopedArsInstancedActorsIterationContext : public FArsInstancedActorsIterationContext
{
	~FScopedArsInstancedActorsIterationContext()
	{
		FlushDeferredActions();
	}
};

