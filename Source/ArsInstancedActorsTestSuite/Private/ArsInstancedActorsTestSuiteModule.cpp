// Copyright (c) 2024 Lorenzo Santa Cruz. All rights reserved.

#include "IArsInstancedActorsTestSuiteModule.h"

#define LOCTEXT_NAMESPACE "ArsInstancedActorsTestSuite"

class FArsInstancedActorsTestSuiteModule : public IArsInstancedActorsTestSuiteModule
{
};

IMPLEMENT_MODULE(FArsInstancedActorsTestSuiteModule, ArsInstancedActorsTestSuite)

#undef LOCTEXT_NAMESPACE
