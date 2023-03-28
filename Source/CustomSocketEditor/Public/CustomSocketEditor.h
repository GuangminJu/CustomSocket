// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeCategories.h"
#include "Modules/ModuleManager.h"

class ISocketManager;
class FToolBarBuilder;
class FMenuBuilder;

class FCustomSocketEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command (by default it will bring up plugin window) */
	void PluginButtonClicked();

	static TSharedPtr<SCompoundWidget> CreateSocketManager(TSharedPtr<class IStaticMeshEditor> InStaticMeshEditor, FSimpleDelegate InOnSocketSelectionChanged );

	static EAssetTypeCategories::Type BYCAssetCategoryBit;
private:

	void RegisterMenus();

	void SpawnCustomSocketEditor();

private:
	TSharedPtr<class FUICommandList> PluginCommands;
};
