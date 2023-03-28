// Fill out your copyright notice in the Description page of Project Settings.


#include "SeatMapFactory.h"

#include "SeatSocket/SeatSocket.h"

#define LOCTEXT_NAMESPACE "SeatMapFactory"

USeatMapFactory::USeatMapFactory()
{
	SupportedClass = USeatMap::StaticClass();
	bCreateNew = true;

}

UObject* USeatMapFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags,
                                           UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<USeatMap>(InParent, Class, Name, Flags);
}

FText USeatMapFactory::GetDisplayName() const
{
	return LOCTEXT("SeatMapFactoryDescription", "Seat Map");
}

#undef LOCTEXT_NAMESPACE
