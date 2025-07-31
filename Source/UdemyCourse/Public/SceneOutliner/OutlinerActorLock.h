// Copyright MODogma. All Rights Reserved.

#pragma once

#include "ISceneOutlinerColumn.h"

class FOutlinerActorLock : public ISceneOutlinerColumn
{
public:
	// Empty constructor needed for column registration
	FOutlinerActorLock(ISceneOutliner& SceneOutliner) {}

	// Required pure functions from ISceneOutliner
	virtual FName GetColumnID() override {return FName("ActorLock");}
	virtual SHeaderRow::FColumn::FArguments ConstructHeaderRowColumn() override;
	virtual const TSharedRef<SWidget> ConstructRowWidget(FSceneOutlinerTreeItemRef InTreeItem, const STableRow<FSceneOutlinerTreeItemPtr>& InRow) override;

	// Fix for errors C2039 & C3861
	static FName GetID() {return FName("ActorLock");}

private:

	void OnRowCheckStateChanged(ECheckBoxState InNewState, TWeakObjectPtr<AActor> InLinkedActor);
};