// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SeatSocket/SeatSocket.h"
#include "SeatPreviewComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CUSTOMSOCKETEDITOR_API USeatPreviewComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	USeatPreviewComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	void OnObjectPropertyChanged(UObject* Object, FPropertyChangedEvent& PropertyChangedEvent);
public:
	virtual void Update();
	virtual void SetSeatSocket(USeatSocket* InSeatSocket);

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY()
	USkeletalMeshComponent* SkeletalMeshComponent;
	
	USceneComponent* GetPreviewComponent() const;
	
	UPROPERTY()
	USeatSocket* SeatSocket;
};
