﻿#pragma once

#include "Engine/DataAsset.h"
#include "Engine/EngineTypes.h"
#include "Engine/NetSerialization.h"
#include "AlsMantlingSettings.generated.h"

class UAnimMontage;
class UCurveFloat;
class UCurveVector;

UENUM(BlueprintType)
enum class EAlsMantlingType : uint8
{
	High,
	Low,
	InAir
};

USTRUCT(BlueprintType)
struct ALS_API FAlsMantlingParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	TWeakObjectPtr<UPrimitiveComponent> TargetPrimitive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	FVector_NetQuantize100 TargetRelativeLocation{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	FRotator TargetRelativeRotation{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ForceUnits = "cm"))
	float MantlingHeight{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	EAlsMantlingType MantlingType{EAlsMantlingType::High};
};

UCLASS(Blueprintable, BlueprintType)
class ALS_API UAlsMantlingSettings : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	TObjectPtr<UAnimMontage> Montage;

	// Mantling time to blend in amount curve.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", Meta = (ClampMin = 0, EditCondition = "!bUseMontageBlendIn"))
	TObjectPtr<UCurveFloat> BlendInCurve;

	// If checked, mantling will use the blend in curve from the animation montage instead of from this asset.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", Meta = (ClampMin = 0))
	bool bUseMontageBlendIn{true};

	// Mantling time to interpolation, horizontal and vertical correction amounts curve.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	TObjectPtr<UCurveVector> InterpolationAndCorrectionAmountsCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FVector3f StartRelativeLocation{-65.0f, 0.0f, -100.0f};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", Meta = (ClampMin = 0))
	FVector2f ReferenceHeight{50.0f, 100.0f};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", Meta = (ClampMin = 0, EditCondition = "!bAutoCalculateStartTime"))
	FVector2f StartTime{0.5f, 0.0f};

	// If checked, mantling will automatically calculate the start time based on how much vertical
	// distance the character needs to move to reach the object they are about to mantle.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", Meta = (ClampMin = 0))
	bool bAutoCalculateStartTime{false};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", Meta = (ClampMin = 0))
	FVector2f PlayRate{1.0f, 1.0f};

public:
	float GetStartTimeByHeight(float MantlingHeight) const;

	float GetPlayRateByHeight(float MantlingHeight) const;
};

USTRUCT(BlueprintType)
struct ALS_API FAlsMantlingTraceSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0))
	FVector2f LedgeHeight{50.0f, 225.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ForceUnits = "cm"))
	float ReachDistance{75.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ForceUnits = "cm"))
	float TargetLocationOffset{15.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0))
	bool bDrawFailedTraces{false};
};

USTRUCT(BlueprintType)
struct ALS_API FAlsGeneralMantlingSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	bool bAllowMantling{true};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ClampMax = 180, ForceUnits = "deg"))
	float TraceAngleThreshold{110.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ClampMax = 180, ForceUnits = "deg"))
	float MaxReachAngle{50.0f};

	// If a dynamic object has a speed bigger than this value, then do not start mantling.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ForceUnits = "cm/s"))
	float TargetPrimitiveSpeedThreshold{10.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ForceUnits = "cm"))
	float MantlingHighHeightThreshold{125.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	FAlsMantlingTraceSettings GroundedTrace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	FAlsMantlingTraceSettings InAirTrace{{50.0f, 150.0f}, 70.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	TEnumAsByte<ECollisionChannel> MantlingTraceChannel{ECC_Visibility};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	TArray<TEnumAsByte<ECollisionChannel>> MantlingTraceResponseChannels;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS")
	FCollisionResponseContainer MantlingTraceResponses{ECR_Ignore};

	// Used when the mantling was interrupted and we need to stop the animation.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", Meta = (ClampMin = 0, ForceUnits = "s"))
	float BlendOutDuration{0.3f};

	// If checked, ragdolling will start if the object the character is mantling on was destroyed.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	bool bStartRagdollingOnTargetPrimitiveDestruction{true};

public:
#if WITH_EDITOR
	void PostEditChangeProperty(const FPropertyChangedEvent& PropertyChangedEvent);
#endif
};

inline float UAlsMantlingSettings::GetStartTimeByHeight(const float MantlingHeight) const
{
	return FMath::GetMappedRangeValueClamped(ReferenceHeight, StartTime, MantlingHeight);
}

inline float UAlsMantlingSettings::GetPlayRateByHeight(const float MantlingHeight) const
{
	return FMath::GetMappedRangeValueClamped(ReferenceHeight, PlayRate, MantlingHeight);
}
