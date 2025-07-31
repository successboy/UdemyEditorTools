// Copyright MODogma. All Rights Reserved.

#include "UdemyCourse.h"
#include "DebugHeader.h"
#include "ContentBrowserModule.h"
#include "EditorAssetLibrary.h"
#include "AssetToolsModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "SlateWidgets/AdvancedDeletionWidget.h"
#include "UdemyCourseStyle.h"
#include "LevelEditor.h"
#include "Engine/Selection.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "Styling/AppStyle.h"
#include "UICommands/UdemyCourseUICommands.h"
#include "SceneOutliner/OutlinerActorLock.h"

#define LOCTEXT_NAMESPACE "FUdemyCourseModule"

void FUdemyCourseModule::StartupModule()
{
	FUdemyCourseStyle::InitializeIcons();
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	InitCBMenuExtension();
	RegisterAdvancedDeletionTab();

	//Must be called before InitCustomUICommands() to prevent crash
	FUdemyCourseUICommands::Register();
	// Must be called before InitLevelEditorExtension(), due to accessing its shared ref
	InitCustomUICommands();
	InitLevelEditorExtension();

	InitCustomSelectionEvent();
	InitSceneOutlinerExtension();
}

#pragma region SelectionLock
void FUdemyCourseModule::InitCustomSelectionEvent()
{
	USelection* Selection = GEditor->GetSelectedActors();

	// Bind new function to native selection delegate
	Selection->SelectObjectEvent.AddRaw(this, &FUdemyCourseModule::OnActorSelected);
}

void FUdemyCourseModule::OnActorSelected(UObject* InSelectedObject)
{
	if (!GetEditorActorSubsystem())
	{
		return;
	}

	// Cast UObject to AActor to get the proper actor label name. The object
	// may be better in UE 5.3+, though, the ID name is always unique
	if (AActor* CurrentSelectedActor = Cast<AActor>(InSelectedObject))
	{
		if (IsActorLocked(CurrentSelectedActor))
		{
			// These native class member functions won't work
			//CurrentSelectedActor->IsSelected();
			//CurrentSelectedActor->IsSelectable();
			WeakEditorActorSubsystem->SetActorSelectionState(CurrentSelectedActor, false);
		}
	}
}

void FUdemyCourseModule::LockSelectedActor(AActor* ActorToProcess)
{
	if (!ActorToProcess)
	{
		return;
	}

	ActorToProcess->Tags.AddUnique(TEXT("LockActor"));
	// Lock location as well, or else non-level selection can still transform the actor
	// "Transform -> Lock actor movement" in the context menu
	ActorToProcess->SetLockLocation(true);
	RefreshSceneOutliner();
}

void FUdemyCourseModule::UnlockSelectedActor(AActor* ActorToProcess)
{
	if (!ActorToProcess)
	{
		return;
	}

	ActorToProcess->Tags.Remove(TEXT("LockActor"));
	ActorToProcess->SetLockLocation(false);
	RefreshSceneOutliner();
}

void FUdemyCourseModule::UnlockAllActors()
{
	TArray<AActor*> AllActors = WeakEditorActorSubsystem->GetAllLevelActors();

	for (AActor* Actor : AllActors)
	{
		Actor->Tags.Remove(TEXT("LockActor"));
	}
}

bool FUdemyCourseModule::IsActorLocked(AActor* ActorToProcess)
{
	if (!ActorToProcess)
	{
		return false;
	}
	
	return ActorToProcess->Tags.Contains("LockActor");
}

void FUdemyCourseModule::RefreshSceneOutliner()
{
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	TSharedPtr<ISceneOutliner> SceneOutliner = LevelEditorModule.GetFirstLevelEditor()->GetMostRecentlyUsedSceneOutliner()/*GetSceneOutliner() API deprecation per the warning*/;

	if (!SceneOutliner.IsValid())
	{
		return;
	}

	// Refresh() doesn't update the state of the outliner properly, so using FullRefresh()
	SceneOutliner->FullRefresh();
}

#pragma endregion // SelectionLock

