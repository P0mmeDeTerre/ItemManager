// Copyright Ryckbosch Arthur 2024. All Rights Reserved.


#include "ItemManagerComponent.h"

DEFINE_LOG_CATEGORY(ItemManager);

void UItemManagerComponent::SwitchItem(int newItemIndex)
{
    float Delay;
    float OldItemDespawnDelay;
    float NewItemSpawnDelay;

    if (newItemIndex < 0 || newItemIndex >= Items.Num())
    {
        UE_LOG(ItemManager, Warning, TEXT("Invalid item index at %d"), newItemIndex);
        OnFailedtoSwitchItem.Broadcast(1);
        return;
    }

    if (CurrentItemIndex == newItemIndex && ItemState != EItemState::IS_None)
    { 
        UE_LOG(ItemManager, Warning, TEXT("Attempt to switch to the same item"));
        OnFailedtoSwitchItem.Broadcast(2);
        return; 
    }

    if (bIsSwitchingItem)
    {
        UE_LOG(ItemManager, Warning, TEXT("Item is switching, cannot switch at the moment."));
        OnFailedtoSwitchItem.Broadcast(3);
        return; 
    }

    bIsSwitchingItem = true;
    OldItemDespawnDelay = Items[CurrentItemIndex].ItemInfos.TimeBeforeDespawn;
    NewItemSpawnDelay = Items[newItemIndex].ItemInfos.TimeBeforeSpawn + OldItemDespawnDelay;
    Delay = OldItemDespawnDelay > NewItemSpawnDelay ? OldItemDespawnDelay : NewItemSpawnDelay;

    ActivateSwitching(NewItemSpawnDelay);

    
    // destroy current item
    DestroyItemLambda(OldItemDespawnDelay);
    
    CurrentItemIndex = newItemIndex;
    
    ItemState = EItemState::IS_Switching;


    SpawnItemLambda(NewItemSpawnDelay);


	UE_LOG(ItemManager, Display, TEXT("New item index is %d"), newItemIndex);

	OnitemSwitchedDelegate.Broadcast(Items[newItemIndex]);
}

FItemCollectableData UItemManagerComponent::SetItemCollectableData(AItemCollectable* ItemCollectable)
{
    FItemCollectableData ItemCollectableData;

    if (IsValid(ItemCollectable)) 
    {
        ItemCollectableData.AnimatedItemProperties = ItemCollectable->GetAnimatedItemProperties();
        ItemCollectableData.bEnableCollisions = ItemCollectable->AreCollisionsEnabled();
        ItemCollectableData.bEnableOutline = ItemCollectable->IsOutlineEnabled();
        ItemCollectableData.bEnableTransparency = ItemCollectable->IsTransparencyEnabled();
        ItemCollectableData.GroundTypeProperties = ItemCollectable->GetGroundTypeProperties();
        ItemCollectableData.Item = ItemCollectable->GetItem();
        ItemCollectableData.ItemDisplay = ItemCollectable->GetItemDisplay();
        ItemCollectableData.OutlineMaterial = ItemCollectable->GetOutlineMaterial();
        ItemCollectableData.Size = ItemCollectable->GetTriggerBoxSize();
    }

    return ItemCollectableData;
}

void UItemManagerComponent::CollectItem()
{
    if(IsValid(CurrentItemCollectable) && AddItem(CurrentItemCollectable->GetItem()) == 0)
    {
        Items[Items.Num() - 1].ItemCollectableData = SetItemCollectableData(CurrentItemCollectable);

        // Remove ItemCollectable from ItemsCollectables
        ItemsCollectable.Remove(CurrentItemCollectable);

        CurrentItemCollectable->Destroy();
        UE_LOG(ItemManager, Display, TEXT("Item has been collected"));
        OnItemCollectedDelegate.Broadcast();

        // auto switch
        if(Items[Items.Num() - 1].Item.GetDefaultObject()->EquipWhenPickedUp())
        {
            SwitchIndexItem(Items.Num() - 1);
        }


    }
    else
    {
        UE_LOG(ItemManager, Warning, TEXT("Can't collect item"));
        CannotCollectItemDelegate.Broadcast();
    }
}

void UItemManagerComponent::OnPickableBeginOverlap(AItemCollectable* ItemCollectable)
{
    // prevent other items to be picked up when boxes are inside each other
    if(CurrentItemCollectable == nullptr && IsValid(ItemCollectable))
    {
        CurrentItemCollectable = ItemCollectable;
        OnBeginOverlapDelegate.Broadcast(ItemCollectable);
    }
    
}

