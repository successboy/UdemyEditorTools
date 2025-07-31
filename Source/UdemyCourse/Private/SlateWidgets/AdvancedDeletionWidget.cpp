// Copyright MODogma. All Rights Reserved.

#include "SlateWidgets/AdvancedDeletionWidget.h"
#include "Internationalization/Text.h" // This may not be required if already included in a core dependency header/module
#include "DebugHeader.h"
#include "UdemyCourse.h"

// Custom macros for combo box entries
#define LISTALL LOCTEXT("ComboBoxPtr", "All Assets")
#define LISTUNUSED LOCTEXT("ComboBoxPtr", "Unused Assets")
#define LISTDUPLICATES LOCTEXT("ComboBoxPtr", "Duplicate Names")

#define LOCTEXT_NAMESPACE "AdvancedDeletionWidget"

// Minimal constructor for memory object allocation of TSharedPtrs
SAdvancedDeletionTab::SAdvancedDeletionTab()
// Initialize the empty memory object
	: ClickedRowAssets(MakeShared<TArray<FString>>())
{

}

void SAdvancedDeletionTab::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;

	// Global variable in the header, and ASsetsDataToStore is the SLATE_ARGUMENT()
	// See FUdemyCourseModule::OnSpawnAdvancedDeletionTab and FUdemyCourseModule::GetAssetDataInDirectory
	StoredAssetsData = InArgs._AssetsDataToStore; // Get assets to add to list view
	DisplayedAssetsData = StoredAssetsData;

	// Clear memory for global reference/pointer arrays
	ConstructedCheckBoxes.Empty();
	StoredAssetsDataToDelete.Empty();
	ComboBoxSourceItems.Empty();

	// Add a first element to the combo box
	ComboBoxSourceItems.Add(MakeShared<FText>(LISTALL));
	ComboBoxSourceItems.Add(MakeShared<FText>(LISTUNUSED));
	ComboBoxSourceItems.Add(MakeShared<FText>(LISTDUPLICATES));

	FSlateFontInfo TitleTextFont = GetEmbossedTextFont();
	TitleTextFont.Size = 12.f;

	ChildSlot
	[
		// Root vertical box which can contain multiple child slots
		SNew(SVerticalBox)

		//// The first vertical slot for title text
		//+SVerticalBox::Slot()
		//.AutoHeight()
		//[
		//	SNew(STextBlock)
		//	.Text(LOCTEXT("AdvancedDeletion", "Advanced Deletion"))
		//	.Font(TitleTextFont)
		//	.Justification(ETextJustify::Center)
		//	.ColorAndOpacity(FColor::White)
		//	.ToolTipText(LOCTEXT("TooltipText", "TODO: Remove title...All plugins use dock tab text only, for title."))
		//]
		
		// Second vertical slot is for dropdown box to specify the listing condition
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				ConstructCurrentPathText()
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Filter", "Filter: "))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				ConstructComboBox()
			]
		]

		// TODO: Add a heading row with same number of columns as the list 
		// (likely need to explicitly-define the first list entry in OnGenerateRowForList()
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 10.f, 0.f, 10.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ColumnTitles", "TODO: Add column titles entry as a first row for Select/Asset Class/Asset Name"))
				.Justification(ETextJustify::Center)
			]
		]

		// Third slot is for the list
		+SVerticalBox::Slot()
		.VAlign(VAlign_Fill) // VAlign_Fill scroll box scrollable
		[
			SNew(SScrollBox)
			//.AllowOverscroll(EAllowOverscroll::Yes) // Doesn't enable interia scrolling, and is already default yes
			+SScrollBox::Slot()
			.Padding(0.f, 10.f)
			[
				ConstructList()
			]
		]

		// Fourth slot for three buttons: to populate the list and refresh results
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			//+ SHorizontalBox::Slot()
			//[
			//	SNew(SButton)
			//	.Text(LOCTEXT("ButtonText", "TODO button"))
			//	.HAlign(HAlign_Center)
			//	.ToolTipText(LOCTEXT("ButtonTooltip", "This is a tooltip for the button."))
			//]
			+SHorizontalBox::Slot()
			[
				ConstructSelectAllButton()
				//SNew(SButton)
				//.HAlign(HAlign_Center)
				//.Text(LOCTEXT("SelectAll", "Select All"))
				//.ToolTipText(LOCTEXT("SelectAllTooltip", "Select all assets in the current list."))
				//.OnClicked(this, &SAdvancedDeletionTab::OnSelectAllButtonClicked)
			]
			+SHorizontalBox::Slot()
			[
				ConstructDeselectAllButton()
				//SNew(SButton)
				//.HAlign(HAlign_Center)
				//.Text(LOCTEXT("DeselectAll", "Deselect All"))
				//.ToolTipText(LOCTEXT("DeselectAllTooltip", "Deselect all assets in the current list."))
				//.OnClicked(this, &SAdvancedDeletionTab::OnDeslectAllButtonClicked)
			]
			+SHorizontalBox::Slot()
			[
				ConstructDeleteSelectedButton()
				//SNew(SButton)
				//.HAlign(EHorizontalAlignment::HAlign_Center)
				//.Text(LOCTEXT("DeleteSelected", "Delete All"))
				//.ToolTipText(LOCTEXT("DeleteSelectedTooltip", "Deletes all assets in the current list."))
				//.OnClicked(this, &SAdvancedDeletionTab::OnDeleteSelectedbuttonClicked)
			]
		]
	];
}

