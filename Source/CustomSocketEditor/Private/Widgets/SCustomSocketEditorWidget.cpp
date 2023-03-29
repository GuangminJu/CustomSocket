// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/SCustomSocketEditorWidget.h"

#include "CustomSocketEditor.h"
#include "ISocketManager.h"
#include "LevelEditor.h"
#include "SCustomSocketManager.h"
#include "SlateOptMacros.h"
#include "StaticMeshEditorModule.h"
#include "PropertyCustomizationHelpers.h"

#define LOCTEXT_NAMESPACE "SocketEditor"

const FName CustomSocketEditorAppIdentifier = FName(TEXT("CustomSocketEditorApp"));

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

FStaticMeshSocketEditor::FStaticMeshSocketEditor(const FLinearColor& InWorldCentricTabColorScale,
                                                 const TWeakObjectPtr<UStaticMesh>& InStaticMesh,
                                                 const TWeakObjectPtr<UStaticMeshComponent>& InStaticMeshComponent,
                                                 const TArray<TWeakObjectPtr<UStaticMeshSocket>>& InSelectedSockets,
                                                 const bool InMutlipleSelect,
                                                 const EViewModeIndex ViewMode):
	WorldCentricTabColorScale(InWorldCentricTabColorScale),
	StaticMesh(InStaticMesh),
	StaticMeshComponent(InStaticMeshComponent),
	SelectedSockets(InSelectedSockets),
	MutlipleSelect(InMutlipleSelect),
	ViewMode(ViewMode)
{
	for (TObjectIterator<UStaticMesh> It; It; ++It)
	{
		StaticMesh = *It;
		break;
	}
}

void FStaticMeshSocketEditor::InitSocketEditor()
{
	const TSharedRef<FTabManager::FLayout> DefaultLayout = FTabManager::NewLayout("Custom Socket")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Horizontal)
			->Split
			(
				FTabManager::NewStack()
				->AddTab(CustomSocketEditorViewportTabId, ETabState::OpenedTab)
				->AddTab(CustomSocketEditorStaticMeshPickerTabId, ETabState::OpenedTab)
			)
		);

	FAssetEditorToolkit::InitAssetEditor(EToolkitMode::Standalone, nullptr,
	                                     CustomSocketEditorAppIdentifier,
	                                     DefaultLayout, true, true,
	                                     StaticMesh.Get());
}

void FStaticMeshSocketEditor::SetStaticMesh(UStaticMesh* InStaticMesh)
{
	StaticMesh = InStaticMesh;
	StaticMeshComponent->SetStaticMesh(StaticMesh.Get());
	OnStaticMeshChanged.Broadcast(StaticMesh.Get());
}

UStaticMesh* FStaticMeshSocketEditor::GetStaticMesh() const
{
	return StaticMesh.Get();
}

const FName FStaticMeshSocketEditor::CustomSocketEditorViewportTabId(
	TEXT("CustomSocketEditor_CustomSocketEditorViewport"));
const FName FStaticMeshSocketEditor::CustomSocketEditorStaticMeshPickerTabId(
	TEXT("CustomSocketEditor_CustomSocketEditorStaticMeshPickerTabId"));

void FStaticMeshSocketEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(
		LOCTEXT("WorkspaceMenu_CustomSocketEditor", "Custom Socket Editor"));
	const TSharedRef<FWorkspaceItem> WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(CustomSocketEditorViewportTabId,
	                                 FOnSpawnTab::CreateSP(
		                                 this, &FStaticMeshSocketEditor::SpawnTab_CustomSocketEditorViewport));

	InTabManager->RegisterTabSpawner(CustomSocketEditorStaticMeshPickerTabId,
	                                 FOnSpawnTab::CreateSP(
		                                 this, &FStaticMeshSocketEditor::SpawnTab_CustomSocketEditorStaticMeshPicker));
}

TSharedRef<SDockTab> FStaticMeshSocketEditor::SpawnTab_CustomSocketEditorViewport(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SCustomSocketEditorWidget).StaticMesh(StaticMesh.Get()).SeatMap(SeatMap).StaticMeshSocketEditor(
				SharedThis(this))
		];
}

