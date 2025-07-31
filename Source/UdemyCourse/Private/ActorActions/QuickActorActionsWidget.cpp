// Copyright MODogma. All Rights Reserved.

#include "ActorActions/QuickActorActionsWidget.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "DebugHeader.h"

#define LOCTEXT_NAMESPACE "QuickActorActions"

bool UQuickActorActionsWidget::GetEditorActorSubsystem()
{
	// Get the subsystem if it doesn't currently exist
	if (!EditorActorSubsystem)
	{
		EditorActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
	}

	return EditorActorSubsystem != nullptr;
}

void UQuickActorActionsWidget::SelectActorsWithSameName()
{
	if (!GetEditorActorSubsystem())
	{
		return;
	}

	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();

	if (SelectedActors.IsEmpty())
	{
		DebugHeader::ShowNotification(
			LOCTEXT("NoSelectionWarning", "No Actors are selected!"),
			ELogVerbosity::Warning
		);
		return;
	}
	else if (SelectedActors.Num() > 1)
	{
		DebugHeader::ShowNotification(
			LOCTEXT("NoSelectionWarning", "Select only one actor to use for the reference name!"),
			ELogVerbosity::Warning
		);
	}

	// Get name of the single actor
	FString SelectedActorName = SelectedActors[0]->GetActorLabel();
	FString NameToSearch = SelectedActorName;

	// This is not an elegant system, and is prone to failure...Epic's native select by type is better
	if (NameToSearch.Len() > 4)
	{
		// Remove last 4 characters to bias auto-numeration of duplicated actors, approximation for large actor counts (i.e. _999)
		NameToSearch = NameToSearch.LeftChop(4);
	}

	TArray<AActor*> AllLevelActors = EditorActorSubsystem->GetAllLevelActors();
	TArray<AActor*> ActorsToSelect;

	for (AActor* LevelActor : AllLevelActors)
	{
		if (!LevelActor)
		{
			continue;
		}

		if (LevelActor->GetActorLabel().Contains(NameToSearch, SearchCase))
		{
			//EditorActorSubsystem->SetActorSelectionState(LevelActor, true); // Works, but less efficient
			ActorsToSelect.Add(LevelActor);
		}
	}

	if (ActorsToSelect.IsEmpty())
	{
		DebugHeader::ShowNotification(
			LOCTEXT("NoMatchingNames", "No actors match the current name filter."),
			ELogVerbosity::Warning
		);
		return;
	}

	// More efficient--single selection update as-opposed to multiple
	EditorActorSubsystem->SetSelectedLevelActors(ActorsToSelect);
}

void UQuickActorActionsWidget::DuplicateSelectedActors()
{
	if (OffsetDistance == FVector(0, 0, 0) || NumberOfDuplicates == 0)
	{
		DebugHeader::ShowNotification(
			LOCTEXT("RequirementsUnfulfilled", "Offset distance and number of duplicates must be non-zero!"),
			ELogVerbosity::Warning
		);
		return;
	}

	if (!GetEditorActorSubsystem())
	{
		return;
	}

	UWorld* CurrentWorld = GEditor->GetWorld();

	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();

	if (SelectedActors.IsEmpty())
	{
		DebugHeader::ShowNotification(
			LOCTEXT("NoSelectedActors", "No actors selected!"),
			ELogVerbosity::Warning
		);
		return;
	}

	// Reset before each duplication
	FVector CurrentOffset = OffsetDistance;
	TArray<AActor*> DuplicatedActors;
	// Clamp the rotation to be predictably between 0 - 360.
	//RandomRotation = RandomRotation.Clamp(); // This causes 0 value for 360

	for (AActor* SelectedActor : SelectedActors)
	{
		for (int i = 0; i < NumberOfDuplicates; ++i)
		{
			AActor* DuplicatedActor = EditorActorSubsystem->DuplicateActor(SelectedActor, CurrentWorld, CurrentOffset);
			DuplicatedActors.Add(DuplicatedActor);
			CurrentOffset += OffsetDistance;

			if (RandomRotation == FRotator(0.f, 0.f, 0.f)) // FRotator::IsZero(RandomRotation), or NearlyZero also works
			{
				continue;
			}

			RandomizeActorRotation(DuplicatedActor);
		}
	}

	if (DuplicatedActors.IsEmpty())
	{
		DebugHeader::ShowNotification(
			LOCTEXT("DuplicationFailed", "Duplication has failed with no duplicates created."),
			ELogVerbosity::Error
		);
	}
}

