#include <pch.h>
#include "cheat_state.h"
#include "Menu.h"
#include "pal_editor.h"

using namespace SDK;
using namespace Helper;

float GetDistanceToActor(AActor* pLocal, AActor* pTarget)
{
	if (!pLocal || !pTarget)
		return -1.f;

	FVector pLocation = pLocal->K2_GetActorLocation();
	FVector pTargetLocation = pTarget->K2_GetActorLocation();
	double distance = sqrt(pow(pTargetLocation.X - pLocation.X, 2.0) + pow(pTargetLocation.Y - pLocation.Y, 2.0) + pow(pTargetLocation.Z - pLocation.Z, 2.0));

	return distance / 100.0f;
}

void ChangeWorldSpeed(float speed)
{
	UWorld* pWorld = UWorld::GetWorld();
	if (!pWorld)
		return;

	ULevel* pLevel = pWorld->PersistentLevel;
	if (!pLevel)
		return;

	AWorldSettings* pWorldSettings = pLevel->WorldSettings;
	if (!pWorldSettings)
		return;

	pWorld->PersistentLevel->WorldSettings->TimeDilation = speed;
}


void SetPlayerAttackParam()
{
	APalPlayerCharacter* pPalPlayerCharacter = GetPalPlayerCharacter();
	if (!pPalPlayerCharacter)
		return;

	UPalCharacterParameterComponent* pParams = pPalPlayerCharacter->CharacterParameterComponent;
	if (!pParams)
		return;

	int value = cheatState.attack * 50;
	pParams->AttackUp = value;
}


void SetPlayerInventoryWeight()
{
	UPalPlayerInventoryData* pInventory = GetInventoryComponent();
	if (!pInventory)
		return;

	pInventory->MaxInventoryWeight = cheatState.weight;

	// Not really necessary, but just in case if there are checks in the game that rely on these values
	pInventory->OnRep_maxInventoryWeight();
	pInventory->OnRep_BuffMaxWeight();
	pInventory->OnRep_BuffCurrentWeight();


	// Force server inventory refresh
	pInventory->RequestForceMarkAllDirty_ToServer(true);
	
}

void SetInfiniteAmmo()
{
	APalPlayerCharacter* pPalCharacter = GetPalPlayerCharacter();
	if (!pPalCharacter)
		return;

	UPalShooterComponent* pShootComponent = pPalCharacter->ShooterComponent;
	if (!pShootComponent)
		return;

	APalWeaponBase* pWeapon = pShootComponent->HasWeapon;

	if (!pWeapon)
	{
		return;
	}

	pWeapon->IsRequiredBullet = cheatState.infAmmo ? false : true;

}

void IncreaseAllDurability()
{
	APalPlayerCharacter* player = GetPalPlayerCharacter();
	if (!player) return;

	UPalShooterComponent* pShootComponent = player->ShooterComponent;
	if (!pShootComponent) return;

	APalWeaponBase* pWeapon = pShootComponent->HasWeapon;
	if (!pWeapon) return;

	float currentDurability = pWeapon->GetDurability();

	UPalDynamicWeaponItemDataBase* dynData = pWeapon->TryGetDynamicWeaponData();
	if (!dynData)
		return;	
	
	float newDurability = currentDurability + 99999.0f;
	dynData->Durability = newDurability;

}

int originalAttackValue = 0;
bool originalValueSaved = false;

void SetWeaponDamage()
{
	APalPlayerCharacter* player = GetPalPlayerCharacter();
	if (!player) return;

	UPalShooterComponent* pShootComponent = player->ShooterComponent;
	if (!pShootComponent) return;

	APalWeaponBase* pWeapon = pShootComponent->HasWeapon;
	if (!pWeapon) return;

	UPalStaticWeaponItemData* stat = pWeapon->ownWeaponStaticData;
	if (!stat)
		return;

	if (!originalValueSaved)
	{
		originalAttackValue = stat->AttackValue;
		originalValueSaved = true;
	}

	stat->AttackValue = originalAttackValue * cheatState.weaponDamage;
}

