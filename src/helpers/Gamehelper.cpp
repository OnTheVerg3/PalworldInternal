#include <pch.h>  // Always include pch first
#include "Engine_classes.hpp"
#include "Pal_classes.hpp"

using namespace SDK;

namespace Helper
{
    APalPlayerCharacter* GetPalPlayerCharacter()
    {

        UWorld* world = UWorld::GetWorld();
        if (!world) {
            return nullptr;
        }

        if (!world->OwningGameInstance) {
            return nullptr;
        }

        auto localPlayers = world->OwningGameInstance->LocalPlayers;
        if (localPlayers.Num() == 0) {
            return nullptr;
        }

        auto controller = localPlayers[0]->PlayerController;
        if (!controller) {
            return nullptr;
        }

        if (!controller->AcknowledgedPawn) {
            return nullptr;
        }

        return static_cast<APalPlayerCharacter*>(controller->AcknowledgedPawn);
    }

    ULocalPlayer* GetLocalPlayer()
    {

        UWorld* world = UWorld::GetWorld();
        if (!world) {
            return nullptr;
        }

        if (!world->OwningGameInstance) {
            return nullptr;
        }

        const auto& localPlayers = world->OwningGameInstance->LocalPlayers;
        if (localPlayers.Num() == 0) {
            return nullptr;
        }

        return localPlayers[0];
    }

    SDK::APalPlayerController* GetPalPlayerController()
    {
        SDK::APalPlayerCharacter* pPlayer = GetPalPlayerCharacter();
        if (!pPlayer)
            return nullptr;

        return static_cast<SDK::APalPlayerController*>(pPlayer->GetPalPlayerController());
    }

    APalPlayerState* GetPalPlayerState()
    {
        SDK::APalPlayerCharacter* pPlayer = GetPalPlayerCharacter();
        if (!pPlayer)
            return nullptr;

        return static_cast<APalPlayerState*>(pPlayer->PlayerState);
    }

    UPalPlayerInventoryData* GetInventoryComponent()
    {
        APalPlayerState* pPlayerState = GetPalPlayerState();
        if (!pPlayerState)
            return nullptr;

        return pPlayerState->InventoryData;
    }

    SDK::UPalCharacterImportanceManager* GetCharacterImpManager()
    {
        SDK::UWorld* pWorld = UWorld::GetWorld();
        if (!pWorld)
            return nullptr;

        SDK::UGameInstance* pGameInstance = pWorld->OwningGameInstance;
        if (!pGameInstance)
            return nullptr;

        return static_cast<SDK::UPalGameInstance*>(pGameInstance)->CharacterImportanceManager;
    }

    bool GetTAllNPC(SDK::TArray<class SDK::APalCharacter*>* outResult)
    {
        UPalCharacterImportanceManager* mPal = GetCharacterImpManager();
        if (!mPal)
            return false;

        mPal->GetAllNPC(outResult);
        return true;
    }

    bool GetTAllPals(SDK::TArray<class SDK::APalCharacter*>* outResult)
    {
        SDK::UPalCharacterImportanceManager* mPal = GetCharacterImpManager();
        if (!mPal)
            return false;

        mPal->GetAllPalCharacter(outResult);
        return true;
    }

    SDK::APalWeaponBase* GetPlayerEquippedWeapon()
    {
        SDK::APalPlayerCharacter* pPalCharacter = GetPalPlayerCharacter();
        if (!pPalCharacter)
            return nullptr;

        SDK::UPalShooterComponent* pWeaponInventory = pPalCharacter->ShooterComponent;
        if (!pWeaponInventory)
            return nullptr;

        return pWeaponInventory->HasWeapon;
    }

    bool IsAlive(SDK::AActor* pChar)
    {
        SDK::UPalUtility* pUtil = UPalUtility::GetDefaultObj();
        SDK::APalPlayerCharacter* pLocalChar = GetPalPlayerCharacter();
        if (!pUtil || !pLocalChar || !pChar)
            return false;

        return !pUtil->IsDead(pChar);
    }

    bool IsABaseWorker(SDK::APalCharacter* pChar, bool bLocalControlled)
    {
        SDK::UPalUtility* pUtil = UPalUtility::GetDefaultObj();
        SDK::APalPlayerCharacter* pLocalChar = GetPalPlayerCharacter();
        if (!pUtil || !pLocalChar || !pChar || !IsAlive(pChar))
            return false;

        bool bResult = bLocalControlled ? pUtil->IsLocalPlayerCampPal(pChar) : pUtil->IsBaseCampPal(pChar);
        return bResult;
    }

}