#pragma region CustomUICommands
void FUdemyCourseModule::InitCustomUICommands()
{
	CustomUICommands = MakeShareable(new FUICommandList());
	CustomUICommands->MapAction(FUdemyCourseUICommands::Get().LockActorSelection, FExecuteAction::CreateRaw(this, &FUdemyCourseModule::OnLockSelectedActorShortcutPressed));
	CustomUICommands->MapAction(FUdemyCourseUICommands::Get().UnlockAllActors, FExecuteAction::CreateRaw(this, &FUdemyCourseModule::OnUnlockAllActorsShortcutPressed));
}

void FUdemyCourseModule::OnLockSelectedActorShortcutPressed()
{
	OnLockSelectedActorButtonClicked();
}

void FUdemyCourseModule::OnUnlockAllActorsShortcutPressed()
{
	OnUnlockAllActorsButtonClicked();
}

#pragma endregion // CustomUICommands

#pragma region SceneOutlinerExtension
void FUdemyCourseModule::InitSceneOutlinerExtension()
{
	FSceneOutlinerModule& SceneOutlinerModule = FModuleManager::LoadModuleChecked<FSceneOutlinerModule>(FName("SceneOutliner"));
	// InPriorityIndex = 1: This column will be located after the visibility (eye icon) column, which is index 0
	FSceneOutlinerColumnInfo ActorLockColumnInfo(ESceneOutlinerColumnVisibility::Visible, 1, FCreateSceneOutlinerColumn::CreateRaw(this, &FUdemyCourseModule::OnCreateActorLockColumn));
	SceneOutlinerModule.RegisterDefaultColumnType<FOutlinerActorLock>(ActorLockColumnInfo);
}

TSharedRef<ISceneOutlinerColumn> FUdemyCourseModule::OnCreateActorLockColumn(ISceneOutliner& SceneOutliner)
{
	// Empty constructor is declared in OutlinerActorLock.h
	return MakeShareable(new FOutlinerActorLock(SceneOutliner));
}

bool FUdemyCourseModule::IsLevelActorSelected()
{
	TArray<AActor*> SelectedLevelActors = WeakEditorActorSubsystem->GetSelectedLevelActors();

	return SelectedLevelActors.IsEmpty() ? false : true;
}

void FUdemyCourseModule::UnregisterSceneOutlinerColumnExtension()
{
	FSceneOutlinerModule& SceneOutlinerModule = FModuleManager::LoadModuleChecked<FSceneOutlinerModule>(TEXT("SceneOutliner"));

	SceneOutlinerModule.UnRegisterColumnType<FOutlinerActorLock>();
}

#pragma endregion // SceneOutlinerExtension

bool FUdemyCourseModule::GetEditorActorSubsystem()
{
	if (!WeakEditorActorSubsystem.IsValid())
	{
		WeakEditorActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
	}

	return WeakEditorActorSubsystem.IsValid();
}

#pragma region LevelEditorExtensionMenu
void FUdemyCourseModule::InitLevelEditorExtension()
{
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	// Gets the full list of bound editor actions (will use for UI commands)
	TSharedRef<FUICommandList> GlobalLevelEditorCommands = LevelEditorModule.GetGlobalLevelEditorActions();

	if (CustomUICommands.IsValid())
	{
		// Add our custom UI commands to the existing list
		GlobalLevelEditorCommands->Append(CustomUICommands.ToSharedRef());
	}
	else
	{
		UE_LOG(LogUdemyCourse, Error, TEXT("Custom UI commands not appended due to incorrect initialization order."));
	}

	TArray<FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors>& LevelViewportMenuExtenders = LevelEditorModule.GetAllLevelViewportContextMenuExtenders();
	LevelViewportMenuExtenders.Add(FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors::CreateRaw(this, &FUdemyCourseModule::CustomLevelEditorMenuExtender));
}

TSharedRef<FExtender> FUdemyCourseModule::CustomLevelEditorMenuExtender(const TSharedRef<FUICommandList> InUICommandList, const TArray<AActor*> InSelectedActors)
{
	// This can also be done by TSharedRef<FExtender> MenuExtender(new FExtender()), but it is unsafe as could cause leaks
	TSharedRef<FExtender> MenuExtender = MakeShareable(new FExtender());

	// Spawn the context menu when an actor is selected
	if (InSelectedActors.Num() > 0)
	{
		// Hook into the existing menu at specified hook name
		MenuExtender->AddMenuExtension(
			FName(TEXT("Delete")),
			EExtensionHook::After,
			InUICommandList,
			FMenuExtensionDelegate::CreateRaw(this, &FUdemyCourseModule::AddLevelEditorMenuEntry)
		);
	}

	return MenuExtender;
}

