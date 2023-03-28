// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/SCustomSocketEditorWidget.h"

#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SCustomSocketEditorWidget::Construct(const FArguments& InArgs)
{
	StaticMeshEditorPtr = InArgs._StaticMeshEditor;

	TSharedPtr<IStaticMeshEditor> PinnedEditor = StaticMeshEditorPtr.Pin();
	StaticMesh = PinnedEditor.IsValid() ? PinnedEditor->GetStaticMesh() : nullptr;

	SEditorViewport::Construct(SEditorViewport::FArguments());

	/*
	ChildSlot
	[
		// Populate the widget
	];
	*/
}

SCustomSocketEditorWidget::SCustomSocketEditorWidget()
	: PreviewScene(MakeShareable(new FAdvancedPreviewScene(FPreviewScene::ConstructionValues()))), StaticMesh(nullptr),
	  CurrentViewMode(VMI_Lit),
	  LODSelection(0)
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

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
