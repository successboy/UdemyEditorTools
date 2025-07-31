// Copyright MODogma. All Rights Reserved.

#include "AssetActions/QuickAssetAction.h"
#include "DebugHeader.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h" // Create assets from code
//#include "ObjectTools.h" // for ObjectTools::DeleteAssets()

#define LOCTEXT_NAMESPACE "FQuickAssetAction" // Required for LOCTEXT() macro

void UQuickAssetAction::DuplicateAssets(int32 NumDuplicates)
{
	// Received invalid user input
	if (NumDuplicates <= 0)
	{
		DebugHeader::ShowNotification(LOCTEXT("DuplicatedAssetsNotification", "Incorrect input, no duplicates created."), ELogVerbosity::Error);
		return;
	}

	TArray<FAssetData> SelectedAssetData = UEditorUtilityLibrary::GetSelectedAssetData();
	uint32 NumDuplicated = 0;

	for (const FAssetData& AssetData : SelectedAssetData)
	{
		for (int32 i = 0; i < NumDuplicates; ++i)
		{
			const FString SourceAssetPath = AssetData.GetObjectPathString();
			const FString DuplicatedAssetName = AssetData.AssetName.ToString() + FString::Printf(TEXT("_%d"), i + 1);
			const FString TargetPathName = FPaths::Combine(AssetData.PackagePath.ToString(), DuplicatedAssetName);//TEXT("/Duplicated");

			if (UEditorAssetLibrary::DuplicateAsset(SourceAssetPath, TargetPathName))
			{
				UE_LOG(LogTemp, Log, TEXT("Asset duplicated"));
				UEditorAssetLibrary::SaveAsset(TargetPathName);
				++NumDuplicated;
			}
		}
	}

	if (NumDuplicated == 0)
	{
		DebugHeader::ShowNotification(LOCTEXT("DuplicatedAssetsNotification", "No duplicates created, check the code."), ELogVerbosity::Error);
		return;
	}

	DebugHeader::ShowNotification(FText::Format(LOCTEXT("DuplicatedAssetsNotification", "Successfully duplicated {0} asset(s)!"), NumDuplicated));
}

void UQuickAssetAction::AddPrefixes()
{
	TArray<UObject*> SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();
	uint32 Counter = 0;

	for (UObject* SelectedObject : SelectedObjects)
	{
		// Skip current object if it is not valid
		if (!SelectedObject)
		{
			UE_LOG(LogUdemyCourse, Error, TEXT("Invalid data for SelectedObject %s, skipping."), *SelectedObject->GetPackage()->GetName());
			continue;
		}

		FString* PrefixFound = PrefixMap.Find(SelectedObject->GetClass());

		// There is no prefix element in PrefixMap. It needs to be added for the class
		if (!PrefixFound || PrefixFound->IsEmpty())
		{
			DebugHeader::ShowNotification(FText::Format(LOCTEXT("PrefixFindFailed", "Failed to find prefix for asset class {0}, skipping."), FText::FromString(SelectedObject->GetClass()->GetName())), ELogVerbosity::Error);
			continue;
		}

		FString OldName = SelectedObject->GetName();
		FString CurrentName = OldName; // Storing separately so OldName can be used for logging

		// Prefix already exists, skip renaming
		if (OldName.StartsWith(*PrefixFound))
		{
			UE_LOG(LogUdemyCourse, Warning, TEXT("%s already has a proper prefix, not adding."), *OldName);
			continue;
		}

		// Remove M_ prefix and _Inst suffix from material instance class (default Epic naming).
		// This should be extended for more prefixes and suffixes, like Texture_,_BaseColor, etc.
		if (SelectedObject->IsA<UMaterialInstanceConstant>())
		{
			CurrentName.RemoveFromEnd(TEXT("_Inst"));
			CurrentName.RemoveFromStart(TEXT("M_"));
		}

		// Build the new name and rename asset
		const FString NewName = *PrefixFound + CurrentName;
		UEditorUtilityLibrary::RenameAsset(SelectedObject, NewName);
		UE_LOG(LogUdemyCourse, Log, TEXT("File renamed: %s->%s"), *OldName, *NewName);
		++Counter;
	}

	if (Counter > 0)
	{
		DebugHeader::ShowNotification(FText::Format(LOCTEXT("FinishedPrefixeRename", "Finished renaming {0} asset(s) with new prefix!"), FText::AsNumber(Counter)));
	}
	else
	{
		DebugHeader::ShowNotification(LOCTEXT("WarningMessage", "Asset(s) already have a proper prefix, not adding."), ELogVerbosity::Warning);
	}
}

