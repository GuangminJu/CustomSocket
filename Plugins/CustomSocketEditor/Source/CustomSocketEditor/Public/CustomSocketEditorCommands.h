// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "CustomSocketEditorStyle.h"

class FCustomSocketEditorCommands : public TCommands<FCustomSocketEditorCommands>
{
public:

	FCustomSocketEditorCommands()
		: TCommands<FCustomSocketEditorCommands>(TEXT("CustomSocketEditor"), NSLOCTEXT("Contexts", "CustomSocketEditor", "CustomSocketEditor Plugin"), NAME_None, FCustomSocketEditorStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};