TSharedRef<ITableRow> SAdvancedDeletionTab::OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay, const TSharedRef<STableViewBase>& OwnerTable)
{
	// Additional safety check to ensure the asset data is valid
	if (!AssetDataToDisplay->IsValid())
	{
		// Return an empty row widget for invalid asset data
		return SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable);
	}

	const FString DisplayAssetName = AssetDataToDisplay->AssetName.ToString();
	const FString DisplayAssetClass = AssetDataToDisplay->GetClass()->GetName();// AssetDataToDisplay->AssetClassPath.ToString();
	FSlateFontInfo AssetClassFont = GetEmbossedTextFont();
	AssetClassFont.Size = 10.f;

	// Construct rows of the table. The row defaults to one slot
	TSharedRef<STableRow<TSharedPtr<FAssetData>>> ListViewRowWidget = SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable)
		// Vertical gap between generated rows
		//.Padding(FMargin(0.f, 5.f, 0.f, 5.f))
		[
			SNew(SHorizontalBox)
			// First slot: Check box for asset selection
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.AutoWidth()
			.Padding(0.f, 0.f, 3.f, 0.f)
			//.FillWidth(.03f) // Gap between checkbox and asset name
			[
				ConstructCheckBox(AssetDataToDisplay)
			]

			//// Space between slots
			//+SHorizontalBox::Slot()
			//[
			//	SNew(SSpacer)
			//	.Size(FVector2D(5.f, 0.f))
			//]

			// Second slot: Display asset class name
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.AutoWidth()
			.Padding(0.f, 0.f, 6.f, 0.f)
			//.FillWidth(0.2f) // Truncates the text
			[
				ConstructRowText(FText::Format(LOCTEXT("AssetClass", "{0} |"), FText::FromString(DisplayAssetClass)), AssetClassFont)
			]

			//// Spacer between slots
			//+SHorizontalBox::Slot()
			//[
			//	SNew(SSpacer)
			//	.Size(FVector2D(5.f, 0.f))
			//]

			// Third slot: Display asset name
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.AutoWidth()
			//.FillWidth(1.f)
			[
				SNew(STextBlock)
				.Text(FText::Format(LOCTEXT("AssetName", "{0}"), FText::FromString(DisplayAssetName)))
			]

			// Fourth slot: Inline button to delete the asset directly
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			[
				ConstructButtonForRow(AssetDataToDisplay)
			]
		];

		return ListViewRowWidget;
}

TSharedRef<SCheckBox> SAdvancedDeletionTab::ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SCheckBox> ConstructedCheckBox = SNew(SCheckBox)
		.Type(ESlateCheckBoxType::CheckBox)
		.OnCheckStateChanged(this, &SAdvancedDeletionTab::OnCheckBoxStateChanged, AssetDataToDisplay)
		.Visibility(EVisibility::Visible);

	// Add to array for select all button
	ConstructedCheckBoxes.Add(ConstructedCheckBox);
	return ConstructedCheckBox;
}

void SAdvancedDeletionTab::OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData)
{
	switch (NewState)
	{
	case ECheckBoxState::Unchecked:
		if (StoredAssetsDataToDelete.Contains(AssetData))
		{
			StoredAssetsDataToDelete.Remove(AssetData);
		}
		break;

	case ECheckBoxState::Checked:
		StoredAssetsDataToDelete.AddUnique(AssetData);
		break;

	case ECheckBoxState::Undetermined:
		DebugHeader::PrintDebugMessage(AssetData->GetFullName() + TEXT(" undetermined"), FColor::Yellow);
		break;

	default:
		break;
	}
}

