// Copyright Epic Games, Inc. All Rights Reserved.

#include "ArsInstancedActorsCustomVersion.h"

#include "Serialization/CustomVersion.h"

const FGuid FArsInstancedActorsCustomVersion::GUID(0xA6EBC74A, 0x3BAD46B8, 0xAA30D6B2, 0x7D58EA25);

// Register the custom version with core
FCustomVersionRegistration GRegisterArsInstancedActorsCustomVersion(FArsInstancedActorsCustomVersion::GUID, FArsInstancedActorsCustomVersion::LatestVersion, TEXT("ArsInstancedActors"));
