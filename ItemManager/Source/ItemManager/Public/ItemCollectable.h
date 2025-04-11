// Copyright Ryckbosch Arthur 2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <ItemParent.h>
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInstance.h"
#include "ItemCollectable.generated.h"


UENUM(BlueprintType)
enum class EItemDisplay : uint8
{
    ID_Grounded UMETA(DisplayName = "Grounded"),
    ID_Animated UMETA(DisplayName = "Animated"),
    ID_Physics  UMETA(DisplayName = "Physics"),
    ID_None     UMETA(DisplayName = "None")
};

UENUM(BlueprintType)
enum class EGroundedType : uint8
{
    GT_KeepGroundRotation UMETA(DisplayName = "Keep Ground Rotation"),
    GT_KeepMeshRotation UMETA(DisplayName = "Keep Mesh Rotation")
};

USTRUCT(BlueprintType)
struct FGroundTypeProperties
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, meta = (DisplayName = "Item Ground Type", ToolTip = ""), Category = "Item")
    EGroundedType GroundRotationType = EGroundedType::GT_KeepGroundRotation;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "Use Item Width Instead", ToolTip = "If your mesh is not proprely placed, use the width instead of the height of the mesh"), Category = "Item")
    bool UseItemWidthInstead = false;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "Item Max Height", ToolTip = ""), Category = "Item")
    float MaxHeight = { 300.f };

    UPROPERTY(EditAnywhere, meta = (DisplayName = "Item Adjusted Ground Rotator", ToolTip = ""), Category = "Item")
    FRotator AdjustedRotator{ 0.f, 0.f, 0.f };

    UPROPERTY(EditAnywhere, meta = (DisplayName = "Invert Ground Rotation", ToolTip = "If true, the rotation of the mesh will be inverted. Note : if check/uncheck Item Adjusted Ground Rotator will be reset."), Category = "Item")
    bool bInvertGroundRotation{ true };
};

USTRUCT(BlueprintType)
struct FAnimatedItemProperties
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Height", ToolTip = "Height of animated item"), Category = "Animated Item Infos")
    float Height{ 50.f };

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animated Item Infos")
    float HeightSpeed{ 1.f };

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animated Item Infos")
    float RotationSpeed{ 100.f };
};

USTRUCT(BlueprintType)
struct FItemCollectableData
{
    GENERATED_USTRUCT_BODY()

    TSubclassOf<AItemParent> Item;
    FVector Size{ 50.f, 50.f, 50.f };
    EItemDisplay ItemDisplay;
    FGroundTypeProperties GroundTypeProperties;
    FAnimatedItemProperties AnimatedItemProperties;
    bool bEnableCollisions;
    bool bEnableTransparency;
    bool bEnableOutline;
    UMaterialInstance* OutlineMaterial;
};

UCLASS()
class ITEMMANAGER_API AItemCollectable : public AActor
{
	GENERATED_BODY()
	
public:
    
    
	AItemCollectable();

    virtual void Tick(float DeltaTime) override;

    void Init(FItemCollectableData ItemCollectableData);

    UFUNCTION(BlueprintPure, BlueprintCallable, meta = (DisplayName = "Get Item", ToolTip = "Get the collectable item"), Category = "Item")
    TSubclassOf<AItemParent> GetItem() const { return Item; }

    UFUNCTION(BlueprintPure, BlueprintCallable, meta = (DisplayName = "Get Trigger Box Size", ToolTip = "Get the trigger box size"), Category = "Item")
    FVector GetTriggerBoxSize() const { return Size; }

    UFUNCTION(BlueprintPure, BlueprintCallable, meta = (DisplayName = "Get Item Display", ToolTip = "Get the item display type"), Category = "Item")
    EItemDisplay GetItemDisplay() const { return ItemDisplay; }

    UFUNCTION(BlueprintPure, BlueprintCallable, meta = (DisplayName = "Get Ground Type Properties", ToolTip = "Get the ground type properties"), Category = "Item")
    FGroundTypeProperties GetGroundTypeProperties() const { return GroundTypeProperties; }

    UFUNCTION(BlueprintPure, BlueprintCallable, meta = (DisplayName = "Get Animated Type Properties", ToolTip = "Get the animated type properties"), Category = "Item")
    FAnimatedItemProperties GetAnimatedItemProperties() const { return AnimatedItemProperties; }

