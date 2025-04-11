// Copyright Ryckbosch Arthur 2024. All Rights Reserved.


#include "ItemCollectable.h"

#include "ItemManagerComponent.h"

// Sets default values
AItemCollectable::AItemCollectable()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	FString OutlineMaterialPath = TEXT("/ItemManager/Materials/M_Outline_Inst.M_Outline_Inst");

	UMaterialInstance* MaterialInstance = Cast<UMaterialInstance>(StaticLoadObject(UMaterialInstance::StaticClass(), nullptr, *OutlineMaterialPath));
	if (MaterialInstance)
	{
		OutlineMaterial = MaterialInstance;
	}

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene Root"));
	RootComponent = SceneComponent;

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal Mesh"));
	SkeletalMesh->SetupAttachment(SceneComponent);

	TriggerBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger Component"));
	TriggerBoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBoxComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBoxComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBoxComponent->SetupAttachment(SkeletalMesh);

}

// Called when the game starts or when spawned
void AItemCollectable::BeginPlay()
{
	Super::BeginPlay();	

	TriggerBoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AItemCollectable::OnTriggerBeginOverlap);
	TriggerBoxComponent->OnComponentEndOverlap.AddDynamic(this, &AItemCollectable::OnTriggerEndOverlap);

	if(!OutlineMaterial)
	{
		UE_LOG(ItemManager, Error, TEXT("Cannot load OutlineMaterial"));
	}

	if (bEnableCollisions && !SkeletalMesh || !SkeletalMesh->GetPhysicsAsset())
	{
		UE_LOG(ItemManager, Error, TEXT("Item (%s) has no physics asset. Collisions and physics may not work."), *(Item ? Item->GetName() : "Unknown"));
	}
}

void AItemCollectable::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	TriggerBoxComponent->SetBoxExtent(Size);
	SetupMesh();

	if (bCacheInvertGroundRotation != GroundTypeProperties.bInvertGroundRotation)
	{
		GroundTypeProperties.AdjustedRotator = {0.f, 0.f, 0.f};
		bCacheInvertGroundRotation = GroundTypeProperties.bInvertGroundRotation;
	}

	//	Place mesh to the ground
	if(ItemDisplay == EItemDisplay::ID_Grounded)
	{
		PlaceMeshToTheGround();
	}
	else if(ItemDisplay == EItemDisplay::ID_Physics && SkeletalMesh)
	{
		bEnableCollisions = true;
		SkeletalMesh->SetSimulatePhysics(true);
	}

	if (bEnableCollisions)
	{
		EnableCollisions();
	}
	else
	{
		DisableCollisions();
	}
}

void AItemCollectable::SetTransparency(bool Value)
{
	SkeletalMesh->SetRenderCustomDepth(Value);
}

void AItemCollectable::EnableTransparency()
{
	bEnableTransparency = true;
	SkeletalMesh->SetRenderCustomDepth(true);
}

void AItemCollectable::DisableTransparency()
{
	bEnableTransparency = false;
	SkeletalMesh->SetRenderCustomDepth(false);
}

void AItemCollectable::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UItemManagerComponent* ItemManagerComponent = Cast<UItemManagerComponent>(OtherActor->GetComponentByClass(UItemManagerComponent::StaticClass()));
	SetItemInstance();

	if(ItemManagerComponent && ItemInstance && ItemInstance->CanBeCollected())
	{

		// Add outline material
		if (bEnableOutline && SkeletalMesh && OutlineMaterial)
		{
			UMaterialInterface* OutlineMaterialInterface = Cast<UMaterialInterface>(OutlineMaterial);
			SkeletalMesh->SetOverlayMaterial(OutlineMaterialInterface);
		}

		SetTransparency(false);

		ItemManagerComponent->OnPickableBeginOverlap(this);
	}


}

void AItemCollectable::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UItemManagerComponent* ItemManagerComponent = Cast<UItemManagerComponent>(OtherActor->GetComponentByClass(UItemManagerComponent::StaticClass()));

	if(ItemManagerComponent)
	{
		ItemManagerComponent->OnPickableEndOverlap(this);
	}

	if (SkeletalMesh)
	{
		SkeletalMesh->SetOverlayMaterial(nullptr);
	}

	SetTransparency(bEnableTransparency);
}

