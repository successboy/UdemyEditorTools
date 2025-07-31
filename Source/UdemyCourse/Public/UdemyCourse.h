// Copyright MODogma. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "SceneOutlinerModule.h"

class FUdemyCourseModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:

#pragma region LevelEditorMenuExtension
	void InitLevelEditorExtension();
	void AddLevelEditorMenuEntry(class FMenuBuilder& InMenuBuilder);
	TSharedRef<FExtender> CustomLevelEditorMenuExtender(const TSharedRef<FUICommandList> InUICommandList, const TArray<AActor*> InSelectedActors);

public:
	void OnLockSelectedActorButtonClicked();
	void OnUnlockSelectedActorButtonClicked();
	void OnUnlockAllActorsButtonClicked();

#pragma endregion
private:
#pragma region SelectionLock
	void InitCustomSelectionEvent();
	void OnActorSelected(UObject* InSelectedObject);
	//void UnlockSelectedActor(AActor* ActorToProcess);
	void UnlockAllActors();
	/** Refreshes the scene outliner so that widget states will be synced with level editor changes */
	void RefreshSceneOutliner();

public:
	void LockSelectedActor(AActor* ActorToProcess);
	void UnlockSelectedActor(AActor* ActorToProcess);
	// Needs to be public so it can be accessed externally, for SceneOutliner
	bool IsActorLocked(AActor* ActorToProcess);
	//AActor* SelectedActor;

#pragma endregion

private:

#pragma region CustomUICommands
	// Forward declaring FUICommandList with class keyword reduces compile time as the full
	// definition and members are unneeded, since we are simply additively extending UI commands
	TSharedPtr<class FUICommandList> CustomUICommands;
	void InitCustomUICommands();
	void OnLockSelectedActorShortcutPressed();
	void OnUnlockAllActorsShortcutPressed();
#pragma endregion

#pragma region SceneOutlinerExtension

	void InitSceneOutlinerExtension();
	// Preventing inclusioon of the two header files by forward declaring both classes
	TSharedRef<class ISceneOutlinerColumn> OnCreateActorLockColumn(class ISceneOutliner& SceneOutliner);
	void UnregisterSceneOutlinerColumnExtension();
	
#pragma endregion

private:
	/** Cannot mark the subsystem as a UPROPERTY(), so object point is next-best option for auto-mem management */
	TWeakObjectPtr<class UEditorActorSubsystem> WeakEditorActorSubsystem;
	bool GetEditorActorSubsystem();

#pragma region ContentBrowserMenuExtension

	TSharedRef<FExtender> CustomCBMenuExtender(const TArray<FString>& InSelectedPaths);
	TArray<FString> SelectedPaths;
	void InitCBMenuExtension();
	void AddCBMenuEntry(class FMenuBuilder& MenuBuilder);
	void OnDeleteUnusedAssetsButtonClicked();
	void OnDeleteEmptyFoldersButtonClicked();
	void OnAdvancedDeletionButtonClicked();
	bool FixupRedirectors();
	
	// Edit conditions for menu entries
	bool CanExecuteDeleteUnusedAssets();
	bool CanExecuteDeleteEmptyFolders();
#pragma endregion

#pragma region CustomEditorTab

	TSharedRef<SDockTab> OnSpawnAdvancedDeletionTab(const FSpawnTabArgs& SpawnTabArgs);

	/** Used as an edit condition (when the reference is valid) for the advanced deletion menu entries  */
	TSharedPtr<SDockTab> ConstructedDockTab;
	TArray<TSharedPtr<FAssetData>> GetAssetDataInDirectory();

	void RegisterAdvancedDeletionTab();
	void OnAdvancedDeletionTabClosed(TSharedRef<SDockTab> InDockTab);

public:
	void SyncCBToSelectedRows(const TArray<FString>& SelectedRowAssets);
	/** Getter for passing data to the widget */
	TArray<FString> GetSelectedPaths() {return SelectedPaths;}
#pragma endregion

public:

#pragma region ProcessData

	/** 
	 * Delete an asset from a button in the ListView widget element.
	 * Returns true if asset was successfully deleted
	 */
	bool DeleteAssetFromWidget(const FAssetData& AssetData);

	bool DeleteCheckedWidgetAssets(const TArray<FAssetData>& AssetsToDelete);
	void GetUnusedAssetsForList(const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter, TArray<TSharedPtr<FAssetData>>& OutUnusedAssetsData);

	/** 
	 * The new Epic standard is to use package names instead for this reason, as duplicate
	 * names sometimes occur an disparate assets. The better check would be resource size && asset class && asset name
	 */
	void GetDuplicateNameAssets(const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter, TArray<TSharedPtr<FAssetData>>& OutDuplicateNameAssetsData);

	/** Returns false if there are no selected level actors */
	bool IsLevelActorSelected();

	TArray<AActor*> GetSelectedActors();
#pragma endregion
};
