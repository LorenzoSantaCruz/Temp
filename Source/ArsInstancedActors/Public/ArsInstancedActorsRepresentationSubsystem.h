// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ArsMechanicaAPI.h"

#include "MassRepresentationSubsystem.h"
#include "ArsInstancedActorsRepresentationSubsystem.generated.h"


UCLASS()
class ARSMECHANICA_API UArsInstancedActorsRepresentationSubsystem : public UMassRepresentationSubsystem
{
	GENERATED_BODY()

protected:
	//~ Begin USubsystem Overrides
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~ End USubsystem Overrides

	void OnSettingsChanged();

	FDelegateHandle OnSettingsChangedHandle;
};