void FUdemyCourseModule::AddLevelEditorMenuEntry(class FMenuBuilder& InMenuBuilder)
{
	InMenuBuilder.AddMenuEntry(
		LOCTEXT("LockActor", "Lock Actor"), // Title
		LOCTEXT("LockActorTooltip", "Lock the selected actor(s) from being selected or moved."), // Tooltip
		FSlateIcon(FUdemyCourseStyle::GetStyleSetName(), "LevelEditor.LockActor"), // Icon
		FExecuteAction::CreateRaw(this, &FUdemyCourseModule::OnLockSelectedActorButtonClicked) // Function binding
	);

	InMenuBuilder.AddMenuEntry(
		LOCTEXT("UnlockAl", "Unlock All"), // Title
		LOCTEXT("UnlockAllTooltip", "Unlocks all currently locked actors."), // Tooltip
		FSlateIcon(FUdemyCourseStyle::GetStyleSetName(), "LevelEditor.UnlockActor"), // Icon
		FExecuteAction::CreateRaw(this, &FUdemyCourseModule::OnUnlockAllActorsButtonClicked) // Function binding
	);
}

void FUdemyCourseModule::OnLockSelectedActorButtonClicked()
{
	if (!GetEditorActorSubsystem())
	{
		return;
	}

	TArray<AActor*> SelectedActors = WeakEditorActorSubsystem->GetSelectedLevelActors();

	if (SelectedActors.IsEmpty())
	{
		DebugHeader::ShowNotification(
			LOCTEXT("NoSelectionWarning", "No actors currently selected!"),
			ELogVerbosity::Warning
		);
		return;
	}

	FString LockedActorLabels = TEXT("Selected actors locked: ");

	for (AActor* SelectedActor : SelectedActors)
	{
		if (!SelectedActor)
		{
			continue;
		}

		// TODO: Add visual indicator that the actor is locked (in level editor or world outliner)
		LockSelectedActor(SelectedActor);
		LockedActorLabels.Append(TEXT("\n") + SelectedActor->GetActorLabel());
		//WeakEditorActorSubsystem->SetActorSelectionState(SelectedActor, false); // Opation A (inefficient)
	}

	// This is more-efficient than iterating over IsLevelActorSelected()
	WeakEditorActorSubsystem->SelectNothing(); // Option B
	DebugHeader::ShowNotification(
		FText::Format(LOCTEXT("LockedActorStats", "{0}"), FText::FromString(LockedActorLabels))
	);
}

void FUdemyCourseModule::OnUnlockSelectedActorButtonClicked()
{
	if (!GetEditorActorSubsystem())
	{
		return;
	}

	TArray<AActor*> SelectedActors = WeakEditorActorSubsystem->GetSelectedLevelActors();
	FString UnlockedActorLabels = TEXT("Unlocked actors: ");
	TArray<AActor*> UnlockedActors;

	for (AActor* SelectedActor : SelectedActors)
	{
		if (!SelectedActor)
		{
			continue;
		}

		if (SelectedActor->Tags.Contains(TEXT("LockActor")))
		{
			UnlockSelectedActor(SelectedActor);
			UnlockedActorLabels.Append(TEXT("\n") + SelectedActor->GetActorLabel());
			UnlockedActors.Add(SelectedActor);
		}

	}

	if (UnlockedActors.IsEmpty())
	{
		return;
	}

	//WeakEditorActorSubsystem->SelectNothing(); // Maintaining selection is desired for unlocking
	DebugHeader::ShowNotification(FText::Format(LOCTEXT("UnlockedActorStats", "{0}"), FText::FromString(UnlockedActorLabels)));
}