void SetInfiniteMagazine()
{
	APalPlayerCharacter* player = GetPalPlayerCharacter();
	if (!player) return;

	UPalShooterComponent* pShootComponent = player->ShooterComponent;
	if (!pShootComponent) return;

	APalWeaponBase* pWeapon = pShootComponent->HasWeapon;
	if (!pWeapon) return;

	pWeapon->IsInfinityMagazine = cheatState.infMag;

}

void ResetStamina()
{
	if (!cheatState.infStamina)
		return;

	APalPlayerCharacter* pPalCharacter = GetPalPlayerCharacter();
	if (!pPalCharacter)
		return;

	UPalCharacterParameterComponent* pParams = pPalCharacter->CharacterParameterComponent;
	if (!pParams)
		return;

	pParams->ResetSP();
}

FGuid GenerateGuidRandomly()
{
	// Seed the RNG once
	static bool seeded = false;
	if (!seeded)
	{
		std::srand(static_cast<unsigned int>(std::time(nullptr)));
		seeded = true;
	}

	FGuid guid;

	auto gen = []() -> uint32_t {
		return static_cast<uint32_t>((std::rand() << 16) ^ std::rand());
		};

	guid.A = gen();
	guid.B = gen();
	guid.C = gen();
	guid.D = gen();

	return guid;
}

void AddItemToInventoryByName(std::string itemName, int count)
{
	static UKismetStringLibrary* lib = UKismetStringLibrary::GetDefaultObj();

	APalPlayerCharacter* pPalPlayerCharacter = GetPalPlayerCharacter();
	APalPlayerState* pPalPlayerState = GetPalPlayerState();
	if (!pPalPlayerCharacter || !pPalPlayerState)
		return;

	SDK::UPalPlayerInventoryData* pInventoryData = pPalPlayerState->GetInventoryData();
	if (!pInventoryData)
		return;

	FName Name = lib->Conv_StringToName(FString(std::wstring(itemName.begin(), itemName.end()).c_str()));


	UPalItemContainer* container = nullptr;

	if (!pInventoryData->TryGetContainerFromStaticItemID(Name, &container) || !container)
	{
		printf("[DEBUG] Could not resolve container for item!\n");
		return;
	}

	UPalItemSlot* slot = nullptr;
	if (!pInventoryData->TryGetEmptySlot(
		pInventoryData->GetInventoryTypeFromStaticItemID(Name),
		&slot) || !slot)
	{
		printf("[DEBUG] No empty slot found!\n");
		return;
	}


	pInventoryData->AddItem_ServerInternal(Name, count, true, 0.0f);

	slot->ItemId.StaticId = Name;
	slot->StackCount = count;
	container->ForceMarkSlotDirty_ServerInternal();


	FGuid RequestID = GenerateGuidRandomly();
	FPalItemSlotId SlotId = slot->GetSlotId();
	TArray<FPalItemSlotIdAndNum> Froms;
	Froms.Add({ SlotId, count });

	// Move from SlotId to itself
	APalPlayerController* pPalPlayerController = GetPalPlayerController();
	pPalPlayerController->Transmitter->Item->RequestMove_ToServer(RequestID, SlotId, Froms);
	pInventoryData->RequestForceMarkAllDirty_ToServer(true);

}

void TeleportPlayerTo(const FVector& pos)
{
	APalPlayerState* pPalPlayerState = GetPalPlayerState();
	APalPlayerController* pPalPlayerController = GetPalPlayerController();

	if (!pPalPlayerController || !pPalPlayerState)
		return;

	// to avoid spawning inside terrain +100
	FVector safeLocation = FVector(pos.X, pos.Y + 100.0f, pos.Z);
	FQuat defaultRotation(0.f, 0.f, 0.f, 1.f);

	if (!pPalPlayerController->Transmitter || !pPalPlayerController->Transmitter->Player) return;

	// Get player unique ID (needed for server call)
	FGuid guid = pPalPlayerState->PlayerUId;

	pPalPlayerController->Transmitter->Player->RegisterRespawnPoint_ToServer(guid, safeLocation, defaultRotation);

	pPalPlayerState->RequestRespawn();
}

