// Copyright Epic Games, Inc. All Rights Reserved.

#include "IArsInstancedActorsTestSuiteModule.h"

#define LOCTEXT_NAMESPACE "ArsInstancedActorsTestSuite"

class FArsInstancedActorsTestSuiteModule : public IArsInstancedActorsTestSuiteModule
{
};

IMPLEMENT_MODULE(FArsInstancedActorsTestSuiteModule, ArsInstancedActorsTestSuite)

#undef LOCTEXT_NAMESPACE