void FUdemyCourseModule::OnUnlockAllActorsButtonClicked()
{
	TArray<AActor*> AllActors = WeakEditorActorSubsystem->GetAllLevelActors();
	FString UnlockedActorLabels = TEXT("Unlocked actors: ");

	if (AllActors.IsEmpty())
	{
		DebugHeader::ShowNotification(
			LOCTEXT("NoLockedActorsWarning", "There are no locked actors to unlock!"),
			ELogVerbosity::Warning
		);
		return;
	}

	for (AActor* Actor : AllActors)
	{
		if (Actor->Tags.Contains(TEXT("LockActor")))
		{
			Actor->Tags.Remove(TEXT("LockActor"));
			Actor->SetLockLocation(false);
			UnlockedActorLabels.Append(TEXT("\n" + Actor->GetActorLabel()));
		}
	}

	RefreshSceneOutliner();
	//WeakEditorActorSubsystem->SelectNothing(); // Maintaining selection is desired for unlocking
	DebugHeader::ShowNotification(
		FText::Format(LOCTEXT("UnlockedActorStats", "{0}"), FText::FromString(UnlockedActorLabels))
	);
}

#pragma endregion // LevelEditorExtensionMenu

#pragma region ContentBrowserMenuExtension

void FUdemyCourseModule::InitCBMenuExtension()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TArray<FContentBrowserMenuExtender_SelectedPaths>& CBExtenderSelectedPaths = ContentBrowserModule.GetAllPathViewContextMenuExtenders();

	// First binding: Bind the new CB menu hook. This is adding a custom delegate to all existing delegates
	CBExtenderSelectedPaths.Add(FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FUdemyCourseModule::CustomCBMenuExtender));
}

/** First: Binds the location of the new CB menu entry to hook into the existing CB menu */
TSharedRef<FExtender> FUdemyCourseModule::CustomCBMenuExtender(const TArray<FString>& InSelectedPaths)
{
	// The new keyword creates a new instance. Only use with smart pointer type (TSharefRef) to prevent memory leaking
	TSharedRef<FExtender> MenuExtender = MakeShareable(new FExtender());

	if (InSelectedPaths.Num() > 0)
	{
		// Get a list of hook names by enabling bDisplayUIExtensionPoints=True in editor settings
		MenuExtender->AddMenuExtension
		(
			FName(TEXT("Delete")), // The name of the existing hook
			EExtensionHook::After, // Relative position to the hook
			TSharedPtr<FUICommandList>(), // Add custom keyboard shortcuts here
			FMenuExtensionDelegate::CreateRaw(this, &FUdemyCourseModule::AddCBMenuEntry) // Second binding: Bind the entry details
		);

		// Store the array of selected to be used in another function
		SelectedPaths = InSelectedPaths;
	}

	return MenuExtender;
}

/** Second binding: Binds the details of the menu entry */
void FUdemyCourseModule::AddCBMenuEntry(class FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
		LOCTEXT("DeleteUnusedAssets", "Delete Unused Assets"), // Title
		LOCTEXT("DeleteAssetsTooltip", "Deletes assets which have no references within the selected folder.\nScans for empty folders after completion."), // Tooltip
		FSlateIcon(FUdemyCourseStyle::GetStyleSetName(), "ContentBrowser.DeleteUnusedAssets"), // Custom icon
		FUIAction(
			FExecuteAction::CreateRaw(this, &FUdemyCourseModule::OnDeleteUnusedAssetsButtonClicked),
			FCanExecuteAction::CreateRaw(this, &FUdemyCourseModule::CanExecuteDeleteUnusedAssets)
		)
	);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("DeleteEmptyFolders", "Delete Empty Folders"), // Title
		LOCTEXT("DeleteFoldersTooltip", "Deletes selected folder with no contents, including subfolders."), // Tooltip
		FSlateIcon(FUdemyCourseStyle::GetStyleSetName(), "ContentBrowser.DeleteEmptyFolders"), // Custom icon
		FUIAction(
			FExecuteAction::CreateRaw(this, &FUdemyCourseModule::OnDeleteEmptyFoldersButtonClicked),
			FCanExecuteAction::CreateRaw(this, &FUdemyCourseModule::CanExecuteDeleteEmptyFolders)
		)
	);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("AdvancedDeletion", "Advanced Deletion"), // Title
		LOCTEXT("AdvancedDeletionTooltip", "Opens advanced deletion window for deleting assets."), // Tooltip
		FSlateIcon(FUdemyCourseStyle::GetStyleSetName(), "ContentBrowser.AdvancedDeletion"), // Custom icon
		FUIAction(FExecuteAction::CreateRaw(this, &FUdemyCourseModule::OnAdvancedDeletionButtonClicked)) // Third: Binding to execute click function
	);
}