void TeleportPlayerToHome()
{
	TArray<SDK::APalCharacter*> allPals;
	if (!GetTAllPals(&allPals))
		return;

	for(SDK::APalCharacter* pal : allPals)
	{
		if (!pal)
			continue;
		APalPlayerCharacter* palPlayer = static_cast<APalPlayerCharacter*>(pal);
		if (!palPlayer)
			continue;

		if (IsAlive(palPlayer) && IsABaseWorker(palPlayer))
		{
			FVector homeLocation = palPlayer->K2_GetActorLocation();
			TeleportPlayerTo(homeLocation);
			return;
		}	
	}
}

void SetCameraFov()
{
	APalPlayerCharacter* player = GetPalPlayerCharacter();
	if (!player) return;

	auto cameraComp = player->FollowCamera;
	if (!cameraComp) return;

	cameraComp->WalkFOV = cheatState.cameraFov;
	cameraComp->SprintFOV = cheatState.cameraFov;
	cameraComp->AimFOV = cheatState.cameraFov;
}

void SetCameraBrightness()
{
	APalPlayerCharacter* player = GetPalPlayerCharacter();
	if (!player) return;

	UPalCharacterCameraComponent* cameraComp = player->FollowCamera;
	if (!cameraComp) return;

	cameraComp->PostProcessSettings.bOverride_AutoExposureBias = true;
	cameraComp->PostProcessBlendWeight = 1.0f;

	cameraComp->PostProcessSettings.AutoExposureBias = cheatState.cameraBrightness;
}

void AddWaypointLocation(const std::string& wpName)
{
	APalCharacter* pPalCharacter = GetPalPlayerCharacter();
	if (!pPalCharacter) return;

	SDK::FVector wpLocation = pPalCharacter->K2_GetActorLocation();
	SDK::FRotator wpRotation = pPalCharacter->K2_GetActorRotation();

	g_Waypoints.emplace_back("[WAYPOINT] " + wpName, wpLocation, wpRotation);
}

bool RemoveWaypointLocationByName(const std::string& wpName)
{
	for (auto it = g_Waypoints.begin(); it != g_Waypoints.end(); ++it)
	{
		if (it->waypointName == wpName)
		{
			g_Waypoints.erase(it);
			return true;
		}
	}
	return false;
}

void CollectAllRelicsInMap()
{
	UWorld* world = UWorld::GetWorld();
	if (!world) return;

	APalPlayerController* controller = GetPalPlayerController();
	if (!controller || !controller->Transmitter || !controller->Transmitter->Player)
	{
		return;
	}

	APalPlayerCharacter* player = GetPalPlayerCharacter();
	if (!player || !player->InteractComponent)
	{
		return;
	}

	AActor* nearestRelic = nullptr;
	float nearestDistance = FLT_MAX;
	FVector playerLoc = player->K2_GetActorLocation();

	for (ULevel* level : world->Levels)
	{
		if (!level) continue;

		for (AActor* actor : level->Actors)
		{
			if (!actor) continue;

			std::string className;
			try { className = actor->Class->GetName(); }
			catch (...) { continue; }

			// Match relic class
			if (className == "BP_LevelObject_Relic_C" || className.find("PalLevelObjectRelic") != std::string::npos)
			{
				APalLevelObjectObtainable* relic = reinterpret_cast<APalLevelObjectObtainable*>(actor);
				if (!relic) continue;

				if (relic->bPickedInClient)
				{
					continue;
				}

				float dist = relic->K2_GetActorLocation().GetDistanceTo(playerLoc);
				if (dist < nearestDistance)
				{
					nearestDistance = dist;
					nearestRelic = relic;
				}
			}
		}
	}

	if (!nearestRelic)
	{
		return;
	}

	APalLevelObjectObtainable* relic = reinterpret_cast<APalLevelObjectObtainable*>(nearestRelic);

	if (relic && !relic->bPickedInClient)
	{
		// Request to collect the relic
		player->InteractComponent->SetEnableInteract(true, false);

		controller->Transmitter->Player->RequestObtainLevelObject_ToServer(relic);
	}
}

void RevealMapAroundPlayer()
{
	int touched = 0;
	int nullStreak = 0, idx = 0;

	while (nullStreak < 50000)
	{
		SDK::UObject* obj = SDK::UObject::GObjects->GetByIndex(idx++);
		if (!obj) { ++nullStreak; continue; }
		nullStreak = 0;

		if (obj->IsA(SDK::UPalGameSetting::StaticClass()) && !obj->IsDefaultObject())
		{
			static_cast<SDK::UPalGameSetting*>(obj)->worldmapUIMaskClearSize = 20000.0f;
			++touched;
		}
	}

	int touchedCDO = 0;
	if (auto* cdo = static_cast<SDK::UPalGameSetting*>(
		SDK::UObject::FindObjectFastImpl("Default__PalGameSetting")))
	{
		cdo->worldmapUIMaskClearSize = 20000.0f;
		touchedCDO = 1;
	}
}

