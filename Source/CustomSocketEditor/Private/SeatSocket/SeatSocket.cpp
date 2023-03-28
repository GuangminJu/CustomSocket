// Fill out your copyright notice in the Description page of Project Settings.


#include "SeatSocket.h"

#if WITH_EDITOR
void USeatSocket::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property)
	{
		ChangedEvent.Broadcast(this, PropertyChangedEvent.MemberProperty);
	}
}

void USeatMap::AddSeat(UStaticMesh* InStaticMesh, USeatSocket* InSeatSocket)
{
	GetSeats(InStaticMesh).Seats.Add(InSeatSocket);
}

void USeatMap::RemoveSeat(UStaticMesh* InStaticMesh, USeatSocket* InSeatSocket)
{
	GetSeats(InStaticMesh).Seats.Remove(InSeatSocket);
}

FSeats& USeatMap::GetSeats(UStaticMesh* InStaticMesh)
{
	FSeats* Seats = SeatMap.Find(InStaticMesh);
	if (!Seats)
		SeatMap.Add(InStaticMesh, FSeats());

	return SeatMap[InStaticMesh];
}

void USeatMap::AddAssetUserData(UAssetUserData* InUserData)
{
	if (InUserData != nullptr)
	{
		UAssetUserData* ExistingData = GetAssetUserDataOfClass(InUserData->GetClass());
		if (ExistingData != nullptr)
		{
			AssetUserData.Remove(ExistingData);
		}
		AssetUserData.Add(InUserData);
	}
}

UAssetUserData* USeatMap::GetAssetUserDataOfClass(TSubclassOf<UAssetUserData> InUserDataClass)
{
	for (int32 DataIdx = 0; DataIdx < AssetUserData.Num(); DataIdx++)
	{
		UAssetUserData* Datum = AssetUserData[DataIdx];
		if (Datum != nullptr && Datum->IsA(InUserDataClass))
		{
			return Datum;
		}
	}
	return nullptr;
}

const TArray<UAssetUserData*>* USeatMap::GetAssetUserDataArray() const
{
	return &AssetUserData;
}

void USeatMap::RemoveUserDataOfClass(TSubclassOf<UAssetUserData> InUserDataClass)
{
	for (int32 DataIdx = 0; DataIdx < AssetUserData.Num(); DataIdx++)
	{
		UAssetUserData* Datum = AssetUserData[DataIdx];
		if (Datum != nullptr && Datum->IsA(InUserDataClass))
		{
			AssetUserData.RemoveAt(DataIdx);
			return;
		}
	}
}
#endif
