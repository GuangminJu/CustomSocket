// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomSocketEditor.h"

#include "AssetToolsModule.h"
#include "AssetTypeAction_SeatMap.h"
#include "CustomSocketEditorStyle.h"
#include "CustomSocketEditorCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "ToolMenus.h"
#include "Widgets/SCustomSocketEditorWidget.h"
#include "Widgets/SCustomSocketManager.h"

static const FName CustomSocketEditorTabName("CustomSocketEditorTabTitle");

EAssetTypeCategories::Type FCustomSocketEditorModule::BYCAssetCategoryBit;

#define LOCTEXT_NAMESPACE "FCustomSocketEditorModule"

void FCustomSocketEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FCustomSocketEditorStyle::Initialize();
	FCustomSocketEditorStyle::ReloadTextures();

	FCustomSocketEditorCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FCustomSocketEditorCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FCustomSocketEditorModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FCustomSocketEditorModule::RegisterMenus));

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	BYCAssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("BYC")), LOCTEXT("BYCAssetCategory", "BYC"));
	AssetTools.RegisterAssetTypeActions(MakeShared<FAssetTypeActions_SeatMap>());
}

void FCustomSocketEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FCustomSocketEditorStyle::Shutdown();

	FCustomSocketEditorCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(CustomSocketEditorTabName);
}

void FCustomSocketEditorModule::SpawnCustomSocketEditor()
{
	
}

void FCustomSocketEditorModule::PluginButtonClicked()
{
	SpawnCustomSocketEditor();
}

TSharedPtr<SCompoundWidget> FCustomSocketEditorModule::CreateSocketManager(
	TSharedPtr<IStaticMeshEditor> InStaticMeshEditor, FSimpleDelegate InOnSocketSelectionChanged)
{
	TSharedPtr<SCustomSocketManager> SocketManager;
	SAssignNew(SocketManager, SCustomSocketManager)
		.StaticMeshEditorPtr(InStaticMeshEditor)
		.OnSocketSelectionChanged(InOnSocketSelectionChanged);

	return SocketManager;
}

void FCustomSocketEditorModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FCustomSocketEditorCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(
					FToolMenuEntry::InitToolBarButton(FCustomSocketEditorCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCustomSocketEditorModule, CustomSocketEditor)
