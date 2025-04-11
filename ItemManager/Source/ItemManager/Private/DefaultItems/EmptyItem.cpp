// Copyright Ryckbosch Arthur 2024. All Rights Reserved.


#include "DefaultItems/EmptyItem.h"

AEmptyItem::AEmptyItem()
{
	ItemInfos.FriendlyName = "Empty";
	ItemInfos.ItemAttachSocket.ItemSocket = "Root";
	ItemInfos.ItemAttachSocket.AttachementRules = EAttachmentRules::AR_KeepRelativeTransform;
	ItemInfos.ItemDetachSocket.ItemSocket = "Root";
	ItemInfos.ItemDetachSocket.AttachementRules = EAttachmentRules::AR_KeepRelativeTransform;
	ItemInfos.TimeBeforeDespawn = 0.0f;
	ItemInfos.TimeBeforeSpawn = 0.0f;
	ItemInfos.bIsDropable = false;

	bCanBeCollected = false;
	bCanBeSwitched = true;
	bCanBeUsed = false;
	bEquipWhenPickedUp = false;
	bDespawnItemWhenSwitched = true;
}
