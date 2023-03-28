#include "AssetTypeAction_SeatMap.h"

#include "CustomSocketEditor.h"
#include "Widgets/SCustomSocketEditorWidget.h"

uint32 FAssetTypeActions_SeatMap::GetCategories()
{
	return FCustomSocketEditorModule::BYCAssetCategoryBit;
}

void FAssetTypeActions_SeatMap::OpenAssetEditor(const TArray<UObject*>& InObjects,
                                                TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	for (UObject* Object : InObjects)
	{
		const TSharedPtr<FStaticMeshSocketEditor> SocketEditor = MakeShared<FStaticMeshSocketEditor>(
			Cast<USeatMap>(Object), GEngine->GetWorld());
		SocketEditor->InitSocketEditor();
	}
}
