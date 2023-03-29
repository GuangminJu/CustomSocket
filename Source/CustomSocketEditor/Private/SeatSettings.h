// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SeatSettings.generated.h"

/**
 * 
 */
UCLASS(Config = Game)
class CUSTOMSOCKETEDITOR_API USeatSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Config)
	TSoftObjectPtr<USkeletalMesh> PreviewSkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Config)
	TSoftClassPtr<UAnimInstance> PreviewAnimBlueprint;
};