TSharedRef<SDockTab> FStaticMeshSocketEditor::SpawnTab_CustomSocketEditorStaticMeshPicker(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SCustomSocketManager).SeatMap(SeatMap).StaticMeshSocketEditor(SharedThis(this))
		];
}

FStaticMeshSocketEditor::FStaticMeshSocketEditor(USeatMap* InSeatMap, UWorld* InWorld):
	StaticMesh(nullptr),
	StaticMeshComponent(nullptr),
	SelectedSockets(),
	MutlipleSelect(false),
	ViewMode(EViewModeIndex::VMI_Lit),
	World(InWorld),
	SeatMap(InSeatMap)
{
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
	StaticMesh = InArgs._StaticMesh;
	SeatMap = InArgs._SeatMap;
	StaticMeshSocketEditor = InArgs._StaticMeshSocketEditor;

	PreviewScene = MakeShareable(new FAdvancedPreviewScene(FPreviewScene::ConstructionValues()));
	StaticMeshComponent = NewObject<UStaticMeshComponent>(GetTransientPackage(), NAME_None, RF_Transient);
	StaticMeshComponent->SetStaticMesh(StaticMesh);

	PreviewScene->AddComponent(StaticMeshComponent, FTransform::Identity);

	SEditorViewport::Construct(SEditorViewport::FArguments());

	ViewportOverlay->AddSlot()
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.VAlign(VAlign_Top)
		[
			SNew(SObjectPropertyEntryBox).AllowedClass(UStaticMesh::StaticClass()).ObjectPath_Lambda([this]()
			                             {
				                             return StaticMesh ? StaticMesh->GetPathName() : FString();
			                             })
			                             .OnObjectChanged_Lambda([this](const FAssetData& AssetData)
			                             {
				                             SetStaticMesh(Cast<UStaticMesh>(AssetData.GetAsset()));
			                             })
		]
	];

	FCoreUObjectDelegates::OnObjectPropertyChanged.AddRaw(this, &SCustomSocketEditorWidget::OnObjectPropertyChanged);
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
	Collector.AddReferencedObject(SeatMap);
	Collector.AddReferencedObjects(SeatPreviewComponents);
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

void SCustomSocketEditorWidget::SetStaticMesh(UStaticMesh* InStaticMesh)
{
	StaticMesh = InStaticMesh;
	StaticMeshComponent->SetStaticMesh(StaticMesh);
	StaticMeshSocketEditor->SetStaticMesh(StaticMesh);
	RebuildSeatPreviewComponents(SeatMap);
}

void SCustomSocketEditorWidget::OnSocketSelectionChanged()
{
}

void SCustomSocketEditorWidget::RebuildSeatPreviewComponents(UObject* Object)
{
	if (Object != SeatMap)
		return;

	for (USeatPreviewComponent* SeatPreviewComponent : SeatPreviewComponents)
	{
		SeatPreviewComponent->DestroyComponent();
	}
	SeatPreviewComponents.Empty();

	const FSeats& Seats = SeatMap->GetSeats(StaticMesh);
	int32 SeatsCount = Seats.Seats.Num();

	for (USeatSocket* SeatSocket : Seats.Seats)
	{
		USeatPreviewComponent* SeatPreviewComponent = NewObject<USeatPreviewComponent>(GetTransientPackage());
		SeatPreviewComponent->SetSeatSocket(SeatSocket);
		SeatPreviewComponents.Add(SeatPreviewComponent);
	}

	for (const USeatPreviewComponent* SeatPreviewComponent : SeatPreviewComponents)
	{
		PreviewScene->AddComponent(SeatPreviewComponent->GetPreviewComponent(), {
			                           SeatPreviewComponent->SeatSocket->RelativeRotation,
			                           SeatPreviewComponent->SeatSocket->RelativeLocation, FVector::OneVector
		                           });
	}
}

void SCustomSocketEditorWidget::OnObjectPropertyChanged(UObject* Object, FPropertyChangedEvent& PropertyChangedEvent)
{
	RebuildSeatPreviewComponents(Object);
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
