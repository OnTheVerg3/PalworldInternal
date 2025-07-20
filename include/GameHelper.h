#pragma once
#include "Pal_classes.hpp"

namespace Helper
{
    SDK::APalPlayerCharacter* GetPalPlayerCharacter();
    SDK::ULocalPlayer* GetLocalPlayer();
    SDK::APalPlayerController* GetPalPlayerController();
    SDK::UPalPlayerInventoryData* GetInventoryComponent();
    SDK::APalPlayerState* GetPalPlayerState();
    SDK::APalWeaponBase* GetPlayerEquippedWeapon();
    bool GetTAllPals(SDK::TArray<class SDK::APalCharacter*>* outResult);
    bool IsAlive(SDK::AActor* pCharacter);
    bool IsABaseWorker(SDK::APalCharacter* pChar, bool bLocalControlled = true);

    bool GetTAllNPC(SDK::TArray<class SDK::APalCharacter*>* outResult);
    SDK::UPalCharacterImportanceManager* GetCharacterImpManager();
}