FAttachmentTransformRules UItemManagerComponent::EnumAttachmentRulesToStuct(EAttachmentRules AttachmentRules)
{
    FAttachmentTransformRules AttachmentRulesStruct = FAttachmentTransformRules::KeepWorldTransform;
    switch(AttachmentRules)
    {
        case EAttachmentRules::AR_KeepRelativeTransform:           AttachmentRulesStruct = FAttachmentTransformRules::KeepRelativeTransform; break;
        case EAttachmentRules::AR_KeepWorldTransform:              AttachmentRulesStruct = FAttachmentTransformRules::KeepWorldTransform; break;
        case EAttachmentRules::AR_SnapToTargetIncludingScale:      AttachmentRulesStruct = FAttachmentTransformRules::SnapToTargetIncludingScale; break;
        case EAttachmentRules::AR_SnapToTargetNotIncludingScale:   AttachmentRulesStruct = FAttachmentTransformRules::SnapToTargetNotIncludingScale; break;
    }

    return AttachmentRulesStruct;
}

void UItemManagerComponent::OnPickableEndOverlap(AItemCollectable* ItemCollectable)
{
    if(ItemCollectable && CurrentItemCollectable == ItemCollectable)
    {
        CurrentItemCollectable = nullptr;
        OnEndOverlapDelegate.Broadcast(ItemCollectable);
    }

}

AItemParent* UItemManagerComponent::GetItemInstance(TSubclassOf<AItemParent> Item)
{
    if (Item)
    {
        UClass* ItemClass = Item.Get();

        if (ItemClass)
        {
            return NewObject<AItemParent>(this, ItemClass);
        }
    }

    return nullptr;
}

void UItemManagerComponent::PickupItem(AItemParent* item)
{
    
}

void UItemManagerComponent::SpawnItemLambda(float delay)
{
    // avoid a non called lambda
    if(delay <= 0.0f)
    {
        SpawnItem();
    }
    else
    {
        GetWorld()->GetTimerManager().SetTimer(SpawnItemTimerHandle, [this]()
        {
            SpawnItem();
        }, delay, false);
    }
    

}

void UItemManagerComponent::SpawnItem()
{

    FString FriendlyName = "";

    if(Items.IsValidIndex(CurrentItemIndex))
    {
        FriendlyName = Items[CurrentItemIndex].ItemInfos.FriendlyName;
    }
  

    if (IsValid(GetOwner()) && Items.IsValidIndex(CurrentItemIndex) && !Items[CurrentItemIndex].Actor) // if the actor does not exist in world, spawn it.
    {
        Items[CurrentItemIndex].Actor = GetWorld()->SpawnActor<AItemParent>(Items[CurrentItemIndex].Item, Items[CurrentItemIndex].ItemInfos.SpawnRelativeTransform);

        if (IsValid(Items[CurrentItemIndex].Actor))    
        {
            UE_LOG(ItemManager, Display, TEXT("The Item (%s) successfully spawned"), *FriendlyName);
        }
        else
        {
            UE_LOG(ItemManager, Warning, TEXT("The Item (%s) failed to spawn"), *FriendlyName);
        }

    }
    else
    {
        UE_LOG(ItemManager, Display, TEXT("The Item (%s) alreay exist !"), *FriendlyName);
    }

    if (!IsValid(CharacterMesh))
    {
        CharacterMesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
    }

    if(CharacterMesh && Items[CurrentItemIndex].Actor)
    {
        Items[CurrentItemIndex].Actor->AttachToComponent(CharacterMesh, EnumAttachmentRulesToStuct(Items[CurrentItemIndex].ItemInfos.ItemAttachSocket.AttachementRules), Items[CurrentItemIndex].ItemInfos.ItemAttachSocket.ItemSocket);
    }

    ItemState = EItemState::IS_Idle;
    OnItemSpawnedDelegate.Broadcast(Items[CurrentItemIndex]);
}

void UItemManagerComponent::DestroyItemLambda(float delay)
{
    int OldItemIndex = CurrentItemIndex;

    // avoid a non called lambda
    if(delay <= 0.0f)
    {
        DestroyItem(OldItemIndex);
    }
    else
    {
        GetWorld()->GetTimerManager().SetTimer(DestroyItemTimerHandle, [this, OldItemIndex]()
        {
            DestroyItem(OldItemIndex);
        }, delay, false);
    }


}

