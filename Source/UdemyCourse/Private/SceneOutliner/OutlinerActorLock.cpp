// Copyright MODogma. All Rights Reserved.

#include "SceneOutliner/OutlinerActorLock.h"
#include "UdemyCourseStyle.h"
#include "UdemyCourse.h"
#include "ActorTreeItem.h"

#define LOCTEXT_NAMESPACE "OutlinerActorLock"

SHeaderRow::FColumn::FArguments FOutlinerActorLock::ConstructHeaderRowColumn()
{
	return SHeaderRow::Column(GetColumnID())
		.FixedWidth(24.f) // 24.f matches width of visibility column
		.HAlignHeader(HAlign_Center)
		.VAlignHeader(VAlign_Center)
		.HAlignCell(HAlign_Center)
		.VAlignCell(VAlign_Center)
		.DefaultTooltip(LOCTEXT("LockActorTooltip", "Lock the selected actor from re-selection and movement."))
		[
			SNew(SImage)
			.ColorAndOpacity(FSlateColor::UseForeground())
			.Image(FUdemyCourseStyle::GetCreatedStyleSet()->GetBrush(FName("LevelEditor.LockActor")))
		];
}

const TSharedRef<SWidget> FOutlinerActorLock::ConstructRowWidget(FSceneOutlinerTreeItemRef InTreeItem, const STableRow<FSceneOutlinerTreeItemPtr>& InRow)
{
	// Use CastTo() to cast to a different class
	FActorTreeItem* ActorTreeItem = InTreeItem->CastTo<FActorTreeItem>();

	if (!ActorTreeItem || !ActorTreeItem->IsValid())
	{
		return SNullWidget::NullWidget;
	}

	FUdemyCourseModule& UdemyCourseModule = FModuleManager::LoadModuleChecked<FUdemyCourseModule>(TEXT("UdemyCourse"));
	bool bIsActorLocked = UdemyCourseModule.IsActorLocked(ActorTreeItem->Actor.Get());

	TSharedRef<SWidget> ConstructedRowCheckBox = SNew(SCheckBox)
		.HAlign(HAlign_Center)
		// InRow isn't properly linked from this virtual function, as InRow.IsParentValid() returns false...
		//.Visibility(InRow.IsHovered() || bIsActorLocked ? EVisibility::Visible : EVisibility::Hidden)
		.IsChecked(bIsActorLocked ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
		.OnCheckStateChanged(this, &FOutlinerActorLock::OnRowCheckStateChanged, ActorTreeItem->Actor)
		.CheckedHoveredImage(FUdemyCourseStyle::GetCreatedStyleSet()->GetBrush(TEXT("LevelEditor.LockActor")))
		.BackgroundHoveredImage(FUdemyCourseStyle::GetCreatedStyleSet()->GetBrush(TEXT("LevelEditor.LockActor")))
		// Anywhere.Blank is working, but doesn't allow hovering since not an image (even when non-zero alpha)
		.BackgroundImage(FUdemyCourseStyle::GetCreatedStyleSet()->GetBrush(TEXT("SceneOutliner.BlankImage")))
		.BackgroundPressedImage(FUdemyCourseStyle::GetCreatedStyleSet()->GetBrush(TEXT("LevelEditor.LockActor")))
		.CheckedPressedImage(FUdemyCourseStyle::GetCreatedStyleSet()->GetBrush(TEXT("LevelEditor.LockActor")))
		.UncheckedPressedImage(FUdemyCourseStyle::GetCreatedStyleSet()->GetBrush(TEXT("LevelEditor.LockActor")))
		// This adds an icon to the background rounded-corner square default checkbox icon
		//.UncheckedImage(FUdemyCourseStyle::GetCreatedStyleSet()->GetBrush(TEXT("SceneOutliner.BlankLock")))
		.CheckedImage(FUdemyCourseStyle::GetCreatedStyleSet()->GetBrush(TEXT("LevelEditor.LockActor")))
		.ToolTipText(LOCTEXT("LockActorTooltip", "Lock the selected actor from re-selection and movement."));

	//// Toggle button for practice. The style not as good
	//TSharedRef<SWidget> ConstructedRowCheckBox = SNew(SCheckBox)
	//	.HAlign(HAlign_Center)
	//	.Visibility(EVisibility::Visible)
	//	.Type(ESlateCheckBoxType::ToggleButton)
	//	.Style(FUdemyCourseStyle::GetCreatedStyleSet(), "SceneOutliner.ActorLock")
	//	.IsChecked(bIsActorLocked ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
	//	.OnCheckStateChanged(this, &FOutlinerActorLock::OnRowCheckStateChanged, ActorTreeItem->Actor);
	
	return ConstructedRowCheckBox;
}

void FOutlinerActorLock::OnRowCheckStateChanged(ECheckBoxState InNewState, TWeakObjectPtr<AActor> InLinkedActor)
{
	FUdemyCourseModule& UdemyCourseModule = FModuleManager::LoadModuleChecked<FUdemyCourseModule>(TEXT("UdemyCourse"));
	TArray<AActor*> SelectedActors = UdemyCourseModule.GetSelectedActors();

	switch (InNewState)
	{
	case ECheckBoxState::Unchecked:
		// Are multiple row actors are selected and the checked box (InLinkedActor) is the same?
		if (SelectedActors.Contains(InLinkedActor))
		{
			UdemyCourseModule.OnUnlockSelectedActorButtonClicked();
		}
		else
		{
			// Unlock the single row actor only
			UdemyCourseModule.UnlockSelectedActor(InLinkedActor.Get());
		}
		break;
	case ECheckBoxState::Checked:
		// Are multiple row actors are selected and the checked box (InLinkedActor) is the same
		if (SelectedActors.Contains(InLinkedActor))
		{
			UdemyCourseModule.OnLockSelectedActorButtonClicked();
		}
		else
		{
			// Lock the single row actor only
			UdemyCourseModule.LockSelectedActor(InLinkedActor.Get());
		}
		break;
	case ECheckBoxState::Undetermined:
		break;
	default:
		break;
	}
}

#undef LOCTEXT_NAMESPACE