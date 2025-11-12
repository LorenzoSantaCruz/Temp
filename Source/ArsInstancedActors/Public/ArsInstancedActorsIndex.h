// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UObject/ObjectMacros.h"
#include "Serialization/StructuredArchive.h"
#include "ArsInstancedActorsIndex.generated.h"

#define UE_API ARSINSTANCEDACTORS_API


class UArsInstancedActorsData;
class AArsInstancedActorsManager;
struct FArsInstancedActorsIterationContext;

/** This type is only valid to be used with the instance of UArsInstancedActorsData it applies to. */
USTRUCT()
struct FArsInstancedActorsInstanceIndex
{
	GENERATED_BODY()
	
	FArsInstancedActorsInstanceIndex() = default;
	explicit FArsInstancedActorsInstanceIndex(const int32 InIndex) : Index((uint16)InIndex) 
	{ 
		check((InIndex >= INDEX_NONE) && (InIndex < (MAX_uint16 - 1))); // -1 for INDEX_NONE
	}

	friend FArchive& operator<<(FArchive& Ar, FArsInstancedActorsInstanceIndex& InstanceIndex)
	{
		Ar << InstanceIndex.Index;
		return Ar;
	}

	friend void operator<<(FStructuredArchive::FSlot Slot, FArsInstancedActorsInstanceIndex& InstanceIndex)
	{
		Slot << InstanceIndex.Index;
	}

	friend uint32 GetTypeHash(const FArsInstancedActorsInstanceIndex& InstanceIndex)
	{
		return uint32(InstanceIndex.Index);
	}

	bool IsValid() const { return Index != INDEX_NONE; }

	/** Returns a string suitable for debug logging to identify this instance. */
	UE_API FString GetDebugName() const;

	bool operator==(const FArsInstancedActorsInstanceIndex&) const = default;

	int32 GetIndex() const { return Index; }

	static constexpr FORCEINLINE int32 BuildCompositeIndex(const uint16 InstanceDataID, const int32 InstanceIndex)
	{
		const uint32 HighBits = InstanceDataID;
		const uint32 LowBits = InstanceIndex;
		check(LowBits <= MAX_uint16);
		return static_cast<int32>((HighBits << InstanceIndexBits) | LowBits);
	}

	static constexpr FORCEINLINE int32 ExtractInstanceDataID(const int32 CompositeIndex)
	{
		return CompositeIndex >> InstanceIndexBits;
	}

	static constexpr FORCEINLINE int32 ExtractInternalInstanceIndex(const int32 CompositeIndex)
	{
		return (CompositeIndex & InstanceIndexMask);
	}

private:
	/** Stable(consistent between client and server) instance index into UArsInstancedActorsData */
	UPROPERTY()
	uint16 Index = INDEX_NONE;

	static constexpr int32 InstanceIndexBits = 16;
	static constexpr int32 InstanceIndexMask = (1 << InstanceIndexBits) - 1;
};

template<>
struct TStructOpsTypeTraits<FArsInstancedActorsInstanceIndex> : public TStructOpsTypeTraitsBase2<FArsInstancedActorsInstanceIndex>
{
	enum
	{
		WithZeroConstructor = true,
		WithCopy = true,
		WithIdenticalViaEquality = true,
	};
};

USTRUCT(BlueprintType)
struct FArsInstancedActorsInstanceHandle
{
	GENERATED_BODY()

	FArsInstancedActorsInstanceHandle() = default;
	UE_API FArsInstancedActorsInstanceHandle(UArsInstancedActorsData& InInstancedActorData, FArsInstancedActorsInstanceIndex InIndex);

	UArsInstancedActorsData* GetInstanceActorData() const { return InstancedActorData.Get(); }
	UE_API UArsInstancedActorsData& GetInstanceActorDataChecked() const;

	UE_API AArsInstancedActorsManager* GetManager() const;
	UE_API AArsInstancedActorsManager& GetManagerChecked() const;

	UE_API bool IsValid() const;

	/** Returns a string suitable for debug logging to identify this instance. */
	UE_API FString GetDebugName() const;

	bool operator==(const FArsInstancedActorsInstanceHandle&) const = default;
	FArsInstancedActorsInstanceIndex GetInstanceIndex() const { return Index; }
	int32 GetIndex() const { return Index.GetIndex(); }

	void Reset()
	{
		InstancedActorData = nullptr;
		Index = FArsInstancedActorsInstanceIndex();
	}

private:

	friend ARSINSTANCEDACTORS_API uint32 GetTypeHash(const FArsInstancedActorsInstanceHandle& InstanceHandle);
	friend UArsInstancedActorsData;
	friend AArsInstancedActorsManager;
	friend FArsInstancedActorsIterationContext;

	/** Specific UArsInstancedActorsData responsible for this instance.Can be used to get to it's owning Manager. */
	UPROPERTY()
	TWeakObjectPtr<UArsInstancedActorsData> InstancedActorData = nullptr;

	/** Stable(consistent between client and server) instance index into UArsInstancedActorsData. */
	UPROPERTY()
	FArsInstancedActorsInstanceIndex Index;
};

#undef UE_API
