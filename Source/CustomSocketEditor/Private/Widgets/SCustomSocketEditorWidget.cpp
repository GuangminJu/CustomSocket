// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/SCustomSocketEditorWidget.h"

#include "CustomSocketEditor.h"
#include "ISocketManager.h"
#include "LevelEditor.h"
#include "SCustomSocketManager.h"
#include "SlateOptMacros.h"
#include "StaticMeshEditorModule.h"

#define LOCTEXT_NAMESPACE "SocketEditor"

const FName CustomSocketEditorAppIdentifier = FName(TEXT("CustomSocketEditorApp"));

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

class ICustomSocketToolkitHost : public SCompoundWidget, public IToolkitHost
{
public:
	explicit ICustomSocketToolkitHost(UWorld* const World);
	virtual TSharedRef<SWidget> GetParentWidget() override;
	virtual void BringToFront() override;
	virtual TSharedRef<SDockTabStack> GetTabSpot(const EToolkitTabSpot::Type TabSpot) override;
	virtual TSharedPtr<FTabManager> GetTabManager() const override;
	virtual void OnToolkitHostingStarted(const TSharedRef<IToolkit>& Toolkit) override;
	virtual void OnToolkitHostingFinished(const TSharedRef<IToolkit>& Toolkit) override;
	virtual UWorld* GetWorld() const override;

private:
	UWorld* World;
};

ICustomSocketToolkitHost::ICustomSocketToolkitHost(UWorld* const World): World(World)
{
}

TSharedRef<SWidget> ICustomSocketToolkitHost::GetParentWidget()
{
	return AsShared();
}

void ICustomSocketToolkitHost::BringToFront()
{
}

TSharedRef<SDockTabStack> ICustomSocketToolkitHost::GetTabSpot(const EToolkitTabSpot::Type TabSpot)
{
	return TSharedPtr<SDockTabStack>().ToSharedRef();
}

TSharedPtr<FTabManager> ICustomSocketToolkitHost::GetTabManager() const
{
	const FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
	TSharedPtr<FTabManager> LevelEditorTabManager = LevelEditorModule.GetLevelEditorTabManager();
	return LevelEditorTabManager;
}

UWorld* ICustomSocketToolkitHost::GetWorld() const
{
	return World;
}

void ICustomSocketToolkitHost::OnToolkitHostingStarted(const TSharedRef<IToolkit>& Toolkit)
{
}

void ICustomSocketToolkitHost::OnToolkitHostingFinished(const TSharedRef<IToolkit>& Toolkit)
{
}

FStaticMeshSocketEditor::FStaticMeshSocketEditor(const FLinearColor& WorldCentricTabColorScale,
                                                 const TWeakObjectPtr<UStaticMesh>& StaticMesh,
                                                 const TWeakObjectPtr<UStaticMeshComponent>& StaticMeshComponent,
                                                 const TArray<TWeakObjectPtr<UStaticMeshSocket>>& SelectedSockets,
                                                 const bool InMutlipleSelect,
                                                 const EViewModeIndex ViewMode):
	WorldCentricTabColorScale(WorldCentricTabColorScale),
	StaticMesh(StaticMesh),
	StaticMeshComponent(StaticMeshComponent),
	SelectedSockets(SelectedSockets),
	MutlipleSelect(InMutlipleSelect),
	ViewMode(ViewMode)
{
}

void FStaticMeshSocketEditor::InitSocketEditor()
{
	const TSharedRef<FTabManager::FLayout> DefaultLayout = FTabManager::NewLayout("Custom Socket")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->Split
			(
				FTabManager::NewStack()
				->AddTab("Socket Edit", ETabState::OpenedTab)
			)
		);

	FAssetEditorToolkit::InitAssetEditor(EToolkitMode::Standalone, CustomSocketToolkitHost,
	                                     CustomSocketEditorAppIdentifier,
	                                     DefaultLayout, false, false,
	                                     StaticMesh.Get());
}

const FName FStaticMeshSocketEditor::CustomSocketEditorTabId(TEXT("StaticMeshEditor_CustomSocketEditor"));

void FStaticMeshSocketEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(
		LOCTEXT("WorkspaceMenu_CustomSocketEditor", "Custom Socket Editor"));
	const TSharedRef<FWorkspaceItem> WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(CustomSocketEditorTabId,
	                                 FOnSpawnTab::CreateSP(this, &FStaticMeshSocketEditor::SpawnTab_CustomSocketEditor))
	            .SetDisplayName(LOCTEXT("CustomSocketEditorTab", "Custom Socket Editor"))
	            .SetGroup(WorkspaceMenuCategoryRef)
	            .SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Viewports"));

}

TSharedRef<SDockTab> FStaticMeshSocketEditor::SpawnTab_CustomSocketEditor(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SCustomSocketEditorWidget)
		];
}

FStaticMeshSocketEditor::FStaticMeshSocketEditor(UWorld* InWorld):
	StaticMesh(nullptr),
	StaticMeshComponent(nullptr),
	SelectedSockets(),
	MutlipleSelect(false),
	ViewMode(EViewModeIndex::VMI_Lit),
	World(InWorld)
{
	CustomSocketToolkitHost = MakeShared<ICustomSocketToolkitHost>(World);

	for (TObjectIterator<UStaticMesh> It; It; ++It)
	{
		StaticMesh = *It;
		break;
	}

	StaticMeshComponent = NewObject<UStaticMeshComponent>(GetTransientPackage(), NAME_None, RF_Transient);
	StaticMeshComponent->SetStaticMesh(StaticMesh.Get());
}

FLinearColor FStaticMeshSocketEditor::GetWorldCentricTabColorScale() const
{
	return WorldCentricTabColorScale;
}

FName FStaticMeshSocketEditor::GetToolkitFName() const
{
	return "SocketEditor";
}

FText FStaticMeshSocketEditor::GetBaseToolkitName() const
{
	return LOCTEXT("SocketEditor", "Socket Editor");
}

FString FStaticMeshSocketEditor::GetWorldCentricTabPrefix() const
{
	return "SocketEditor";
}

void SCustomSocketEditorWidget::Construct(const FArguments& InArgs)
{
	PreviewScene = MakeShareable(new FAdvancedPreviewScene(FPreviewScene::ConstructionValues()));

	for (TObjectIterator<UStaticMesh> It; It; ++It)
	{
		StaticMesh = *It;
		break;
	}

	StaticMeshComponent = NewObject<UStaticMeshComponent>(GetTransientPackage(), NAME_None, RF_Transient);
	StaticMeshComponent->SetStaticMesh(StaticMesh);
	
	const TSharedPtr<ISocketManager> SocketManager = MakeShared<SCustomSocketManager>();
	SEditorViewport::Construct(SEditorViewport::FArguments());
	
	ChildSlot
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		[
			ViewportWidget.ToSharedRef()
		]
		+ SHorizontalBox::Slot()
		[
			SocketManager.ToSharedRef()
		]
	];
}

SCustomSocketEditorWidget::SCustomSocketEditorWidget()
	: PreviewScene(nullptr),
	  StaticMesh(nullptr),
	  CurrentViewMode(VMI_Lit),
	  LODSelection(0),
	  StaticMeshComponent(nullptr)
{
}

SCustomSocketEditorWidget::~SCustomSocketEditorWidget()
{
}

void SCustomSocketEditorWidget::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(StaticMesh);
}

TSharedRef<SEditorViewport> SCustomSocketEditorWidget::GetViewportWidget()
{
	return SharedThis(this);
}

TSharedPtr<FExtender> SCustomSocketEditorWidget::GetExtenders() const
{
	TSharedPtr<FExtender> Result(MakeShareable(new FExtender));
	return Result;
}

void SCustomSocketEditorWidget::OnFloatingButtonClicked()
{
}

TSharedRef<FEditorViewportClient> SCustomSocketEditorWidget::MakeEditorViewportClient()
{
	EditorViewportClient = MakeShareable(new FEditorViewportClient(nullptr, PreviewScene.Get(), SharedThis(this)));
	return EditorViewportClient.ToSharedRef();
}

void SCustomSocketEditorWidget::OnSocketSelectionChanged()
{
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