TSharedRef<STextBlock> SAdvancedDeletionTab::ConstructRowText(const FText& ContentText, const FSlateFontInfo& ContentFont)
{
	TSharedRef<STextBlock> ConstructedTextBlock = SNew(STextBlock)
		.Text(ContentText)
		.Font(ContentFont)
		.ToolTipText(LOCTEXT("AssetFilter", "Select a filter to update the list view."))
		.ColorAndOpacity(FColor::White);
	return ConstructedTextBlock;
}

TSharedRef<SComboBox<TSharedPtr<FText>>> SAdvancedDeletionTab::ConstructComboBox()
{
	TSharedRef<SComboBox<TSharedPtr<FText>>> ConstructedComboBox = SNew(SComboBox<TSharedPtr<FText>>)
		.OptionsSource(&ComboBoxSourceItems)
		// For the second argument (function binding) to work, needs to return a TSharedReference (see header)
		.OnGenerateWidget(this, &SAdvancedDeletionTab::OnGenerateComboBoxContent)
		.ToolTipText(LOCTEXT("ComboBoxTooltip", "Choose a list filter to narrow results."))
		.OnSelectionChanged(this, &SAdvancedDeletionTab::OnComboBoxSelectionChanged)
		[
			// SAssignNew is a specific macro for displaying combo box content. Arg1: member var, Arg2: slate class
			SAssignNew(ComboBoxTextBlock, STextBlock)
			.Text(LOCTEXT("FilterHint", "Select a Filter")) // The default display text of the combo box
		];

	return ConstructedComboBox;
}

TSharedRef<SWidget> SAdvancedDeletionTab::OnGenerateComboBoxContent(TSharedPtr<FText> SourceItem)
{
	TSharedRef<STextBlock> ConstructedComboBoxText = SNew(STextBlock)
	.Text(*SourceItem.Get());

	return ConstructedComboBoxText;
}

void SAdvancedDeletionTab::OnComboBoxSelectionChanged(TSharedPtr<FText> SelectedOption, ESelectInfo::Type InSelectInfo)
{

	if (SelectedOption.IsValid())
	{

		FUdemyCourseModule& UdemyCourseModule = FModuleManager::LoadModuleChecked<FUdemyCourseModule>(TEXT("UdemyCourse"));

		ComboBoxTextBlock->SetText(*SelectedOption.Get());

		// Pass data to the module to filter the list view
		if (SelectedOption->EqualTo(LISTALL))
		{
			DisplayedAssetsData = StoredAssetsData;
			// Unneeded? The list is updating properly without it
			RefreshList();
		}
		else if (SelectedOption->EqualTo(LISTUNUSED))
		{
			UdemyCourseModule.GetUnusedAssetsForList(StoredAssetsData, DisplayedAssetsData);
			// Calling from FUdemyCourseModule::OnAdvancedDeletionButtonClicked(), instead.
			// Calling here requires FixupRedirectors to have public access
			//UdemyCourseModule.FixupRedirectors();
			RefreshList();
		}
		else if (SelectedOption->EqualTo(LISTDUPLICATES))
		{
			UdemyCourseModule.GetDuplicateNameAssets(StoredAssetsData, DisplayedAssetsData);
			RefreshList();
		}
	}
}

TSharedRef<STextBlock> SAdvancedDeletionTab::ConstructButtonText(const FText& ContentText)
{
	FSlateFontInfo ContentFont = GetEmbossedTextFont();
	
	TSharedRef<STextBlock> ConstructedButtonText = SNew(STextBlock)
		.Text(ContentText)
		.Font(ContentFont);
	return ConstructedButtonText;
}

TSharedRef<STextBlock> SAdvancedDeletionTab::ConstructCurrentPathText()
{
	FSlateFontInfo ContentFont = GetEmbossedTextFont();
	FUdemyCourseModule& UdemyCourseModule = FModuleManager::LoadModuleChecked<FUdemyCourseModule>(TEXT("UdemyCourse"));
	TArray<FString> SelectedPaths = UdemyCourseModule.GetSelectedPaths();
	FText PathText;

	// Bounds check to guard against crash if tab opened with no path selected, between sessions
	if (!SelectedPaths.IsEmpty())
	{
		PathText = FText::Format(LOCTEXT("CurrentPath", "Current Path: {0} | "), FText::FromString(SelectedPaths[0]));
	}
	else
	{
		PathText = LOCTEXT("NoPathSelected", "Current Path: None | ");
	}

	TSharedRef<STextBlock> ConstructedCurrentPathText = SNew(STextBlock)
		.Text(PathText)
		.AutoWrapText(true) // Doesn't seem to be working from here
		.Font(ContentFont);

	return ConstructedCurrentPathText;
}

