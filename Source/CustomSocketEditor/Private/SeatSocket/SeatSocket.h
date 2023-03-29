// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshSocket.h"
#include "UObject/Object.h"
#include "SeatSocket.generated.h"

/**
 * 
 */

UENUM()
enum class EPosture : uint8
{
	StandUp,
	SquatDown,
	GetDown
};

UENUM()
enum class ESeatType : uint8
{
	Normal,
	Fireable,
};

UCLASS()
class CUSTOMSOCKETEDITOR_API USeatSocket : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SeatSocket")
	FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SeatSocket")
	FVector RelativeLocation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SeatSocket")
	FRotator RelativeRotation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SeatSocket")
	ESeatType SeatType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SeatSocket")
	EPosture Posture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SeatSocket")
	float YawScope;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SeatSocket")
	float PitchScope;

public:
#if WITH_EDITOR
	/** Broadcasts a notification whenever the socket property has changed. */
	DECLARE_EVENT_TwoParams(USeatSocket, FChangedEvent, const class USeatSocket*, const class FProperty*);

	FChangedEvent& OnPropertyChanged() { return ChangedEvent; }

	//~ Begin UObject Interface
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	//~ End UObject Interface

private:
	/** Broadcasts a notification whenever the socket property has changed. */
	FChangedEvent ChangedEvent;
#endif // WITH_EDITOR
};

USTRUCT()
struct FSeats
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<USeatSocket*> Seats;
};

UCLASS()
class USeatMap : public UObject, public IInterface_AssetUserData
{
	GENERATED_BODY()
public:
	void AddSeat(UStaticMesh* InStaticMesh, USeatSocket* InSeatSocket);
	void RemoveSeat(UStaticMesh* InStaticMesh, USeatSocket* InSeatSocket);
	FSeats& GetSeats(UStaticMesh* InStaticMesh);

	virtual void AddAssetUserData(UAssetUserData* InUserData) override;
	virtual UAssetUserData* GetAssetUserDataOfClass(TSubclassOf<UAssetUserData> InUserDataClass) override;
	virtual const TArray<UAssetUserData*>* GetAssetUserDataArray() const override;
	virtual void RemoveUserDataOfClass(TSubclassOf<UAssetUserData> InUserDataClass) override;

	UPROPERTY()
	TMap<UStaticMesh*, FSeats> SeatMap;

	/** Array of user data stored with the asset */
	UPROPERTY()
	TArray<UAssetUserData*> AssetUserData;
};
