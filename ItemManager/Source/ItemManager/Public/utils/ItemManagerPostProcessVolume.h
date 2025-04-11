// Copyright Ryckbosch Arthur 2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/PostProcessVolume.h"
#include "Materials/MaterialInstance.h"
#include "UObject/ConstructorHelpers.h"
#include "ItemManagerPostProcessVolume.generated.h"

/**
 * 
 */
UCLASS()
class ITEMMANAGER_API AItemManagerPostProcessVolume : public APostProcessVolume
{
	GENERATED_BODY()


public:
	AItemManagerPostProcessVolume();

private:

	UMaterialInstance* PostProcessMaterialInstance = nullptr;

	void setupPostProcess();
	
};
