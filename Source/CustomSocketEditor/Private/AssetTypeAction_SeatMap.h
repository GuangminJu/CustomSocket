
#pragma once
#include "AssetTypeActions_Base.h"
#include "SeatSocket/SeatSocket.h"

class FAssetTypeActions_SeatMap : public FAssetTypeActions_Base
{
public:
	// IAssetTypeActions Implementation
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_SeatMap", "SeatMap"); }
	virtual FColor GetTypeColor() const override { return FColor(0,232,0); }
	virtual UClass* GetSupportedClass() const override { return USeatMap::StaticClass(); }
	virtual uint32 GetCategories() override;
	virtual FString GetObjectDisplayName(UObject* Object) const override { return CastChecked<USeatMap>(Object)->GetName(); }
	virtual void OpenAssetEditor( const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>() ) override;
};
