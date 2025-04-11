// Copyright Ryckbosch Arthur 2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemParent.generated.h"

UENUM(BlueprintType)
enum class EAttachmentRules : uint8
{
	AR_KeepRelativeTransform			UMETA(DisplayName = "Keep Relative Transform"),
	AR_KeepWorldTransform				UMETA(DisplayName = "Keep World Transform"),
	AR_SnapToTargetIncludingScale		UMETA(DisplayName = "Snap To Target Including Scale"),
	AR_SnapToTargetNotIncludingScale	UMETA(DisplayName = "Snap To Target Not Including Scale"),
};

USTRUCT(BlueprintType)
struct FItemSocket
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Item Socket"), Category = "Item Infos")
	FName ItemSocket{ "None" };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Item Attachment Rules"), Category = "Item Infos")
	EAttachmentRules AttachementRules{ EAttachmentRules::AR_KeepRelativeTransform };
};

USTRUCT(BlueprintType)
struct FItemInfos
{
    GENERATED_USTRUCT_BODY()
    
    UPROPERTY(EditAnywhere, Category = "Item Infos")
    FString FriendlyName{ "Item Name" };
    
    UPROPERTY(EditAnywhere, meta = (DisplayName = "Item Attach Socket", ToolTip = "Name of skeletal owner socket. If item is droppable, bone info like position will be use for the drop system."), Category = "Item Infos")
	FItemSocket ItemAttachSocket;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Item Detach Socket", ToolTip = "Name of skeletal owner socket. This socket will be used if item is not destroy when switched"), Category = "Item Infos")
	FItemSocket ItemDetachSocket;
    
    UPROPERTY(EditAnywhere, Category = "Item Infos")
	FTransform SpawnRelativeTransform{ {0.f, 0.f, 0.f}, {0.f, 0.f, 0.f}, {1.f, 1.f, 1.f} };
    
    UPROPERTY(EditAnywhere, Category = "Item Infos")
    float TimeBeforeDespawn{ 1.f };
    
    UPROPERTY(EditAnywhere, Category = "Item Infos")
    float TimeBeforeSpawn{ 1.f };

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Is Dropable"), Category = "Item Infos")
	bool bIsDropable{ true };
};


class UItemManagerComponent;

UCLASS()
class ITEMMANAGER_API AItemParent : public AActor
{
	GENERATED_BODY()


protected:


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	USkeletalMeshComponent* SkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FItemInfos ItemInfos;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "If true, the item will be able to be used."), Category = "Item")
	bool bCanBeUsed = true; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "If true, the item will be able to be collected (as ItemCollectable)."), Category = "Item")
	bool bCanBeCollected = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "If true, the item will be able to switch on. if not, the item manager will skip this item and switch to the next/previous one."), Category = "Item")
	bool bCanBeSwitched = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "If true, the item will be automatically equipped when picked up"), Category = "Item")
	bool bEquipWhenPickedUp = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "If true, the item will despawn when switch. If not, the item will be re-attach to the skeletal mesh owner with the 'detach' socket."), Category = "Item")
	bool bDespawnItemWhenSwitched = false;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Item Used"), Category = "Item")
	void OnItemUsed_BP();
	virtual void OnItemUsed();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Cannot Use Item"), Category = "Item")
	void CannotUseItem_BP();
	virtual void CannotUseItem();

	virtual void BeginPlay() override;

public:	
	
	AItemParent();
    
	UFUNCTION(BlueprintPure, BlueprintCallable, meta = (DisplayName = "Get Item Infos"), Category = " Get Item")
	FItemInfos GetItemInfos() { return ItemInfos; }

	UFUNCTION(BlueprintPure, BlueprintCallable, meta = (DisplayName = "Can Be Used"), Category = "Get Item")
	bool CanBeUsed() const { return bCanBeUsed; }

	UFUNCTION(BlueprintPure, BlueprintCallable, meta = (DisplayName = "Can Be Collcted"), Category = "Get Item")
	bool CanBeCollected() const { return bCanBeCollected; }

	UFUNCTION(BlueprintPure, BlueprintCallable, meta = (DisplayName = "Can Be Switched"), Category = "Get Item")
	bool CanBeSwitched() const { return bCanBeSwitched; }

	UFUNCTION(BlueprintPure, BlueprintCallable, meta = (DisplayName = "Equip When Picked Up"), Category = "Get Item")
	bool EquipWhenPickedUp() const { return bEquipWhenPickedUp; }

	UFUNCTION(BlueprintPure, BlueprintCallable, meta = (DisplayName = "Is Despawning Item When Switched"), Category = "Get Item")
	bool IsItemDespawnWhenSwitched() const { return bDespawnItemWhenSwitched; }

	USkeletalMeshComponent* GetSkeletalMesh() { return SkeletalMesh; }
	void UseItem(UItemManagerComponent* ItemManagerComponent);

	virtual void Tick(float DeltaTime) override;

};


