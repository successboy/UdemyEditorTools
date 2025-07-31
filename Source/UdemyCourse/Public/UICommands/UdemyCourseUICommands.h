// Copyright MODogma. All Rights Reserved.

#pragma once

#include "Framework/Commands/Commands.h"
#include "UdemyCourseStyle.h"

#define LOCTEXT_NAMESPACE "UdemyCourseUICommands"

class FUdemyCourseUICommands : public TCommands<FUdemyCourseUICommands>
{
public:
	// Constructor defaults
	FUdemyCourseUICommands() : TCommands<FUdemyCourseUICommands>(
		FName(TEXT("UdemyCourse")), // Doesn't seem to do anything
		LOCTEXT("ContextDescription", "Udemy Course - Level Editor"), // Shortcut category name
		NAME_None, // InContextParent
		FName(TEXT("UdemyCourse")) // No style is required, but NAME_None will cause a crash--so requires any arbitrary input
	) {}

	virtual void RegisterCommands() override;

	TSharedPtr<FUICommandInfo> LockActorSelection;
	TSharedPtr<FUICommandInfo> UnlockAllActors;
private:
};

#undef LOCTEXT_NAMESPACE