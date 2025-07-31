// Copyright MODogma. All Rights Reserved.

#include "UdemyCourseStyle.h"
#include "Interfaces/IPluginManager.h" // For icons
#include "Styling/StyleColors.h" // For FSlateColors

// Qualifying class members to define them, or else will get linker error from unknown scope
FName FUdemyCourseStyle::StyleSetName = TEXT("UdemyCourseStyle");
TSharedPtr<FSlateStyleSet> FUdemyCourseStyle::CreatedStyleSet = nullptr;

void FUdemyCourseStyle::InitializeIcons()
{
	// If slate style not valid yet, create it
	if (!CreatedStyleSet.IsValid())
	{
		CreatedStyleSet = CreateStyleSet();
		FSlateStyleRegistry::RegisterSlateStyle(*CreatedStyleSet);
	}
}

TSharedRef<FSlateStyleSet> FUdemyCourseStyle::CreateStyleSet()
{
	TSharedRef<FSlateStyleSet> CustomSlateStyle = MakeShareable(new FSlateStyleSet(StyleSetName));
	// Ensure relative plugin resources path
	const FString ResourcesDir = IPluginManager::Get().FindPlugin(TEXT("UdemyCourse"))->GetBaseDir() / "Resources";

	CustomSlateStyle->SetContentRoot(ResourcesDir);

	const FVector2D Icon16x16 = FVector2D(16.f, 16.f);
	// Set custom internal name used for fetching the icon, pointing to relative object path
	CustomSlateStyle->Set("ContentBrowser.DeleteUnusedAssets", new FSlateImageBrush(ResourcesDir / "DeleteUnusedAssets_128.png", Icon16x16, FSlateColor::UseSubduedForeground()));
	CustomSlateStyle->Set("ContentBrowser.DeleteEmptyFolders", new FSlateImageBrush(ResourcesDir / "DeleteEmptyFolders_128.png", Icon16x16, FSlateColor::UseSubduedForeground()));
	CustomSlateStyle->Set("ContentBrowser.AdvancedDeletion", new FSlateImageBrush(ResourcesDir / "AdvancedDeletion_128.png", Icon16x16, FSlateColor::UseSubduedForeground()));
	CustomSlateStyle->Set("LevelEditor.LockActor", new FSlateImageBrush(ResourcesDir / "LockActor_128.png", Icon16x16, FSlateColor::UseSubduedForeground()));
	CustomSlateStyle->Set("LevelEditor.UnlockActor", new FSlateImageBrush(ResourcesDir / "UnlockActor_128.png", Icon16x16, FSlateColor::UseSubduedForeground()));
	CustomSlateStyle->Set("SceneOutliner.BlankImage", new FSlateImageBrush(ResourcesDir / "LockActor_128.png", Icon16x16, FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.f))));
	CustomSlateStyle->Set("Anywhere.BlankColor", new FSlateColorBrush(FLinearColor(0.f, 0.f, 0.f, 0.f)));

	// Create a custom checkbox style for toggle buttons
	const FCheckBoxStyle ActorLockToggleButtonStyle = FCheckBoxStyle()
		.SetCheckBoxType(ESlateCheckBoxType::ToggleButton)
		// Vertical and horizontal padding is required, or toggle buttons are invisible
		.SetPadding(FMargin(10.f, 10.f))
		// FSlateColor(FLinearColor(1.f, 1.f, 1.f, .25f) is the same as FStyleColors::White25
		.SetUncheckedImage(FSlateImageBrush(ResourcesDir / "LockActor_128.png", Icon16x16, FStyleColors::White25))
		.SetUncheckedHoveredImage(FSlateImageBrush(ResourcesDir / "LockActor_128.png", Icon16x16, FStyleColors::AccentBlue))
		.SetUncheckedPressedImage(FSlateImageBrush(ResourcesDir / "LockActor_128.png", Icon16x16, FStyleColors::Foreground))

		.SetCheckedImage(FSlateImageBrush(ResourcesDir / "LockActor_128.png", Icon16x16, FStyleColors::Foreground))
		.SetCheckedHoveredImage(FSlateImageBrush(ResourcesDir / "LockActor_128.png", Icon16x16, FStyleColors::AccentBlack))
		.SetCheckedPressedImage(FSlateImageBrush(ResourcesDir / "LockActor_128.png", Icon16x16, FStyleColors::AccentGray));

	// Add the style to the style set
	CustomSlateStyle->Set("SceneOutliner.ActorLock", ActorLockToggleButtonStyle);

	return CustomSlateStyle;
}

void FUdemyCourseStyle::Shutdown()
{
	if (CreatedStyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*CreatedStyleSet);
		// Remove pointer to release memory
		CreatedStyleSet.Reset();
	}
}