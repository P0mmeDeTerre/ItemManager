// Copyright Ryckbosch Arthur 2024. All Rights Reserved.

#include "ItemParent.h"
#include "ItemManagerComponent.h"


// Sets default values
AItemParent::AItemParent()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal Mesh"));
	RootComponent = SkeletalMesh;

}

void AItemParent::UseItem(UItemManagerComponent* ItemManagerComponent)
{
	if(ItemManagerComponent->GetCurrentItem().Item.GetDefaultObject()->StaticClass() == this->StaticClass())
	{
		if(bCanBeUsed)
		{
			UE_LOG(ItemManager, Display, TEXT("Using item"));
			OnItemUsed_BP();
			OnItemUsed();
		}
		else
		{
			UE_LOG(ItemManager, Warning, TEXT("Cannot use item"));
			CannotUseItem_BP();
			CannotUseItem();
		}

	}
	else
	{
		UE_LOG(ItemManager, Error, TEXT("Cannot use Item(%s)"), *ItemInfos.FriendlyName);
	}
}



void AItemParent::OnItemUsed()
{
}

void AItemParent::CannotUseItem()
{
}

// Called when the game starts or when spawned
void AItemParent::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AItemParent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