void UQuickAssetAction::DeleteUnusedAssets(bool bInGetSelectedAssets)
{
	TArray<FAssetData> AssetsData;
	TArray<FAssetData> UnusedAssetsData;
	
	if (bInGetSelectedAssets)
	{
		AssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	}
	// Get all assets
	else
	{
		// TODO: Finish implementation
		//UE_LOG(LogUdemyCourse, Warning, TEXT("TODO: Finish get all assets implementation"));
		//HandleMsgDialog(LOCTEXT("DevMessage", "TODO: Finish get all assets implementation."), EAppMsgCategory::Warning);
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("DevMessage", "TODO: Finish get all assets implementation."));
		// Remove this when implementation finished
		return;
	}

	for (const FAssetData& AssetData : AssetsData)
	{
		// Only need if the unreferenced assets are needed later, i.e. for logging
		TArray<FString> AssetReferencers = UEditorAssetLibrary::FindPackageReferencersForAsset(AssetData.GetObjectPathString());

		// Delete assets with no references
		if (UEditorAssetLibrary::FindPackageReferencersForAsset(AssetData.GetSoftObjectPath().ToString()).IsEmpty())
		{
			UE_LOG(LogUdemyCourse, Log, TEXT("Unreferenced asset: %s"), *AssetData.PackageName.ToString());
			UnusedAssetsData.Add(AssetData);
		}
	}

	// Exit if there are no results
	if (UnusedAssetsData.IsEmpty())
	{
		DebugHeader::ShowNotification(LOCTEXT("NoUnusedAssets", "There are no unused assets in the current selection!"), ELogVerbosity::Warning);
		return;
	}

	// -- Option A -- The problem with this is that the window asks to delete even the referenced assets,
	// so deleting only the unused ones requires only unreferenced ones to be selected first
	//int32 NumDeletedAssets = ObjectTools::DeleteAssets(UnusedAssetsData);

	//if (NumDeletedAssets == 0)
	//{
	//	return;
	//}

	//ShowNotification(FText::Format(LOCTEXT("SuccessMessage", "Successfully deleted {0} unused assets!"), NumDeletedAssets));

	// -- Option B --
	// User has declined
	if (!FMessageDialog::Open(EAppMsgType::YesNo, FText::Format(LOCTEXT("UserInputRequest", "{0} assets found with no references. Would you like to delete them?"), UnusedAssetsData.Num())))
	{
		return;
	}
	
	// User has chosen to continue from the message dialog
	for (FAssetData UnusedAssetData : UnusedAssetsData)
	{
		UEditorAssetLibrary::DeleteAsset(UnusedAssetData.GetSoftObjectPath().ToString());
		UE_LOG(LogUdemyCourse, Log, TEXT("Deleted unreferenced asset: %s"), *UnusedAssetData.PackageName.ToString());
	}

	DebugHeader::ShowNotification(FText::Format(LOCTEXT("SuccessMessage", "Successfully deleted {0} unused asset(s)!"), UnusedAssetsData.Num()));
}

void UQuickAssetAction::FixUpAllRedirectors()
{
	TArray<UObjectRedirector*> RedirectorsToFix;

	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();

	FARFilter Filter;
	Filter.bRecursivePaths = true;
	// Get content root path to get all redirector assets
	Filter.PackagePaths.Emplace("/Game");
	// Filter.ClassPaths.Add(UObjectRedirector::StaticClass()->GetClassPathName()) also works, but has slight overhead
	Filter.ClassPaths.Emplace("/Script/CoreUObject.ObjectRedirector");
	TArray<FAssetData> OutRedirectors;
	// Fill OutRedirectors with filtered redirector assets
	AssetRegistry.GetAssets(Filter, OutRedirectors);

	for (const FAssetData& Redirector : OutRedirectors)
	{
		if (UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(Redirector.GetAsset()))
		{
			RedirectorsToFix.Add(RedirectorToFix);
			UE_LOG(LogUdemyCourse, Log, TEXT("Fixed up redirector: %s"), *Redirector.PackageName.ToString());
		}
	}

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	// This spawns its own window. No need to show notification
	AssetToolsModule.Get().FixupReferencers(RedirectorsToFix);
}

void UQuickAssetAction::RenameSelection(bool bAddPrefixes, FString NewName)
{
	TArray<UObject*> SelectedAssets = UEditorUtilityLibrary::GetSelectedAssets();
	int32 Counter = 1;

	// Iterate over all selected assets and rename
	for (UObject* SelectedAsset : SelectedAssets)
	{
		// Skip current asset if invalid
		if (!SelectedAsset)
		{
			UE_LOG(LogUdemyCourse, Error, TEXT("Invalid data for SelectedObject %s, skipping."), *SelectedAsset->GetPackage()->GetName());
			continue;
		}

		FString OldName = SelectedAsset->GetName();
		FString FinalName = NewName + TEXT("_") + FString::FromInt(Counter);

		if (bAddPrefixes)
		{
			FString* PrefixFound = PrefixMap.Find(SelectedAsset->GetClass());

			// Skip rename if proper prefix already exists
			if (FinalName.StartsWith(*PrefixFound))
			{
				UE_LOG(LogUdemyCourse, Warning, TEXT("The proper prefix already exists for %s, skipping."), *OldName);
				continue;
			}

			FinalName = *PrefixFound + FinalName;
		}

		UEditorUtilityLibrary::RenameAsset(SelectedAsset, *FinalName);
		UE_LOG(LogUdemyCourse, Log, TEXT("File renamed: %s->%s"), *OldName, *FinalName);
		++Counter;
	}

	DebugHeader::ShowNotification(FText::Format(LOCTEXT("RenameConfirmation", "Successfully renamed {0} assets!"), SelectedAssets.Num()));
}

#undef LOCTEXT_NAMESPACE // Required for LOCTEXT() macro
