// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SeatMapFactory.generated.h"

/**
 * 
 */
UCLASS()
class CUSTOMSOCKETEDITOR_API USeatMapFactory : public UFactory
{
	GENERATED_BODY()

	USeatMapFactory();
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags,
	                                  UObject* Context, FFeedbackContext* Warn) override;
	virtual bool ShouldShowInNewMenu() const override { return true; }
	virtual FText GetDisplayName() const override;
};