/** Third: The function to execute when menu entry is clicked */
// TODO: See FAssetFolderContextMenu::ExecuteFixUpRedirectorsInFolder, as it may be easier...
// ...to call instead of redefining. Note: it already properly handles recursive paths.
void FUdemyCourseModule::OnDeleteUnusedAssetsButtonClicked()
{
	//// Safety check in case want to allow user input for the entries,
	//// but show them a warning message to close the AdvancedDeletion tab first.
	//// The button is already disabled via the menu entry edit condition in FUIAction()
	//if (ConstructedDockTab.IsValid())
	//{
	//	// Show notification here
	//	return;
	//}

	// Multiple folders are selected
	if (SelectedPaths.Num() > 1)
	{
		DebugHeader::ShowNotification(LOCTEXT("FolderSelectionErrorMessage", "Only one selected folder is allowed at a time."), ELogVerbosity::Error);
		return;
	}

	TArray<FString> AssetPathNames = UEditorAssetLibrary::ListAssets(SelectedPaths[0]);
	TArray<FString> UnreferencedAssets;

	// Fixup redirectors for cleaner data before checking for unused assets
	if (!FixupRedirectors())
	{
		return;
	}

	for (const FString& AssetPath : AssetPathNames)
	{
		// Prevent errors if asset isn't properly found
		if (!UEditorAssetLibrary::DoesAssetExist(AssetPath))
		{
			continue;
		}

		TArray<FString> PackageReferencers = UEditorAssetLibrary::FindPackageReferencersForAsset(AssetPath);

		// Add asset without references to the array of unreferenced
		if (PackageReferencers.IsEmpty())
		{
			UnreferencedAssets.Add(AssetPath);
		}
		else
		{
			UE_LOG(LogUdemyCourse, Warning, TEXT("Asset %s has reference(s). Skipping deletion: "), *AssetPath);

			for (const FString& PackageReference : PackageReferencers)
			{
				UE_LOG(LogUdemyCourse, Warning, TEXT("%s"), *PackageReference);
			}
		}
	}

	if (UnreferencedAssets.IsEmpty())
	{
		DebugHeader::ShowNotification(
			FText::Format(
				LOCTEXT("AssetsMissingInFolder", "There are no unreferenced assets in {0}"),
				FText::FromString(SelectedPaths[0])
			),
			ELogVerbosity::Warning
		);

		return;
	}

	FString JoinedUnreferencedAssets = FString::Join(UnreferencedAssets, TEXT("\n"));

	EAppReturnType::Type ConfirmDeletion = FMessageDialog::Open(
		EAppMsgType::YesNo,
		FText::Format(
			LOCTEXT("UserInputRequested", "There are {0} unreferenced asset(s) under {1}:\n{2}\nWould you like to delete them?"),
			UnreferencedAssets.Num(),
			FText::FromString(SelectedPaths[0]),
			FText::FromString(JoinedUnreferencedAssets)
		)
	);
	// Could also use ConfirmSelected == EAppReturnType::Ok, but it appears that true captures all positive types
	if (ConfirmDeletion) 
	{
		for (const FString& UnreferencedAsset : UnreferencedAssets)
		{
			UEditorAssetLibrary::DeleteAsset(UnreferencedAsset);
			UE_LOG(LogUdemyCourse, Log, TEXT("Deleted asset: %s"), *UnreferencedAsset);
		}

		DebugHeader::ShowNotification(
			FText::Format(
				LOCTEXT("AssetsMissingInFolder", "Successfully deleted {0} unreferenced asset(s)!"),
				UnreferencedAssets.Num()
			)
		);

		// Optionally, provide user choice to delete empty folders if they are discovered after asset deletion
		OnDeleteEmptyFoldersButtonClicked();
	}
}

