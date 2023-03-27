// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AdvancedPreviewScene.h"
#include "IStaticMeshEditor.h"
#include "SAssetEditorViewport.h"
#include "SCommonEditorViewportToolbarBase.h"

class FStaticMeshEditorViewportClient;

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
private:
	TSharedPtr<FEditorViewportClient> EditorViewportClient;
	TSharedPtr<FAdvancedPreviewScene> PreviewScene;
	TWeakPtr<IStaticMeshEditor> StaticMeshEditorPtr;
	UStaticMesh* StaticMesh;
	EViewModeIndex CurrentViewMode;
	int32 LODSelection;

};
