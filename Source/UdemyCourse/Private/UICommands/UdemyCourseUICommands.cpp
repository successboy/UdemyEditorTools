// Copyright MODogma. All Rights Reserved.

#include "UICommands/UdemyCourseUICommands.h"

#define LOCTEXT_NAMESPACE "UdemyCourseUICommands"

void FUdemyCourseUICommands::RegisterCommands()
{
	// The first argument is the TSharedPtr class member
	UI_COMMAND(LockActorSelection, "Lock Actor Selection", "Lock the selected actors from being moved or re-selected", EUserInterfaceActionType::Button, FInputChord(EKeys::W, EModifierKey::Alt));
	UI_COMMAND(UnlockAllActors, "Unlock All Actors", "Unlocks all currently-locked actors", EUserInterfaceActionType::Button, FInputChord(EKeys::W, EModifierKey::Alt | EModifierKey::Shift));
}

#undef LOCTEXT_NAMESPACE