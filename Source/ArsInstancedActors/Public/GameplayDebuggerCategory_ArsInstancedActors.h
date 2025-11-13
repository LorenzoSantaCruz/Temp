// Copyright (c) 2024 Lorenzo Santa Cruz. All rights reserved.

#pragma once

#include "ArsMechanicaAPI.h"

#include "ArsInstancedActorsDebug.h"
#include "MassEntityTypes.h"


#if WITH_GAMEPLAY_DEBUGGER && WITH_ARSINSTANCEDACTORS_DEBUG && WITH_MASSENTITY_DEBUG

#include "GameplayDebuggerCategory.h"
#include "HAL/IConsoleManager.h"
#include "MassEntityHandle.h"

struct FMassEntityManager;

/** Gameplay debugger used to debug Intanced Actors */
class ARSMECHANICA_API FGameplayDebuggerCategory_ArsInstancedActors : public FGameplayDebuggerCategory
{
public:
	FGameplayDebuggerCategory_ArsInstancedActors();
	~FGameplayDebuggerCategory_ArsInstancedActors();

	void SetCachedEntity(const FMassEntityHandle Entity, const FMassEntityManager& EntityManager);
	void OnEntitySelected(const FMassEntityManager& EntityManager, const FMassEntityHandle EntityHandle);
	void ClearCachedEntity();

	virtual void CollectData(APlayerController* OwnerPC, AActor* DebugActor) override;

	static TSharedRef<FGameplayDebuggerCategory> MakeInstance();

protected:

	static TArray<FAutoConsoleCommandWithWorld> ConsoleCommands;
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnConsoleCommandBroadcastDelegate, UWorld*);
	static FOnConsoleCommandBroadcastDelegate OnToggleDebugLocalIAMsBroadcast; 	

	using FDelegateHandlePair = TPair<FOnConsoleCommandBroadcastDelegate*, FDelegateHandle>;
	TArray<FDelegateHandlePair> ConsoleCommandHandles;

	int32 ToggleDebugLocalIAMsInputIndex = INDEX_NONE;

	void OnToggleDebugLocalIAMs();

private:

	bool bDebugLocalIAMs = false;

	AActor* CachedDebugActor = nullptr;
	FMassEntityHandle CachedEntity;

	FDelegateHandle OnEntitySelectedHandle;
};

#endif // FN_WITH_GAMEPLAY_DEBUGGER && WITH_ARSINSTANCEDACTORS_DEBUG && WITH_MASSENTITY_DEBUG
