// Copyright MODogma. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "QuickActorActionsWidget.generated.h"

// For practice, only (random rotation is already implemented elsewhere
USTRUCT(BlueprintType)
struct FRandomActorRotation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Duplication"/*, meta = (InlineEditConditionToggle = "true") This allows toggle to affected properties without displaying the actual struct member name as a separate entry*/)
	bool bEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Duplication", meta = (EditCondition = "bEnabled"))
	float Min = -45.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Duplication", meta = (EditCondition = "bEnabled"))
	float Max = 45.f;
};

/**
 *
 */
UCLASS()
class UDEMYCOURSE_API UQuickActorActionsWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

public:
	//////////////
	// Selection
	//////////////
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection", meta = (ToolTip = "Search actor name by case."))
	TEnumAsByte<ESearchCase::Type> SearchCase = ESearchCase::IgnoreCase;

	UFUNCTION(BlueprintCallable, Category = "Selection", meta = (ToolTip = "Handy."))
	void SelectActorsWithSameName();

	//////////////
	// Duplication
	//////////////
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Duplication", meta = (ToolTip = "Specify the amount of actor duplicates."))
	int32 NumberOfDuplicates = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Duplication", meta = (ToolTip = "The distance and axis to offset each duplicate actor (in xyz).\nA non-zero value is required, or else the duplication will not execute."))
	FVector OffsetDistance = FVector(0.f, 0.f, 0.f);

	FTransform RandomActorTransform = FTransform(FRotator(0.f, 0.f, 0.f), FVector(0.f, 0.f, 0.f), FVector(0.f, 0.f, 0.f));

	/** Controls random input blah code tooltip */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Duplication", meta = (ToolTip = "Add a random rotation to each duplicated actor.\nThe rotation range is clamped to the input value for each axis (in xyz),\ni.e. {0, 0, 360} allows for a z-rotation of up to 360 degrees."))
	FRotator RandomRotation = FRotator(0.f, 0.f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Duplication", meta = (ToolTip = "Random min scale to apply to selected actors."))
	FVector RandomScaleMin = FVector(1.f, 1.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Duplication", meta = (ToolTip = "Random max scale to apply to selected actors."))
	FVector RandomScaleMax = FVector(1.f, 1.f, 1.f);

	UFUNCTION(BlueprintCallable, Category = "Duplication", meta = (ToolTip = "Testing."))
	void DuplicateSelectedActors();

	UFUNCTION(BlueprintCallable, Category = "Duplication", meta = (ToolTip = "Apply random rotation to input actors."))
	void RandomizeActorRotation(AActor* InSelectedActor);

	UFUNCTION(BlueprintCallable, Category = "Duplication", meta = (ToolTip = "Apply random scale to selected actors."))
	void RandomizeActorScale(AActor* InSelectedActor);

	UFUNCTION(BlueprintCallable, Category = "Duplication", meta = (ToolTip = "Apply random offset to selected actors."))
	void RandomizeActorOffset(AActor* InSelectedActor);

	UFUNCTION(BlueprintCallable, Category = "Duplication", meta = (ToolTip = "Apply random transform to selected actors."))
	void RandomizeSelectedActorsTransform();


	// For coding practice only...Unused as there was a better solution implemented
	UFUNCTION(BlueprintCallable, Category = "Duplication", meta = (ToolTip = "NON-IMPLEMENTED: test codin only."))
	void RandomizeSelectedActorsRotationX();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Duplication", meta = (ToolTip = "NON-IMPLEMENTED: test coding only."))
	FRandomActorRotation RandomRotationYaw;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Duplication", meta = (ToolTip = "NON-IMPLEMENTED: test coding only."))
	FRandomActorRotation RandomRotationPitch;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Duplication", meta = (ToolTip = "NON-IMPLEMENTED: test coding only."))
	FRandomActorRotation RandomRotationRoll;

private:
	/** For acessing the editor actor subsytem */
	UPROPERTY()
	class UEditorActorSubsystem* EditorActorSubsystem;

	/** Returns true if subsystem get is valid */
	bool GetEditorActorSubsystem();
};