void UnlockAllFastTravelPoints()
{
	APalPlayerController* PC = GetPalPlayerController();
	APalPlayerCharacter* Player = GetPalPlayerCharacter();
	if (!PC || !Player) { printf("[FTUnlock] missing PC/Player\n"); return; }

	// announce start (like the Lua)
	if (auto* Util = SDK::UPalUtility::GetDefaultObj())
		Util->SendSystemAnnounce(PC, FString(L"Attempting to unlock all Fast Travel Points..."));

	const bool bAuthority =
		(APalPlayerState*)Player->PlayerState ? ((APalPlayerState*)Player->PlayerState)->HasAuthority()
		: Player->HasAuthority();

	auto* NetPlayer = (PC->Transmitter ? PC->Transmitter->Player : nullptr);

	int total = 0, changed = 0, nulls = 0, idx = 0;
	while (nulls < 50000)
	{
		SDK::UObject* obj = SDK::UObject::GObjects->GetByIndex(idx++);
		if (!obj) { ++nulls; continue; }
		nulls = 0;

		if (!obj->IsA(SDK::APalLevelObjectUnlockableFastTravelPoint::StaticClass())) continue;
		auto* FT = static_cast<SDK::APalLevelObjectUnlockableFastTravelPoint*>(obj);
		++total;

		if (FT->bUnlocked) continue;

		if (bAuthority)
		{
			// Host/SP: exact same path as the Nexus Lua (ID 26 == UnlockFastTravel)
			FT->OnTriggerInteract(Player, EPalInteractiveObjectIndicatorType::UnlockFastTravel);
			++changed;
		}
		else if (NetPlayer)
		{
			// Client fallback: ask the server to unlock by ID
			NetPlayer->RequestUnlockFastTravelPoint_ToServer(FT->FastTravelPointID);
			++changed;
		}
	}
}

///////////////////////////////////// DEBUG FUNCTIONS ///////////////////////////////////////

void DebugBuildOverlap()
{
	SDK::APalPlayerCharacter* player = GetPalPlayerCharacter();
	if (!player)
	{
		printf("[DebugBuildOverlap] Player not found\n");
		return;
	}

	auto* builder = player->BuilderComponent;
	if (!builder)
	{
		printf("[DebugBuildOverlap] BuilderComponent is null\n");
		return;
	}

	auto* installChecker = builder->InstallChecker;
	if (!installChecker)
	{
		printf("[DebugBuildOverlap] InstallChecker is null\n");
		return;
	}

	auto* overlapChecker = installChecker->OverlapChecker;
	if (!overlapChecker)
	{
		printf("[DebugBuildOverlap] OverlapChecker is null\n");
		return;
	}

	printf("------ Build Overlap Debug ------\n");

	AActor* overlappedActor = overlapChecker->OverlappedActor;
	const auto& overlapBuildObjects = overlapChecker->OverlapBuildObjects;
	const auto& otherObjects = overlapChecker->OverlapOtherObjects;

	if (overlappedActor)
	{
		printf("  Overlapped Actor: %s\n", overlappedActor->GetName().c_str());
	}

	printf("OverlapBuildObjects (%d):\n", overlapBuildObjects.Num());
	for (int i = 0; i < overlapBuildObjects.Num(); ++i)
	{
		auto* obj = overlapBuildObjects[i];
		if (obj)
		{
			printf("  BuildObject [%d]: %s\n", i, obj->GetName().c_str());
		}
	}

	printf("OverlapOtherObjects (%d):\n", otherObjects.Num());
	for (int i = 0; i < otherObjects.Num(); ++i)
	{
		auto* obj = otherObjects[i];
		if (obj)
		{
			printf("  OtherObject [%d]: %s\n", i, obj->GetName().c_str());
		}
	}

	printf("---------------------------------\n");
}

