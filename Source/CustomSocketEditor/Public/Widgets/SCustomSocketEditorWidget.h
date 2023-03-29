// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AdvancedPreviewScene.h"
#include "IStaticMeshEditor.h"
#include "SAssetEditorViewport.h"
#include "SCommonEditorViewportToolbarBase.h"
#include "SeatPreviewComponent.h"

class FStaticMeshSocketEditor;
class ICustomSocketToolkitHost;
class USeatMap;

class CUSTOMSOCKETEDITOR_API SCustomSocketEditorWidget : public SAssetEditorViewport, public FGCObject,
                                                         public ICommonEditorViewportToolbarInfoProvider
{
public:
	SLATE_BEGIN_ARGS(SCustomSocketEditorWidget)
		{
		}

		SLATE_ARGUMENT(UStaticMesh*, StaticMesh)
		SLATE_ARGUMENT(USeatMap*, SeatMap)
		SLATE_ARGUMENT(TSharedPtr<FStaticMeshSocketEditor>, StaticMeshSocketEditor)
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

	void SetStaticMesh(UStaticMesh* InStaticMesh);
	void OnSocketSelectionChanged();
	void RebuildSeatPreviewComponents(UObject* Object);
	void OnObjectPropertyChanged(UObject* Object, FPropertyChangedEvent& PropertyChangedEvent);
private:
	TSharedPtr<FEditorViewportClient> EditorViewportClient;
	TSharedPtr<FAdvancedPreviewScene> PreviewScene;
	UStaticMesh* StaticMesh;
	EViewModeIndex CurrentViewMode;
	int32 LODSelection;
	UStaticMeshComponent* StaticMeshComponent;
	TSharedPtr<FStaticMeshSocketEditor> StaticMeshSocketEditor;
	USeatMap* SeatMap = nullptr;
	TArray<USeatPreviewComponent*> SeatPreviewComponents;
};

class USeatMap;

DECLARE_MULTICAST_DELEGATE_OneParam(FStaticMeshChanged, UStaticMesh*);

class FStaticMeshSocketEditor : public FAssetEditorToolkit
{
public:
	FStaticMeshSocketEditor(const FLinearColor& InWorldCentricTabColorScale,
	                        const TWeakObjectPtr<UStaticMesh>& InStaticMesh,
	                        const TWeakObjectPtr<UStaticMeshComponent>& InStaticMeshComponent,
	                        const TArray<TWeakObjectPtr<UStaticMeshSocket>>& InSelectedSockets,
	                        const bool InMutlipleSelect,
	                        const EViewModeIndex ViewMode);

	FStaticMeshSocketEditor(USeatMap* InSeatMap, UWorld* InWorld);

	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	void InitSocketEditor();
	void SetStaticMesh(UStaticMesh* InStaticMesh);
	UStaticMesh* GetStaticMesh() const;

	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;

	TSharedRef<SDockTab> SpawnTab_CustomSocketEditorViewport(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_CustomSocketEditorStaticMeshPicker(const FSpawnTabArgs& Args);

	static const FName CustomSocketEditorViewportTabId;
	static const FName CustomSocketEditorStaticMeshPickerTabId;

	FStaticMeshChanged OnStaticMeshChanged;
private:
	FLinearColor WorldCentricTabColorScale;
	TWeakObjectPtr<UStaticMesh> StaticMesh;
	TWeakObjectPtr<UStaticMeshComponent> StaticMeshComponent;
	TArray<TWeakObjectPtr<UStaticMeshSocket>> SelectedSockets;
	bool MutlipleSelect = false;
	EViewModeIndex ViewMode = VMI_Lit;
	UWorld* World = nullptr;
	USeatMap* SeatMap = nullptr;
	TSharedPtr<IStaticMeshEditor> StaticMeshEditor;
};
