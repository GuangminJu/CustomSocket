// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AdvancedPreviewScene.h"
#include "IStaticMeshEditor.h"
#include "SAssetEditorViewport.h"
#include "SCommonEditorViewportToolbarBase.h"

class FStaticMeshSocketEditor;
class ICustomSocketToolkitHost;

class CUSTOMSOCKETEDITOR_API SCustomSocketEditorWidget : public SAssetEditorViewport, public FGCObject, public ICommonEditorViewportToolbarInfoProvider
{
public:
	SLATE_BEGIN_ARGS(SCustomSocketEditorWidget) {}
		SLATE_ARGUMENT(TWeakPtr<IStaticMeshEditor>, StaticMeshEditor)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	SCustomSocketEditorWidget();
	virtual ~SCustomSocketEditorWidget() override;
	
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual TSharedRef<SEditorViewport> GetViewportWidget() override;
	virtual TSharedPtr<FExtender> GetExtenders() const override;
	virtual void OnFloatingButtonClicked() override;
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;

	void OnSocketSelectionChanged();
private:
	TSharedPtr<FEditorViewportClient> EditorViewportClient;
	TSharedPtr<FAdvancedPreviewScene> PreviewScene;
	UStaticMesh* StaticMesh;
	EViewModeIndex CurrentViewMode;
	int32 LODSelection;
	UStaticMeshComponent* StaticMeshComponent;
	TSharedPtr<FStaticMeshSocketEditor> SocketEditor;
};

class FStaticMeshSocketEditor : public FAssetEditorToolkit
{
public:
	FStaticMeshSocketEditor(const FLinearColor& WorldCentricTabColorScale,
							const TWeakObjectPtr<UStaticMesh>& StaticMesh,
							const TWeakObjectPtr<UStaticMeshComponent>& StaticMeshComponent,
							const TArray<TWeakObjectPtr<UStaticMeshSocket>>& SelectedSockets,
							const bool InMutlipleSelect,
							const EViewModeIndex ViewMode);

	FStaticMeshSocketEditor(UWorld* InWorld);

	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	void InitSocketEditor();

	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;

	TSharedRef<SDockTab> SpawnTab_CustomSocketEditor(const FSpawnTabArgs& Args);

	static const FName CustomSocketEditorTabId;
private:
	FLinearColor WorldCentricTabColorScale;
	TWeakObjectPtr<UStaticMesh> StaticMesh;
	TWeakObjectPtr<UStaticMeshComponent> StaticMeshComponent;
	TArray<TWeakObjectPtr<UStaticMeshSocket>> SelectedSockets;
	bool MutlipleSelect = false;
	EViewModeIndex ViewMode = VMI_Lit;
	UWorld* World = nullptr;
	TSharedPtr<ICustomSocketToolkitHost, ESPMode::NotThreadSafe> CustomSocketToolkitHost;
};