void UItemManagerComponent::DestroyItem(int OldItemIndex)
{
    if (IsValid(Items[OldItemIndex].Actor))
    {
        if(Items[OldItemIndex].Actor->IsItemDespawnWhenSwitched())
        {
            OnItemDespawnedDelegate.Broadcast(Items[OldItemIndex]);
            
            FString const ItemName = Items[OldItemIndex].Actor->GetItemInfos().FriendlyName;
            Items[OldItemIndex].Actor->Destroy();
            Items[OldItemIndex].Actor = nullptr;

            UE_LOG(ItemManager, Display, TEXT("Item (%s) has been destroyed"), *ItemName)
        }
        else
        {
            if(!IsValid(CharacterMesh))
            {
                CharacterMesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
            }
            
            if(CharacterMesh)
            {
                Items[OldItemIndex].Actor->AttachToComponent(CharacterMesh, EnumAttachmentRulesToStuct(Items[OldItemIndex].ItemInfos.ItemDetachSocket.AttachementRules), Items[OldItemIndex].ItemInfos.ItemDetachSocket.ItemSocket);
            }
        }

    }
    else
    {
        UE_LOG(ItemManager, Warning, TEXT("Failed to destroy item"));
    }
}

void UItemManagerComponent::ActivateSwitching(int Delay)
{
    FTimerHandle TimerHandle;
    int OldItemIndex = CurrentItemIndex;

    // avoid a non called lambda
    if (Delay <= 0.0f)
    {
        bIsSwitchingItem = false;
    }
    else
    {
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, TimerHandle, OldItemIndex]()
        {
            bIsSwitchingItem = false;
        }, Delay, false);
    }
}

bool UItemManagerComponent::IsCurrentItemValid()
{
    return CurrentItemIndex >= 0 && CurrentItemIndex < Items.Num() && Items[CurrentItemIndex].Actor != nullptr;
}

UItemManagerComponent::UItemManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UItemManagerComponent::SwitchNextItem()
{
    AItemParent* ItemInstance;
    int NextItemIndex = CurrentItemIndex;

    if (NextItemIndex < 0 || NextItemIndex >= Items.Num()) { OnFailedtoSwitchItem.Broadcast(1); return; }

    int ItemCount = 0;
    do
    {
        NextItemIndex++;

        if (bLoopSwitching && NextItemIndex >= Items.Num())
        {
            NextItemIndex = 0;
        }

        ItemInstance = GetItemInstance(Items[NextItemIndex].Item);

        if (!ItemInstance) { return;  }

        ItemCount++;
    } while(NextItemIndex < Items.Num() - 1 && ItemCount < Items.Num() && !ItemInstance->CanBeSwitched());

    if(ItemCount == Items.Num() && ItemState != EItemState::IS_None)
    {
        UE_LOG(ItemManager, Display, TEXT("No item to switch on"))
        OnFailedtoSwitchItem.Broadcast(2);
        return;
    }

	SwitchItem(NextItemIndex);
}

void UItemManagerComponent::SwitchPreviousItem()
{
    AItemParent* ItemInstance;
    int PreviousItemIndex = CurrentItemIndex;

    if (Items.Num() <= 0 || PreviousItemIndex < 0) { OnFailedtoSwitchItem.Broadcast(1); return; }

    int ItemCount = 0;
    do
    {
        PreviousItemIndex--;

        if (bLoopSwitching && PreviousItemIndex < 0)
        {
            PreviousItemIndex = Items.Num() - 1;
        }

        ItemInstance = GetItemInstance(Items[PreviousItemIndex].Item);

        if (!ItemInstance) { return; }

        ItemCount++;
    } while (PreviousItemIndex >= 0 && ItemCount < Items.Num() && !ItemInstance->CanBeSwitched());

    if (ItemCount == Items.Num() && ItemState != EItemState::IS_None)
    {
        UE_LOG(ItemManager, Display, TEXT("No item to switch on"))
        OnFailedtoSwitchItem.Broadcast(2);
        return;
    }


	SwitchItem(PreviousItemIndex);
}

void UItemManagerComponent::SwitchIndexItem(int32 Index)
{
    SwitchItem(Index);
}

