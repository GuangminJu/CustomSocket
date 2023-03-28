// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomSocketEditorCommands.h"

#define LOCTEXT_NAMESPACE "FCustomSocketEditorModule"

void FCustomSocketEditorCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "CustomSocketEditor", "Bring up CustomSocketEditor window", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