    UFUNCTION(BlueprintPure, BlueprintCallable, meta = (DisplayName = "Are Collisions Enabled", ToolTip = "Return true if the collisions are enabled"), Category = "Item")
    bool AreCollisionsEnabled() const { return bEnableCollisions; }

    UFUNCTION(BlueprintPure, BlueprintCallable, meta = (DisplayName = "Is Transparency Enabled", ToolTip = "Return true if the transparency (see item throught walls) is enabled"), Category = "Item")
    bool IsTransparencyEnabled() const { return bEnableTransparency; }

    UFUNCTION(BlueprintPure, BlueprintCallable, meta = (DisplayName = "Is Outline Enabled", ToolTip = "Return true if the outline is enabled"), Category = "Item")
    bool IsOutlineEnabled() const { return bEnableOutline; }

    UFUNCTION(BlueprintPure, BlueprintCallable, meta = (DisplayName = "Get Outline Material", ToolTip = "Get the outline material"), Category = "Item")
    UMaterialInstance* GetOutlineMaterial() { return OutlineMaterial; }

    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Transparency", ToolTip = "Set the RenderCustomDepth value.\nThe transparency will be automatcly change if enter/exit the trigger box. To avoid this, use the enable/disable function."), Category = "Item")
    void SetTransparency(bool Value);

    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Enable Transparency", ToolTip = ""), Category = "Item")
    void EnableTransparency();

    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Disable Transparency", ToolTip = ""), Category = "Item")
    void DisableTransparency();

private:
    
    UPROPERTY(EditAnywhere, meta = (DisplayName = "Item To Be Collected", ToolTip = "Select item you want to be collectable here."), Category = "Item")
    TSubclassOf<AItemParent> Item;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "Trigger Item Size", ToolTip = "Set the Size of the trigger box"), Category = "Item")
    FVector Size{ 50.f, 50.f, 50.f };

    UPROPERTY(EditAnywhere, meta = (DisplayName = "Item Display Type", ToolTip = "Type of item display"), Category = "Item")
    EItemDisplay ItemDisplay = EItemDisplay::ID_None;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "Item Ground Type Properties", ToolTip = "Item ground properties", EditCondition = "ItemDisplay == EItemDisplay::ID_Grounded"), Category = "Item")
    FGroundTypeProperties GroundTypeProperties;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "Animated Item Properties", ToolTip = "", EditCondition = "ItemDisplay == EItemDisplay::ID_Animated"), Category = "Item")
    FAnimatedItemProperties AnimatedItemProperties;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "Enable Collisions", ToolTip = "", EditCondition = "ItemDisplay != EItemDisplay::ID_Physics"), Category = "Item")
    bool bEnableCollisions{ false };

    UPROPERTY(EditAnywhere, meta = (DisplayName = "Enable Transparency", ToolTip = ""), Category = "Item")
    bool bEnableTransparency{ false };

    UPROPERTY(EditAnywhere, meta = (DisplayName = "Enable Outine", ToolTip = "Outline will appear when the actor overlap the trigger box "), Category = "Item")
    bool bEnableOutline{ true };

    UPROPERTY(EditAnywhere,meta = (DisplayName = "Outline Material", ToolTip = "", EditCondition = "bEnableOutline"), Category = "Item")
    UMaterialInstance* OutlineMaterial;

    USkeletalMeshComponent* SkeletalMesh;
    USceneComponent* SceneComponent;
    UBoxComponent* TriggerBoxComponent;
    UClass* ItemClass;
    AItemParent* ItemInstance;

    float MeshHeight = 0.0f;
    float MeshWidth = 0.0f;
    float AnimatedRunningTime = 0.0f;
    bool bAnimatedGoingUp = true;
    bool bCacheInvertGroundRotation = GroundTypeProperties.bInvertGroundRotation;
    
	virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

    void SetItemInstance();
    void SetupMesh();
    void PlaceMeshToTheGround();
    void AnimatedMesh(float DeltaTime);
    void EnableCollisions();
    void DisableCollisions();

    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
    UFUNCTION()
    void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
    
};