/** Returns true if redirectors dialog was chosen to fixup (or no fixing required), otherwise false. */
bool FUdemyCourseModule::FixupRedirectors()
{
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();

	TArray<FString> PathsToScan;
	PathsToScan.Add(SelectedPaths[0]);
	// Force sync scan before querying asset registry, for accuracy. InFilePaths is set to empty
	UE::AssetRegistry::EScanFlags ScanFlags = UE::AssetRegistry::EScanFlags::ForceRescan;
	AssetRegistry.ScanSynchronous(PathsToScan, TArray<FString>(), ScanFlags);

	TArray<UObjectRedirector*> RedirectorsToFix;
	TArray<FAssetData> OutRedirectorAssetData;
	FARFilter RedirectorFilter;
	RedirectorFilter.ClassPaths.Add(UObjectRedirector::StaticClass()->GetClassPathName());
	// Simpler than looping through subpaths after AssetRegistry.GetSubPaths()
	RedirectorFilter.bRecursivePaths = true;
	RedirectorFilter.PackagePaths.Add(FName(SelectedPaths[0]));
	// Populate the redirector data, for the selected folder, with all recursive paths included
	AssetRegistry.GetAssets(RedirectorFilter, OutRedirectorAssetData);

	// Fixup redirectors before scanning for unreferenced assets for safer and cleaner result
	if (!OutRedirectorAssetData.IsEmpty())
	{
		EAppReturnType::Type ConfirmFixup = FMessageDialog::Open(
			EAppMsgType::YesNo,
			FText::Format(
				LOCTEXT("UserInputRequested", "There are {0} redirector(s) to fixup before proceeding.\nWould you like to fix them?"),
				OutRedirectorAssetData.Num()
			)
		);

		if (!ConfirmFixup)
		{
			return false;
		}

		for (const FAssetData& RedirectorAssetDatum : OutRedirectorAssetData)
		{
			// If data is valid, add the redirector to the array to fix
			if (UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(RedirectorAssetDatum.GetAsset()))
			{
				RedirectorsToFix.Add(RedirectorToFix);
			}
		}

		AssetToolsModule.Get().FixupReferencers(RedirectorsToFix);
		return true;
	}

	// Redirectors don't need fixing, so return "good" status
	return true;
}

/** Delete the recursive directories of the currently-selected folder */
void FUdemyCourseModule::OnDeleteEmptyFoldersButtonClicked()
{
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
	//TArray<FName, FDefaultAllocator> OutRecursivePaths;
	TArray<FString> OutRecursivePaths;
	TArray<FString> FoldersToDelete;

	// Only add subfolders, as scanning selected folder produces false positive (even when containing assets)
	AssetRegistry.GetSubPaths(SelectedPaths[0], OutRecursivePaths, true);

	for (const FString& Path : OutRecursivePaths)
	{
		// If path is empty, add to array to delete. Non-recursive as we only want contents of current path
		if (UEditorAssetLibrary::DoesDirectoryHaveAssets(Path, false))
		{
			continue;
		}

		FoldersToDelete.Add(Path);
	}

	if (FoldersToDelete.IsEmpty())
	{
		DebugHeader::ShowNotification(
			FText::Format(
				LOCTEXT("NoEmptyFolders", "There are no empty folders under {0}, skipping deletion."),
				FText::FromString(SelectedPaths[0])
			),
			ELogVerbosity::Warning
		);
		return;
	}

	// Join the array into a single string for logging
	FString JoinedFolders = FString::Join(FoldersToDelete, TEXT("\n"));

	EAppReturnType::Type ConfirmDeletion = FMessageDialog::Open(
		EAppMsgType::YesNo,
		FText::Format(
			LOCTEXT("UserInputRequested", "There are {0} empty folders under {1}:\n{2}\nWould you like to delete them?"),
			FoldersToDelete.Num(),
			FText::FromString(SelectedPaths[0]),
			FText::FromString(JoinedFolders)
		)
	);

	if (ConfirmDeletion)
	{
		for (const FString& DeleteFolder : FoldersToDelete)
		{
			UEditorAssetLibrary::DeleteDirectory(DeleteFolder);
		}

		DebugHeader::ShowNotification(
			FText::Format(
				LOCTEXT("SuccessfulDeletion", "Succesfully deleted {0} empty folder(s) under {1}"),
				FoldersToDelete.Num(),
				FText::FromString(SelectedPaths[0])
			)
		);
	}
}

void FUdemyCourseModule::OnAdvancedDeletionButtonClicked()
{
	// Call here so that list filters from combo box will fixup when selection changed
	FixupRedirectors();
	// Tab must match spelling in FUdemyCourseModule::RegisterAdvancedDeletionTab().
	FGlobalTabmanager::Get()->TryInvokeTab(FName("AdvancedDeletion"));
}