TSharedRef<SButton> SAdvancedDeletionTab::ConstructSelectAllButton()
{
	TSharedRef<SButton> ConstructedSelectAllButton = SNew(SButton)
		.HAlign(HAlign_Center)
		.ToolTipText(LOCTEXT("SelectAllTooltip", "Select all assets in the current list."))
		.OnClicked(this, &SAdvancedDeletionTab::OnSelectAllButtonClicked);

	ConstructedSelectAllButton->SetContent(ConstructButtonText(LOCTEXT("SelectAll", "Select All")));
	return ConstructedSelectAllButton;
}

TSharedRef<SButton> SAdvancedDeletionTab::ConstructDeselectAllButton()
{
	TSharedRef<SButton> ConstructedDeselectAllButton = SNew(SButton)
		.HAlign(HAlign_Center)
		.ToolTipText(LOCTEXT("DeselectAllTooltip", "Deselect all assets in the current list."))
		.OnClicked(this, &SAdvancedDeletionTab::OnDeselectAllButtonClicked);

	ConstructedDeselectAllButton->SetContent(ConstructButtonText(LOCTEXT("DeselectAll", "Deselect All")));
	return ConstructedDeselectAllButton;
}

TSharedRef<SButton> SAdvancedDeletionTab::ConstructDeleteSelectedButton()
{
	TSharedRef<SButton> ConstructedDeleteSelectedButton = SNew(SButton)
		.HAlign(EHorizontalAlignment::HAlign_Center)
		//.Text(LOCTEXT("DeleteSelected", "Delete Selected"))
		.ToolTipText(LOCTEXT("DeleteSelectedTooltip", "Deletes all assets in the current list."))
		.OnClicked(this, &SAdvancedDeletionTab::OnDeleteSelectedButtonClicked);

	// TODO: This seems to be a waste and is overly-modular, using .Text() and .Font() from above slate code is fine
	ConstructedDeleteSelectedButton->SetContent(ConstructButtonText(LOCTEXT("DeleteSelected", "Delete Selected")));
	return ConstructedDeleteSelectedButton;
}

TSharedRef<SListView<TSharedPtr<FAssetData>>> SAdvancedDeletionTab::ConstructList()
{
	ConstructedList = SNew(SListView<TSharedPtr<FAssetData>>)
		//.ItemHeight(24.f) // Warning C4996: Soon-deprecated API member
		.ListItemsSource(&DisplayedAssetsData)
		.OnGenerateRow(this, &SAdvancedDeletionTab::OnGenerateRowForList)
		.ScrollBarPadding(FMargin(6.f, 0.f)) // Padding to start content away from scrollbar
		.ToolTipText(LOCTEXT("RowTooltip", "Select one or more rows to select assets in the content browser."))
		// Better than .OnMouseButtonClicke(), which is single-selection
		.OnSelectionChanged(this, &SAdvancedDeletionTab::OnAssetListViewSelectionChanged) 
		.SelectionMode(ESelectionMode::Multi); // For syncing the CB for multiple assetsc

	// Convert the TSharedPtr->TSharedRef, to satisfy the return type
	return ConstructedList.ToSharedRef();
}

TSharedRef<SButton> SAdvancedDeletionTab::ConstructButtonForRow(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SButton> ConstructedButton = SNew(SButton)
		.Text(LOCTEXT("Delete", "Delete Item"))
		.ToolTipText(LOCTEXT("DeleteRowButtonTooltip", "Delete the single asset in the row."))
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Fill)
		.OnClicked(this, &SAdvancedDeletionTab::OnDeleteButtonClicked, AssetDataToDisplay);

	return ConstructedButton;
}

FReply SAdvancedDeletionTab::OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData)
{
	FUdemyCourseModule& UdemyCourseModule = FModuleManager::LoadModuleChecked<FUdemyCourseModule>(TEXT("UdemyCourse"));

	if (ClickedAssetData.IsValid())
	{
		const bool bAssetDeleted = UdemyCourseModule.DeleteAssetFromWidget(*ClickedAssetData.Get());

		if (bAssetDeleted)
		{
			if (StoredAssetsData.Contains(ClickedAssetData))
			{
				// Update the displayed results, as well, which will take the current filter into account
				if (DisplayedAssetsData.Contains(ClickedAssetData))
				{
					DisplayedAssetsData.Remove(ClickedAssetData);
				}

				StoredAssetsData.Remove(ClickedAssetData);
				DebugHeader::ShowNotification(
					FText::Format(
						LOCTEXT("AssetDeleted", "Asset deleted: {0}"),
						FText::FromString(ClickedAssetData->GetObjectPathString())
					)
				);

				// Refresh the list to remove the deleted asset's row
				RefreshList();
			}
			else
			{
				DebugHeader::ShowNotification(
					FText::Format(
						LOCTEXT("AssetNotDeleted", "Asset {0} was not found, skipping deletion"),
						FText::FromString(ClickedAssetData->GetObjectPathString())
					),
					ELogVerbosity::Error
				);
			}
		}
	}

	return FReply::Handled();
}