void UQuickActorActionsWidget::RandomizeActorRotation(AActor* InSelectedActor)
{
	if (!InSelectedActor)
	{
		DebugHeader::ShowNotification(
			LOCTEXT("NoSelectionMade", "No actors are selected!"),
			ELogVerbosity::Warning
		);
		return/* FRotator()*/;
	}

	// FRotator is in Pitch, Yaw, Roll
	FRotator CurrentRandRotation = FRotator(
		FMath::RandRange(0.f, static_cast<float>(RandomRotation.Pitch)),
		FMath::RandRange(0.f, static_cast<float>(RandomRotation.Yaw)),
		FMath::RandRange(0.f, static_cast<float>(RandomRotation.Roll))
	);

	InSelectedActor->AddActorLocalRotation(CurrentRandRotation);
	//InSelectedActor->SetActorRelativeRotation(CurrentRandRotation); // This resets to 0 if values are 0
}

void UQuickActorActionsWidget::RandomizeSelectedActorsRotationX()
{
	if (!RandomRotationYaw.bEnabled && !RandomRotationPitch.bEnabled && !RandomRotationRoll.bEnabled || RandomRotationYaw.Min == 0 && RandomRotationYaw.Max == 0 && RandomRotationPitch.Min == 0 && RandomRotationPitch.Max == 0 && RandomRotationRoll.Min == 0 && RandomRotationRoll.Max == 0)
	{
		// No valid inputs to execute action, exit early
		return;
	}
	
	if (!GetEditorActorSubsystem())
	{
		return;
	}

	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();

	for (AActor* SelectedActor : SelectedActors)
	{
		const float RandRotYaw = RandomRotationYaw.bEnabled == true ? FMath::RandRange(RandomRotationYaw.Min, RandomRotationYaw.Max) : 0.f;
		const float RandRotPitch = RandomRotationPitch.bEnabled == true ? FMath::RandRange(RandomRotationPitch.Min, RandomRotationPitch.Max) : 0.f;
		const float RandRotRoll = RandomRotationRoll.bEnabled == true ? FMath::RandRange(RandomRotationRoll.Min, RandomRotationRoll.Max) : 0.f;
		SelectedActor->AddActorLocalRotation(FRotator(RandRotPitch, RandRotYaw, RandRotRoll));
	}
}

void UQuickActorActionsWidget::RandomizeActorOffset(AActor* InSelectedActor)
{
	FVector CurrentRandomOffset = FVector(
		FMath::RandRange(0.f, static_cast<float>(OffsetDistance.X)),
		FMath::RandRange(0.f, static_cast<float>(OffsetDistance.Y)),
		FMath::RandRange(0.f, static_cast<float>(OffsetDistance.Z))
	);
	InSelectedActor->AddActorLocalOffset(CurrentRandomOffset);
}

void UQuickActorActionsWidget::RandomizeActorScale(AActor* InSelectedActor)
{
	FVector CurrentRandomScale = FVector(
		FMath::RandRange(static_cast<float>(RandomScaleMin.X), static_cast<float>(RandomScaleMax.X)),
		FMath::RandRange(static_cast<float>(RandomScaleMin.Y), static_cast<float>(RandomScaleMax.Y)),
		FMath::RandRange(static_cast<float>(RandomScaleMin.Z), static_cast<float>(RandomScaleMax.Z))
	);
	//InSelectedActor->SetActorScale3D(CurrentRandomScale);
	InSelectedActor->SetActorRelativeScale3D(CurrentRandomScale); // This has same effect as non-relative?
}

void UQuickActorActionsWidget::RandomizeSelectedActorsTransform()
{
	if (!GetEditorActorSubsystem())
	{
		return;
	}

	if (OffsetDistance.IsZero() && RandomRotation.IsZero() && RandomScaleMin == RandomScaleMax)
	{
		DebugHeader::ShowNotification(
			LOCTEXT("NoValidInputs", "Offset distance, random rotation, and random scale are all unchanged!"),
			ELogVerbosity::Warning
		);
		return;
	}

	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();

	for (AActor* SelectedActor : SelectedActors)
	{
		// AddActorLocalTransform is not working well, as it is additive. Thus, each transform type is split
		RandomizeActorRotation(SelectedActor);
		RandomizeActorOffset(SelectedActor);
		RandomizeActorScale(SelectedActor);
	}
}

#undef LOCTEXT_NAMESPACE