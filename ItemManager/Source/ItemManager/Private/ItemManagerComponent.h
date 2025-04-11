// Copyright Ryckbosch Arthur 2024. All Rights Reserved.



#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include <ItemParent.h>
#include <ItemCollectable.h>
#include <DefaultItems/EmptyItem.h>
#include "ItemManagerComponent.generated.h"

UENUM(BlueprintType)
enum class EItemState : uint8
{
    IS_None UMETA(DisplayName = "None"),
    IS_Switching UMETA(DisplayName = "Switching"),
    IS_Idle UMETA(DisplayName = "Idle")
};

USTRUCT(BlueprintType)
struct FItemObject
{
    GENERATED_USTRUCT_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Object")
    TSubclassOf<AItemParent> Item;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Object")
    AItemParent* Actor;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Object")
    FItemInfos ItemInfos;

    FItemCollectableData ItemCollectableData;

    bool operator==(const FItemObject& Other) const
    {
        return Item == Other.Item;
    }
};
DECLARE_LOG_CATEGORY_EXTERN(ItemManager, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnItemCollectedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCannotCollectItemDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnitemSwitchedDelegate, FItemObject, SwitchedItem);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFailedtoSwitchItem, int, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemSpawnedDelegate, FItemObject, SpawnedItem);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemDespawnedDelegate, FItemObject, DespawnedItem);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeginOverlapDelegate, AItemCollectable*, NewItem);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndOverlapDelegate, AItemCollectable*, NewItem);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAddingItem, int, Value);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent, DisplayName = "Item Manager", ToolTip = "Item Manager Component"), Category = "Item Manager")
class UItemManagerComponent : public UActorComponent
{
	GENERATED_BODY()

private:
    TArray<FItemObject> Items;
    TArray<AItemCollectable*> ItemsCollectable;
	int CurrentItemIndex;
    bool bIsSwitchingItem;
	AItemCollectable* CurrentItemCollectable;
    EItemState ItemState = EItemState::IS_None;
    USkeletalMeshComponent* CharacterMesh;
    FTimerHandle SpawnItemTimerHandle;
    FTimerHandle DestroyItemTimerHandle;

    AItemParent* GetItemInstance(TSubclassOf<AItemParent> Item);
    void PickupItem(AItemParent* item);
	void SwitchItem(int newItemIndex);
    void SpawnItemLambda(float delay);
    void SpawnItem();
    void DestroyItemLambda(float delay);
    void DestroyItem(int OldItemIndex);
    void ActivateSwitching(int Delay);
    bool IsCurrentItemValid();
    FItemCollectableData SetItemCollectableData(AItemCollectable* ItemCollectable);
    FAttachmentTransformRules EnumAttachmentRulesToStuct(EAttachmentRules AttachmentRules);

public:	
	// Sets default values for this component's properties
	UItemManagerComponent();

	void OnPickableBeginOverlap(AItemCollectable* ItemCollectable);
	void OnPickableEndOverlap(AItemCollectable* ItemCollectable);

protected:

	UFUNCTION(BlueprintCallable, Category = "Item")
	void CollectItem();
	
    // return value based on error. such as invalid item or item limit reach.
    // 0 --> No error
    // 1 --> Invalid Item
    // 2 --> Limit Reach
    // 3 --> Duplicates
    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Add Item", ToolTip = "Add an item to the list of items. If item already exist, it will not be added."), Category = "Item Manager")
    int AddItem(TSubclassOf<AItemParent> Item);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Switch Next Item"), Category = "Item Manager")
	void SwitchNextItem();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Switch Previous Item"), Category = "Item Manager")
	void SwitchPreviousItem();

    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Switch Index Item"), Category = "Item Manager")
    void SwitchIndexItem(int32 Index);

    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Drop Item"), Category = "Item Manager")
    void DropItem();

    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Use Item"), Category = "Item Manager")
    void UseItem();

    UPROPERTY(EditAnywhere, meta = (DisplayName = "Loop Switching", ToolTip = "If true, Item Manager will loop in the items list. "), Category = "Item Manager")
    bool bLoopSwitching{ true };

    UPROPERTY(EditAnywhere, meta = (DisplayName = "Add Empty Item By Default", ToolTip = "Called when item 'spawn' (even if the actor is already loaded, this will be called)"), Category = "Item Manager")
    bool bAddEmptyItemByDefault{ true };

    UPROPERTY(EditAnywhere, meta = (DisplayName = "Allows Duplicates", ToolTip = ""), Category = "Item Manager")
    bool bAllowsDuplicates{ true };

