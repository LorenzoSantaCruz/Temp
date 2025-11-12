// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Misc/Guid.h"

#define UE_API ARSINSTANCEDACTORS_API

struct FArsInstancedActorsCustomVersion
{
	enum Type
	{
		// Before any version changes were made in the plugin
		InitialVersion = 0,

		// -----<new versions can be added above this line>-------------------------------------------------
		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1
	};

	// The GUID for this custom version number
	UE_API const static FGuid GUID;

private:
	FArsInstancedActorsCustomVersion() {}
};

#undef UE_API
