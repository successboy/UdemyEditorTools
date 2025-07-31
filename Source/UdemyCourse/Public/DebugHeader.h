// Copyright MODogma. All Rights Reserved.

#pragma once
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "CoreMinimal.h"
#include "Misc/MessageDialog.h"

#define ENABLE_UDEMYCOURSE_LOG 1 // See InstanceToolModule.h

DECLARE_LOG_CATEGORY_EXTERN(LogUdemyCourse, Log, All);

#if ENABLE_UDEMYCOURSE_LOG
#define UDEMYCOURSE_LOG(Verbosity, Format, ...) \
	UE_LOG(LogUdemyCourse, Verbosity, Format, ##__VA_ARGS__);
#else
#define UDEMYCOURSE_LOG(Verbosity, Format, ...)
#endif

#define LOCTEXT_NAMESPACE "DebugHeader" // Required for LOCTEXT() macro

namespace DebugHeader
{
	static void PrintDebugMessage(const FString& Message, const FColor& Color = FColor::Cyan)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 8.f, Color, TEXT("UdemyDebug: ") + Message);
		}

		UE_LOG(LogUdemyCourse, Log, TEXT("%s"), *Message);
	}

	//// UE_LOG already does this...
	//void PrintLog(const FString& Message)
	//{
	//	UE_LOG(LogUdemyCourse, Warning, TEXT("%s"), *Message);
	//}

	/** Task status notification for displaying non-blocking in-editor toast message to user
	 * Reference FLiveCodingModule::ShowNotification for a good example
	 */
	static void ShowNotification(FText Message, ELogVerbosity::Type LogVerbosity = ELogVerbosity::Log, SNotificationItem::ECompletionState State = SNotificationItem::CS_None, float ExpireDuration = 5.f)
	{
		FNotificationInfo Info(Message);
		Info.bFireAndForget = true;
		Info.ExpireDuration = ExpireDuration;
		Info.bUseThrobber = true;
		//Info.bUseSuccessFailIcons = true; // Use for Method B

		// -- Method A -- Checking verbosity states to set icon before calling notification
		if (LogVerbosity == ELogVerbosity::Error)
		{
			Info.Image = FAppStyle::GetBrush(TEXT("Icons.Error")); // (Icons.) in SlateEditorStyle.cpp
			UE_LOG(LogUdemyCourse, Error, TEXT("%s"), *Message.ToString());
		}
		else if (LogVerbosity == ELogVerbosity::Warning)
		{
			Info.Image = FAppStyle::GetBrush(TEXT("Icons.Warning")); // (Icons.) in SlateEditorStyle.cpp
			UE_LOG(LogUdemyCourse, Warning, TEXT("%s"), *Message.ToString());
		}
		else
		{
			Info.Image = FAppStyle::GetBrush(TEXT("Icons.Info")); // (Icons.) in SlateEditorStyle.cpp
			UE_LOG(LogUdemyCourse, Log, TEXT("%s"), *Message.ToString());
		}
		// End Method A

		//// Add the notification to the manager
		TSharedPtr<SNotificationItem> NotificationPtr = FSlateNotificationManager::Get().AddNotification(Info);

		//// If it's a pending state for progress, store the pointer
		//if (State == SNotificationItem::CS_Pending && NotificationPtr.IsValid())
		//{
			// MyPendingNotification = NotificationPtr;
		//}


		// -- Method B -- Setting completion states based on verbosity level, but don't need this for now and...
		// ...Potential TODO: Remove the verbosity checks and keep them above the NotificationPtr .AddNotification()
		// Also, print message to the log. UE_Log is a preprocessor so LogVerbosity input is explicitly defined
		//if (LogVerbosity == ELogVerbosity::Error)
		//{
		//	//Info.Image = FAppStyle::GetBrush(TEXT("MessageLog.Error"));
		//	Info.Image = FAppStyle::GetBrush(TEXT("Icons.Error")); // Icons in SlateEditorStyle.cpp
		//	FSlateNotificationManager::Get().AddNotification(Info); // Calling this for the image, intead of SetCompletionState()
		//	if (NotificationPtr)
		//	{
		//		// Using for icon when Info.bUseSuccessFailIcons = true
		//		NotificationPtr->SetCompletionState(SNotificationItem::CS_Fail);
		//	}
		//	UE_LOG(LogTemp, Error, TEXT("%s"), *Message.ToString());
		//}
		//else if (LogVerbosity == ELogVerbosity::Warning)
		//{
		//	if (NotificationPtr)
		//	{
		//		// Using for icon when Info.bUseSuccessFailIcons = true 
		//		//NotificationPtr->SetCompletionState(SNotificationItem::CS_None); // Hack: Calling .AddNotification() instead, for using image
		//	}
		//	// This is required since SNoficationItem has no warning state. 
		//	// This doesn't work here as notification needs calling after. Trade-off is to use CompletionState system
		//	//Info.Image = FAppStyle::GetBrush(TEXT("Graph.ConnectorFeedback.OKWarn"));
		//	Info.Image = FAppStyle::GetBrush(TEXT("Icons.Warning")); // Icons in SlateEditorStyle.cpp
		//	FSlateNotificationManager::Get().AddNotification(Info); // Calling this for the image, intead of SetCompletionState()
		//	UE_LOG(LogTemp, Warning, TEXT("%s"), *Message.ToString());
		//}
		//else
		//{
		//	if (NotificationPtr)
		//	{
		//		// Using for icon when Info.bUseSuccessFailIcons = true
		//		NotificationPtr->SetCompletionState(SNotificationItem::CS_Success);
		//	}
		//	//Info.Image = FAppStyle::GetBrush(TEXT("Graph.ConnectorFeedback.OK"));
		//	Info.Image = FAppStyle::GetBrush(TEXT("Icons.Info")); // Icons in SlateEditorStyle.cpp
		//	FSlateNotificationManager::Get().AddNotification(Info); // Calling this for the image, intead of SetCompletionState()
		//	UE_LOG(LogTemp, Log, TEXT("%s"), *Message.ToString());
		//}
		// End Method B
	}

	// Using FMessageDialog::Open() is better than this anyway, as it has proper optional return
	// value and many overload options for flexibility and min inputs
	/** Display a message dialog and handle the return condition from user input.
	 * Returns true for accept and false for decline.
	 */
	 //bool HandleMsgDialog(FText Message, EAppMsgCategory MsgCat = EAppMsgCategory::Info, EAppMsgType::Type MsgType = EAppMsgType::Ok)
	 //{
	 //	FText Title = LOCTEXT("DuplicatedAssetsNotification", "Information");
	 //
	 //	if (MsgCat == EAppMsgCategory::Error)
	 //	{
	 //		Title = LOCTEXT("DuplicatedAssetsNotification", "Error");
	 //	}
	 //	else if (MsgCat == EAppMsgCategory::Warning)
	 //	{
	 //		Title = LOCTEXT("DuplicatedAssetsNotification", "Warning");
	 //	}
	 //
	 //	FMessageDialog::Open(MsgCat, MsgType, Message, Title);
	 //
	 //	// Exit when user has declined
	 //	if (EAppReturnType::No || EAppReturnType::Cancel || EAppReturnType::NoAll)
	 //	{
	 //		UE_LOG(LogTemp, Log, TEXT("User has chosen cancel return type. Return false."));
	 //		return false;
	 //	}
	 //
	 //	UE_LOG(LogTemp, Log, TEXT("User hasn't chosen cancel return type. Return true."));
	 //	return true;
	 //}
}

#undef LOCTEXT_NAMESPACE // Required for LOCTEXT() macro