// Copyright MODogma. All Rights Reserved.

#pragma once 

#include "Widgets/SCompoundWidget.h"

class SAdvancedDeletionTab : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SAdvancedDeletionTab) {}
	// Input args are a key-value pair
	SLATE_ARGUMENT(TArray<TSharedPtr<FAssetData>>, AssetsDataToStore)
	SLATE_END_ARGS()

public:
	// The default constructor for shareable defaults
	SAdvancedDeletionTab();
	void Construct(const FArguments& InArgs);

private:
	TSharedPtr<STextBlock> ComboBoxTextBlock;
	TSharedPtr<SListView<TSharedPtr<FAssetData>>> ConstructedList;
	//TSharedPtr<FAssetData> ClickedAssetData;
	TArray<TSharedPtr<FAssetData>> StoredAssetsData;
	/** .ListItemsSource() for SListView */
	TArray<TSharedPtr<FAssetData>> DisplayedAssetsData;
	TSharedPtr<TArray<FString>> ClickedRowAssets;
	/** For passing the displayed data to the module for processing */
	TArray<TSharedPtr<FAssetData>> StoredAssetsDataToDelete;
	TArray<TSharedRef<SCheckBox>> ConstructedCheckBoxes;
	TArray<TSharedPtr<FText>> ComboBoxSourceItems;

	/** Helper functions for readability and encapsulation of repetitive UI components */
	TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay, const TSharedRef<STableViewBase>& OwnerTable);
	TSharedRef<SCheckBox> ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay);
	TSharedRef<SButton> ConstructButtonForRow(const TSharedPtr<FAssetData>& AssetDataToDisplay);
	TSharedRef<SButton> ConstructSelectAllButton();
	TSharedRef<SButton> ConstructDeselectAllButton();
	TSharedRef<SButton> ConstructDeleteSelectedButton();
	TSharedRef<STextBlock> ConstructRowText(const FText& ContentText, const FSlateFontInfo& ContentFont);
	TSharedRef<STextBlock> ConstructButtonText(const FText& ContentText);
	TSharedRef<STextBlock> ConstructCurrentPathText();
	TSharedRef<SComboBox<TSharedPtr<FText>>> ConstructComboBox();
	TSharedRef<SWidget> OnGenerateComboBoxContent(TSharedPtr<FText> SourceItem);
	// Returns the same type as the list view in the SScrollBox of the slate code
	TSharedRef<SListView<TSharedPtr<FAssetData>>> ConstructList();

	FReply OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData);
	FReply OnSelectAllButtonClicked();
	FReply OnDeselectAllButtonClicked();
	FReply OnDeleteSelectedButtonClicked();

	// This is a getter pure function, which only returns the expression within braces
	FSlateFontInfo GetEmbossedTextFont() const { return FCoreStyle::Get().GetFontStyle(FName("EmbossedText")); }

	void OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData);
	void OnComboBoxSelectionChanged(TSharedPtr<FText> SelectedOption, ESelectInfo::Type InSelectInfo);
	void OnAssetListViewSelectionChanged(TSharedPtr<FAssetData> SelectedItems, ESelectInfo::Type SelectInfo);
	/** Wraps ConstructedList pointer in a valid check for abstraction(simplifies the code) */
	void RefreshList();

};