void UItemManagerComponent::DropItem() 
{
    int OldItemIndex = CurrentItemIndex;
    if(!bIsSwitchingItem && CurrentItemIndex >= 0 && CurrentItemIndex < Items.Num() && IsValid(Items[CurrentItemIndex].Actor) && Items[CurrentItemIndex].ItemInfos.bIsDropable)
    {
        UE_LOG(ItemManager, Display, TEXT("Removing Item(%s) at %d"), *Items[CurrentItemIndex].ItemInfos.FriendlyName, CurrentItemIndex)

        if(Items[CurrentItemIndex].Actor)
        {
            Items[CurrentItemIndex].Actor->Destroy();
        }
        
        CurrentItemIndex = 0;
        SpawnItem();

        FTransform Transform;
        if (!IsValid(CharacterMesh))
        {
             CharacterMesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
        }

        Transform.SetLocation(GetOwner()->GetActorLocation());

        if(CharacterMesh)
        {
            if(CharacterMesh->DoesSocketExist(Items[OldItemIndex].ItemInfos.ItemAttachSocket.ItemSocket))
            {
                Transform.SetLocation(CharacterMesh->GetSocketLocation(Items[OldItemIndex].ItemInfos.ItemAttachSocket.ItemSocket));
            }
            
        }

        AItemCollectable* ItemCollectable = GetWorld()->SpawnActorDeferred<AItemCollectable>(AItemCollectable::StaticClass(), Transform);
        ItemCollectable->Init(Items[OldItemIndex].ItemCollectableData);
        ItemCollectable->FinishSpawning(Transform);

        // Add ItemCollectable to ItemsCollectables
        ItemsCollectable.Add(ItemCollectable);

        Items.RemoveAt(OldItemIndex);

        ItemState = Items.Num() <= 0 ? EItemState::IS_None : ItemState;
    }
}

void UItemManagerComponent::UseItem()
{
    if(IsCurrentItemValid())
    {
        if(ItemState == EItemState::IS_Idle)
        {
            Items[CurrentItemIndex].Actor->UseItem(this);
        }
        else
        {
            UE_LOG(ItemManager, Warning, TEXT("Cannot use item"))
        }
        
    }
    else
    {
        UE_LOG(ItemManager, Warning, TEXT("The current item is invalid"))
    }
}

int UItemManagerComponent::AddItem(TSubclassOf<AItemParent> Item)
{
	if (!IsValid(Item)) 
    { 
        OnAddingItem.Broadcast(1);
        return 1; 
    }
    if (ItemLimit > 0 && Items.Num() >= ItemLimit) 
    {
        OnAddingItem.Broadcast(2);
        return 2; 
    }

    AItemParent* ItemInstance = Item.GetDefaultObject();
    
    FItemObject NewItem;
    NewItem.Item = Item;
    NewItem.Actor = nullptr;
    NewItem.ItemInfos = ItemInstance->GetItemInfos();

    if(bAllowsDuplicates || !Items.Contains(NewItem))
    {
        Items.Add(NewItem);
    }
    else
    {
        OnAddingItem.Broadcast(3);
        return 3;
    }
    
    ItemInstance = nullptr;

    OnAddingItem.Broadcast(0);
    return 0;
}

// Called when the game starts
void UItemManagerComponent::BeginPlay()
{
	Super::BeginPlay();

    // create an 'empty' item. this item wont have any actor
    TSubclassOf<AEmptyItem> EmptyItem;

    if(bAddEmptyItemByDefault)
    {
        EmptyItem = AEmptyItem::StaticClass();
        AddItem(EmptyItem);
        SpawnItem();
    }

    // get all items collectables actor in the world
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AItemCollectable::StaticClass(), FoundActors);
    for (AActor* Actor : FoundActors)
    {
        AItemCollectable* ItemCollectable = Cast<AItemCollectable>(Actor);

        if (ItemCollectable)
        {
            ItemsCollectable.Add(ItemCollectable);
        }
    }
}

void UItemManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if(SpawnItemTimerHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(SpawnItemTimerHandle);
    }
    
    if (DestroyItemTimerHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(DestroyItemTimerHandle);
    }
}


// Called every frame
void UItemManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    //GEngine->AddOnScreenDebugMessage(0, MAX_flt, FColor::Cyan, FString(TEXT("Item State : ") + ItemStateToString(ItemState)));
}

void ItemManager_OnItemUse(UItemManagerComponent* ItemManagerClass, AItemParent* ItemClass)
{
    //ItemClass->OnItemUse()
}