bool FUdemyCourseModule::CanExecuteDeleteUnusedAssets()
{
	// Disable when the Advanced Deletion tab is open
	return !ConstructedDockTab.IsValid();
}

bool FUdemyCourseModule::CanExecuteDeleteEmptyFolders()
{
	// Disable when the Advanced Deletion tab is open
	return !ConstructedDockTab.IsValid();
}

#pragma endregion // ContentBrowserMenuExtension

#pragma region CustomEditorTab

void FUdemyCourseModule::RegisterAdvancedDeletionTab()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("AdvancedDeletion"), FOnSpawnTab::CreateRaw(this, &FUdemyCourseModule::OnSpawnAdvancedDeletionTab))
		.SetIcon(FSlateIcon(FUdemyCourseStyle::GetStyleSetName(), "ContentBrowser.AdvancedDeletion"))
		.SetDisplayName(LOCTEXT("AdvancedDeletionTab", "Advanced Deletion"))
		// This doesn't actually work...SetDisplayName() is tabs use name from SetDisplayName() automatically
		.SetTooltipText(LOCTEXT("AdvancedDeletionTooltip", "Tool for filtering and deleting assets using specified conditions."));
}

TSharedRef<SDockTab> FUdemyCourseModule::OnSpawnAdvancedDeletionTab(const FSpawnTabArgs& SpawnTabArgs)
{
	// Safety gaurd in case editor crashes when invoking tab from tools menu
	if (SelectedPaths.IsEmpty())
	{
		// Return an empty dock tab
		return SNew(SDockTab).TabRole(ETabRole::NomadTab);
	}

	ConstructedDockTab = SNew(SDockTab)
	.TabRole(ETabRole::NomadTab)
	// --Option A--. Cursor added this, but there is a more-common method SetOnTabClosed()
	//.OnTabClosed_Lambda([this](TSharedRef<SDockTab> TabBeingClosed)
	//{
	//	// Clear the reference when the tab is closed
	//	ConstructedDockTab.Reset();
	//})
	[
		SNew(SAdvancedDeletionTab) // The class name of the advanced deletion widget we've added
		//.TestString(TEXT("Hello world")) // TestString was renamed to AssetsDataToStore in AdvancedDeletionWidget.h
		.AssetsDataToStore(GetAssetDataInDirectory())
	];

	// --Option B--Clearing the reference when tab is closed
	ConstructedDockTab->SetOnTabClosed(SDockTab::FOnTabClosedCallback::CreateRaw(this, &FUdemyCourseModule::OnAdvancedDeletionTabClosed));

	return ConstructedDockTab.ToSharedRef();
}

TArray<TSharedPtr<FAssetData>> FUdemyCourseModule::GetAssetDataInDirectory()
{
	if (SelectedPaths.IsEmpty())
	{
		// Exit early to prevent crash from nullptr between sessions
		return TArray<TSharedPtr<FAssetData>>();
	}

	TArray<TSharedPtr<FAssetData>> AvailableAssetsData;
	TArray<FString> AssetPathNames = UEditorAssetLibrary::ListAssets(SelectedPaths[0]);

	for (const FString& AssetPath: AssetPathNames)
	{
		// Prevent errors if asset isn't properly found
		if (!UEditorAssetLibrary::DoesAssetExist(AssetPath))
		{
			continue;
		}

		const FAssetData AssetData = UEditorAssetLibrary::FindAssetData(AssetPath);
		
		// Validate that the asset data is valid before adding it
		if (!AssetData.IsValid())
		{
			continue;
		}
		
		// Convert the asset data into a smart pointer (TSharedPtr) for use in slate and add to array
		AvailableAssetsData.Add(MakeShared<FAssetData>(AssetData));
	}

	return AvailableAssetsData;
}

void FUdemyCourseModule::OnAdvancedDeletionTabClosed(TSharedRef<SDockTab> InDockTab)
{
	if (ConstructedDockTab.IsValid())
	{
		ConstructedDockTab.Reset();
		// Empty the paths array to prevent reuse if tab is invoked from the toolbar
		SelectedPaths.Empty();
	}
}