    UPROPERTY(EditAnywhere, meta = (DisplayName = "Item Limit", ToolTip = "Litmit the number of item.\nIf set to 0, the number of item will be unlimited"), Category = "Item Manager")
    int ItemLimit{ 0 };



	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "On Switched Item", ToolTip = "Called when switching item has been done successfully."), Category = "Item Manager")
	FOnitemSwitchedDelegate OnitemSwitchedDelegate;

	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "Failed To Switch Item", ToolTip = "Called when the item manager failed to switch item. When the item is invalid or trying to switch on the same item\n1 Invalid Item.\n2 Try to switch on the same item.\n3 Item is switching"), Category = "Item Manager")
	FOnFailedtoSwitchItem OnFailedtoSwitchItem;
    
    UPROPERTY(BlueprintAssignable, meta = (DisplayName = "On Spawned Item", ToolTip = "Called when item 'spawn' when switched (even if the actor is already loaded, this event will be called)"), Category = "Item Manager")
    FOnItemSpawnedDelegate OnItemSpawnedDelegate;

	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "On Despawn Item", ToolTip = "Called when item 'despawn' when switched (only called if item can despawn)"), Category = "Item Manager")
	FOnItemDespawnedDelegate OnItemDespawnedDelegate;

    UPROPERTY(BlueprintAssignable, meta = (DisplayName = "On Item Collected", ToolTip = "Called when item is collected"), Category = "Item Manager")
    FOnItemCollectedDelegate OnItemCollectedDelegate;

    UPROPERTY(BlueprintAssignable, meta = (DisplayName = "Cannot Collect Item", ToolTip = "Called when the item cannot be collected"), Category = "Item Manager")
    FCannotCollectItemDelegate CannotCollectItemDelegate;

    UPROPERTY(BlueprintAssignable, meta = (DisplayName = "On Begin Overlap Collectable Item", ToolTip = "Called when ItemCollectable's TriggerBox is begin overlapping"), Category = "Item Manager")
    FOnBeginOverlapDelegate OnBeginOverlapDelegate;

    UPROPERTY(BlueprintAssignable, meta = (DisplayName = "On End Overlap Collectable Item", ToolTip = "Called when ItemCollectable's TriggerBox is end overlapping"), Category = "Item Manager")
    FOnEndOverlapDelegate OnEndOverlapDelegate;

    UPROPERTY(BlueprintAssignable, meta = (DisplayName = "On Adding Item", ToolTip = "Called when adding an item.\n 0 : No error.\n 1 : Invalid item.\n 2 : Item limit reached.\n 3 : Duplicates."), Category = "Item Manager")
    FOnAddingItem OnAddingItem;

	virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	
    UFUNCTION(BlueprintPure, BlueprintCallable, meta = (DisplayName = "Get Current Item", ToolTip = "Get the current item"), Category = "Item Manager")
    FItemObject GetCurrentItem() const { return Items[CurrentItemIndex]; };

    UFUNCTION(BlueprintPure, BlueprintCallable, meta = (DisplayName = "Get Current Item Index", ToolTip = "Get the current item"), Category = "Item Manager")
    int32 GetCurrentItemIndex() const { return CurrentItemIndex; };

    UFUNCTION(BlueprintPure, BlueprintCallable, meta = (DisplayName = "Get Items", ToolTip = "Get the current item"), Category = "Item Manager")
    TArray<FItemObject> GetItems() const { return Items; };

    UFUNCTION(BlueprintPure, BlueprintCallable, meta = (DisplayName = "Get Items Collectable", ToolTip = "Get all items collectable in the world"), Category = "Item Manager")
    TArray<AItemCollectable*> GetItemsCollectables() const { return ItemsCollectable; };

    UFUNCTION(BlueprintPure, BlueprintCallable, meta = (DisplayName = "Get Item Limit", ToolTip = "Get the max number of items"), Category = "Item Manager")
    int GetItemLimit() const { return ItemLimit; };

    UFUNCTION(BlueprintPure, BlueprintCallable, meta = (DisplayName = "Is Allowing Duplicates", ToolTip = "Return true if the item manager allows duplicates items"), Category = "Item Manager")
    bool IsAllowingDuplicates() const { return bAllowsDuplicates; };

    UFUNCTION(BlueprintPure, BlueprintCallable, meta = (DisplayName = "Is Adding Empty Item By Default", ToolTip = "Return true if the item manager add an empty item when construct"), Category = "Item Manager")
    bool IsAddingEmptyItemByDefault() const { return bAddEmptyItemByDefault; }; 

    UFUNCTION(BlueprintPure, BlueprintCallable, meta = (DisplayName = "Is Loop Switching", ToolTip = "Return true if the item manager makes a loop when switching item"), Category = "Item Manager")
    bool IsLoopSwitching() const { return bLoopSwitching; };

    UFUNCTION(BlueprintPure, BlueprintCallable, meta = (DisplayName = "Get Item State", ToolTip = "Return the current item state."), Category = "Item Manager")
    EItemState GetItemState() const { return ItemState; };

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