void AItemCollectable::PlaceMeshToTheGround()
{

	FVector ActorLocation = GetActorLocation();
	FVector Start = FVector(ActorLocation.X, ActorLocation.Y, ActorLocation.Z + GroundTypeProperties.MaxHeight);
	FVector End = FVector(ActorLocation.X, ActorLocation.Y, ActorLocation.Z - GroundTypeProperties.MaxHeight);
	FRotator NewRotation;
	FHitResult HitResult;
	FCollisionQueryParams CollisionQueryParams;

	CollisionQueryParams.AddIgnoredActor(this);
	
	if(GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, CollisionQueryParams) && HitResult.GetActor() && Cast<UItemManagerComponent>(HitResult.GetActor()->GetComponentByClass(UItemManagerComponent::StaticClass())))
	{
		CollisionQueryParams.AddIgnoredActor(HitResult.GetActor());
	}

	// Get Z location
	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, CollisionQueryParams))
	{
		//DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 2.0f, 0, 1.0f);

		FVector AdjustedLocation = HitResult.ImpactPoint - HitResult.ImpactNormal;
		
		SetActorLocation(AdjustedLocation);

		if(GroundTypeProperties.UseItemWidthInstead)
		{
			SkeletalMesh->AddRelativeLocation({ 0.f, 0.f, MeshWidth / 2 });
		}
		else
		{
			SkeletalMesh->AddRelativeLocation({ 0.f, 0.f, MeshHeight });
		}
		
		if(GroundTypeProperties.GroundRotationType == EGroundedType::GT_KeepGroundRotation)
		{
			NewRotation = HitResult.ImpactNormal.Rotation();
		}
		else
		{
			SkeletalMesh->SetWorldRotation({0, 0, 0});
		}

		if (GroundTypeProperties.bInvertGroundRotation)
		{
			NewRotation += {180.f, 0.f, 0.f };
		}

		NewRotation += GroundTypeProperties.AdjustedRotator;
		
		SkeletalMesh->SetWorldRotation(NewRotation);


		AttachToActor(HitResult.GetActor(), FAttachmentTransformRules::KeepWorldTransform, "None");
	}
}

void AItemCollectable::SetItemInstance()
{
	if (Item)
	{
		ItemClass = Item.Get();

		if (ItemClass)
		{
			ItemInstance = NewObject<AItemParent>(this, ItemClass);
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("Could not get class"));
		}
	}
}

void AItemCollectable::SetupMesh()
{

	SetItemInstance();

	if (ItemInstance && ItemInstance->GetSkeletalMesh() && ItemInstance->GetSkeletalMesh())
	{
		SkeletalMesh->SetSkeletalMeshAsset(ItemInstance->GetSkeletalMesh()->GetSkeletalMeshAsset());

		if (SkeletalMesh->GetSkeletalMeshAsset())
		{
			MeshHeight = SkeletalMesh->GetSkeletalMeshAsset()->GetBounds().BoxExtent.X / 2;
			MeshWidth = SkeletalMesh->GetSkeletalMeshAsset()->GetBounds().BoxExtent.Z / 2;
			SetTransparency(bEnableTransparency);
			SkeletalMesh->SetRelativeTransform({});
			SkeletalMesh->SetSimulatePhysics(false);
			DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		}
	}
}

void AItemCollectable::AnimatedMesh(float DeltaTime)
{
	FVector NewLocation = SkeletalMesh->GetRelativeLocation();
	FRotator NewRotation = SkeletalMesh->GetRelativeRotation();
	float DeltaHeight = FMath::Sin(AnimatedRunningTime * AnimatedItemProperties.HeightSpeed) * (AnimatedItemProperties.Height);


	NewLocation.Z = DeltaHeight;
	SkeletalMesh->SetRelativeLocation(NewLocation);

	NewRotation.Yaw += DeltaTime * AnimatedItemProperties.RotationSpeed;
	SkeletalMesh->SetRelativeRotation(NewRotation);

	AnimatedRunningTime += DeltaTime;	

	if (AnimatedRunningTime > 2 * PI / AnimatedItemProperties.HeightSpeed)
	{
		AnimatedRunningTime = 0;
	}
}

void AItemCollectable::Init(FItemCollectableData ItemCollectableData)
{
	Item = ItemCollectableData.Item;
	Size = ItemCollectableData.Size;
	ItemDisplay = ItemCollectableData.ItemDisplay;
	GroundTypeProperties = ItemCollectableData.GroundTypeProperties;
	AnimatedItemProperties = ItemCollectableData.AnimatedItemProperties;
	bEnableCollisions = ItemCollectableData.bEnableCollisions;
	bEnableTransparency = ItemCollectableData.bEnableTransparency;
	bEnableOutline = ItemCollectableData.bEnableOutline;
	OutlineMaterial = ItemCollectableData.OutlineMaterial;
}

void AItemCollectable::EnableCollisions()
{
	SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SkeletalMesh->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
	SkeletalMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
}

void AItemCollectable::DisableCollisions()
{
	SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SkeletalMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
}

void AItemCollectable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ItemDisplay == EItemDisplay::ID_Animated)
	{
		AnimatedMesh(DeltaTime);
	}
}