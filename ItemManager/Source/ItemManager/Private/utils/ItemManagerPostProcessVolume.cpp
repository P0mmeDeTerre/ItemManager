// Copyright Ryckbosch Arthur 2024. All Rights Reserved.


#include "utils/ItemManagerPostProcessVolume.h"

AItemManagerPostProcessVolume::AItemManagerPostProcessVolume()
{
    FString PostProcessMaterialPath = TEXT("'/ItemManager/Materials/M_TransparencyPP_Inst.M_TransparencyPP_Inst'");

    static ConstructorHelpers::FObjectFinder<UMaterialInstance> PostProcessMateriallObject(*PostProcessMaterialPath);

    if (PostProcessMateriallObject.Succeeded()) 
    {
        PostProcessMaterialInstance = PostProcessMateriallObject.Object;
        UMaterialInterface* PostProcessMaterialInterface = reinterpret_cast<UMaterialInterface*>(PostProcessMaterialInstance);
        Settings.AddBlendable(PostProcessMaterialInterface, 1.f);
    }
	bUnbound = true;
    SetActorLocation({ 0.f, 0.f, 0.f });
}