FReply SAdvancedDeletionTab::OnSelectAllButtonClicked()
{
	if (ConstructedCheckBoxes.IsEmpty())
	{
		return FReply::Handled(); 
	}

	// Iterate over the check box pointers and call the set the checkbox state to checked 
	for (const TSharedRef<SCheckBox>& CheckBoxRef : ConstructedCheckBoxes)
	{
		CheckBoxRef->SetIsChecked(ECheckBoxState::Checked);
	}

	return FReply::Handled();
}

FReply SAdvancedDeletionTab::OnDeselectAllButtonClicked()
{
	if (ConstructedCheckBoxes.IsEmpty())
	{
		return FReply::Handled();
	}

	for (const TSharedRef<SCheckBox>& CheckBoxRef : ConstructedCheckBoxes)
	{
		CheckBoxRef->SetIsChecked(ECheckBoxState::Unchecked);
	}

	return FReply::Handled();
}

FReply SAdvancedDeletionTab::OnDeleteSelectedButtonClicked()
{
	if (StoredAssetsDataToDelete.IsEmpty())
	{
		DebugHeader::ShowNotification(
			FText::Format(
				LOCTEXT("DeleteSelectedWarning", "There are no assets checked in the outliner, skipping deletion."),
				ELogVerbosity::Warning
			)
		);

		return FReply::Handled();
	}

	TArray<FAssetData> AssetsDataToDelete;

	// Convert the array data to a non-pointer
	for (const TSharedPtr<FAssetData>& AssetData : StoredAssetsDataToDelete)
	{
		AssetsDataToDelete.Add(*AssetData.Get());
	}

	// Pass data to the module for deletion
	FUdemyCourseModule& UdemyCourseModule = FModuleManager::LoadModuleChecked<FUdemyCourseModule>(TEXT("UdemyCourse"));
	const bool bAssetsDeleted = UdemyCourseModule.DeleteCheckedWidgetAssets(AssetsDataToDelete);

	if (bAssetsDeleted)
	{
		// Clear the dangling pointers
		for (const TSharedPtr<FAssetData>& DanglingData : StoredAssetsDataToDelete)
		{
			// Compare to the list pointers and sync the data
			if (StoredAssetsData.Contains(DanglingData))
			{
				StoredAssetsData.Remove(DanglingData);
			}

			if (DisplayedAssetsData.Contains(DanglingData))
			{
				DisplayedAssetsData.Remove(DanglingData);
			}
		}
		
		// Refresh the list after any assets have been deleted
		RefreshList();
	}

	return FReply::Handled();
}

void SAdvancedDeletionTab::OnAssetListViewSelectionChanged(TSharedPtr<FAssetData> SelectedItems, ESelectInfo::Type SelectInfo)
{
	// ClickedRowAssets is initialized in the constructor, safe to access directly without valid check
	ClickedRowAssets->Empty();

	TArray<TSharedPtr<FAssetData>> CurrentSelectedItems;
	ConstructedList->GetSelectedItems(CurrentSelectedItems);

	// Iterate over each selected item and add its object path for syncing the CB
	for (const TSharedPtr<FAssetData>& AssetData : CurrentSelectedItems)
	{
		if (AssetData.IsValid())
		{
			ClickedRowAssets->Add(AssetData->GetObjectPathString());
		}
	}

	FUdemyCourseModule& UdemyCourseModule = FModuleManager::LoadModuleChecked<FUdemyCourseModule>(TEXT("UdemyCourse"));
	UdemyCourseModule.SyncCBToSelectedRows(*ClickedRowAssets);
}

void SAdvancedDeletionTab::RefreshList()
{
	// Clear any danging pointers from memory
	StoredAssetsDataToDelete.Empty();
	ConstructedCheckBoxes.Empty();

	if (ConstructedList.IsValid())
	{
		ConstructedList->RebuildList();
	}
}


#undef LOCTEXT_NAMESPACE