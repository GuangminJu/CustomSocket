// Fill out your copyright notice in the Description page of Project Settings.


#include "SeatPreviewComponent.h"

#include "SeatSettings.h"
#include "Animation/AnimInstance.h"


// Sets default values for this component's properties
USeatPreviewComponent::USeatPreviewComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PreviewSkeletalMesh"), true);
	SkeletalMeshComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	SkeletalMeshComponent->SetRelativeTransform(FTransform::Identity);
	SkeletalMeshComponent->SetSkeletalMesh(GetDefault<USeatSettings>()->PreviewSkeletalMesh.LoadSynchronous());

	FCoreUObjectDelegates::OnObjectPropertyChanged.AddUObject(this, &USeatPreviewComponent::OnObjectPropertyChanged);

	UClass* PreviewAnimBlueprintClass = GetDefault<USeatSettings>()->PreviewAnimBlueprint.LoadSynchronous();
	SkeletalMeshComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	SkeletalMeshComponent->SetAnimInstanceClass(PreviewAnimBlueprintClass);
}


// Called when the game starts
void USeatPreviewComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}

void USeatPreviewComponent::Update()
{
	SetRelativeLocation(SeatSocket->RelativeLocation);
	SetRelativeRotation(SeatSocket->RelativeRotation);
}

void USeatPreviewComponent::SetSeatSocket(USeatSocket* InSeatSocket)
{
	SeatSocket = InSeatSocket;
	if (SeatSocket)
		Update();
}

void USeatPreviewComponent::OnObjectPropertyChanged(UObject* Object, FPropertyChangedEvent& PropertyChangedEvent)
{
	if (Object != SeatSocket)
		return;

	Update();
}

// Called every frame
void USeatPreviewComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

USceneComponent* USeatPreviewComponent::GetPreviewComponent() const
{
	return SkeletalMeshComponent;
}