#pragma endregion // CustomEditorTab

#pragma region ProcessData

bool FUdemyCourseModule::DeleteAssetFromWidget(const FAssetData& AssetData)
{
	if (AssetData.IsValid())
	{
		// Returns deletion state of the asset via DeleteAsset's bool return type
		return UEditorAssetLibrary::DeleteAsset(AssetData.GetObjectPathString());
	}

	// Asset didn't have a chance to delete, therefore return false deletion state
	return false;
}

bool FUdemyCourseModule::DeleteCheckedWidgetAssets(const TArray<FAssetData>& AssetsToDelete)
{
	for (const FAssetData& AssetToDelete : AssetsToDelete)
	{
		// The deleted assets is greater than 0
		if (UEditorAssetLibrary::DeleteAsset(AssetToDelete.GetObjectPathString()))
		{
			return true;
		}
	}

	return false;
}

void FUdemyCourseModule::GetUnusedAssetsForList(const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter, TArray<TSharedPtr<FAssetData>>& OutUnusedAssetsData)
{
	OutUnusedAssetsData.Empty();
	
	for (const TSharedPtr<FAssetData>& AssetDataSharedPtr : AssetsDataToFilter)
	{
		if (!AssetDataSharedPtr.IsValid())
		{
			continue;
		}
		
		TArray<FString> AssetReferencers = UEditorAssetLibrary::FindPackageReferencersForAsset(AssetDataSharedPtr->GetObjectPathString());

		if (AssetReferencers.IsEmpty())
		{
			OutUnusedAssetsData.Add(AssetDataSharedPtr);
		}
	}
}

void FUdemyCourseModule::GetDuplicateNameAssets(const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter, TArray<TSharedPtr<FAssetData>>& OutDuplicateNameAssetsData)
{
	OutDuplicateNameAssetsData.Empty();
	TMultiMap<FString, TSharedPtr<FAssetData>> DuplicateNameAssets;

	for (const TSharedPtr<FAssetData>& AssetData : AssetsDataToFilter)
	{
		if (!AssetData.IsValid())
		{
			continue;
		}

		// Emplace is generally more-efficient than .Add(), although not in this case because .ToString()
		// triggers a copy construction for the return value. 
		// Emplace would be more-efficient if the value is directly-forwarded to the container class.
		DuplicateNameAssets.Emplace(AssetData->AssetName.ToString(), AssetData);
	}

	TArray<TSharedPtr<FAssetData>> OutFoundDuplicateNames;

	// Iterate over list of duplicate names only
	for (const TSharedPtr<FAssetData>& AssetData : AssetsDataToFilter)
	{
		OutFoundDuplicateNames.Empty();
		DuplicateNameAssets.MultiFind(AssetData->AssetName.ToString(), OutFoundDuplicateNames);

		// No duplicates exist for the asset name
		if (OutFoundDuplicateNames.Num() <= 1)
		{
			continue;
		}

		for (const TSharedPtr<FAssetData>& DuplicateNameAssetData : OutFoundDuplicateNames)
		{
			if (DuplicateNameAssetData.IsValid())
			{
				// AddUnqiue() is safer than Add() for fault protection
				OutDuplicateNameAssetsData.AddUnique(DuplicateNameAssetData);
			}
		}
	}
}

void FUdemyCourseModule::SyncCBToSelectedRows(const TArray<FString>& SelectedRowAssets)
{
	UEditorAssetLibrary::SyncBrowserToObjects(SelectedRowAssets);
}

TArray<AActor*> FUdemyCourseModule::GetSelectedActors()
{
	if (!GetEditorActorSubsystem())
	{
		return TArray<AActor*>();
	}

	return WeakEditorActorSubsystem->GetSelectedLevelActors();
}

#pragma endregion // ProcessData

void FUdemyCourseModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FName("AdvancedDeletion"));
	FUdemyCourseStyle::Shutdown();
	FUdemyCourseUICommands::Unregister();
	UnregisterSceneOutlinerColumnExtension();
}

#undef LOCTEXT_NAMESPACE

// Define the log category that was declared in DebugHeader.h.
DEFINE_LOG_CATEGORY(LogUdemyCourse);
	
IMPLEMENT_MODULE(FUdemyCourseModule, UdemyCourse)