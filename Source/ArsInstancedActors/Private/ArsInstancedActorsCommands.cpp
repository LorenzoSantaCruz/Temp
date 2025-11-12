// Copyright Epic Games, Inc. All Rights Reserved.

#include "ArsInstancedActorsCommands.h"
#include "ArsInstancedActorsDebug.h"
#include "MassEntityTypes.h"
#include "MassLODFragments.h"
#include "MassDistanceLODProcessor.h"
#include "MassStationaryISMSwitcherProcessor.h"
#include "ArsInstancedActorsVisualizationProcessor.h"

#ifndef ARSINSTANCEDACTORS_AS_SMARTOBJECTS
#define ARSINSTANCEDACTORS_AS_SMARTOBJECTS 1
#endif // ARSINSTANCEDACTORS_AS_SMARTOBJECTS

#if ARSINSTANCEDACTORS_AS_SMARTOBJECTS
#include "MassSmartObjectRegistration.h"
#endif // ARSINSTANCEDACTORS_AS_SMARTOBJECTS

#if CSV_PROFILER_STATS || WITH_ARSINSTANCEDACTORS_DEBUG
#	define DEBUG_NAME(Name) , FName(TEXT(Name))
#else
#	define DEBUG_NAME(Name)
#endif // CSV_PROFILER_STATS || WITH_MASSENTITY_DEBUG

namespace UE::ArsInstancedActors
{	
	FMassTagBitSet& GetDetailedLODTags()
	{
		static FMassTagBitSet DetailedLODTags = UE::Mass::Utils::ConstructTagBitSet<EMassCommandCheckTime::CompileTimeCheck
			, FMassDistanceLODProcessorTag				// UMassDistanceLODProcessor requirement
			, FMassCollectDistanceLODViewerInfoTag		// UMassLODDistanceCollectorProcessor requirement
			, FMassStationaryISMSwitcherProcessorTag	// UMassStationaryISMSwitcherProcessor requirement
			, FArsInstancedActorsVisualizationProcessorTag	// UArsInstancedActorsVisualizationProcessor requirement
#if ARSINSTANCEDACTORS_AS_SMARTOBJECTS
			, FMassInActiveSmartObjectsRangeTag
#endif // ARSINSTANCEDACTORS_AS_SMARTOBJECTS
		>();

		return DetailedLODTags;
	}

	FEnableDetailedLODCommand::FEnableDetailedLODCommand()
		: Super(EMassCommandOperationType::Add
			, GetDetailedLODTags()
			, FMassTagBitSet{}
			DEBUG_NAME("DetailedLODEnable"))
	{
	}

	FEnableBatchLODCommand::FEnableBatchLODCommand()
		: Super(EMassCommandOperationType::Remove
			, FMassTagBitSet{}
			, GetDetailedLODTags()
			DEBUG_NAME("BatchLODEnable"))
	{
	}
} // UE::ArsInstancedActors

#undef DEBUG_NAME
