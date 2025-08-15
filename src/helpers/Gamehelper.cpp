#include <pch.h>  // Always include pch first
#include "Engine_classes.hpp"
#include "Pal_classes.hpp"
#include <random>
#include <ctime>
#include <mutex>
#include <queue>
#include <future>
#include <memory>
#include <type_traits>

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
        if (!Helper::IsProbablyValidPtr(pChar)) return false;

        // Cast then verify again as APalCharacter
        SDK::APalCharacter* pal = static_cast<SDK::APalCharacter*>(pChar);
        if (!Helper::IsProbablyValidPtr(pal)) return false;

        // Access member safely (read can fault if owner is stale)
        auto* params = (SDK::UPalCharacterParameterComponent*)nullptr;
        if (!Helper::Try([&] { params = pal->CharacterParameterComponent; }) ||
            !Helper::IsProbablyValidPtr(params))
            return false;

        // Get default utility safely
        SDK::UPalUtility* util = nullptr;
        if (!Helper::Try([&] { util = SDK::UPalUtility::GetDefaultObj(); }) ||
            !Helper::IsProbablyValidPtr(util))
            return false;

        bool isDead = false;
        if (!Helper::SafeCallRet(isDead, [&] { return util->IsDead(pal); }))
            return false;                     // treat failed call as "not safe"

        return !isDead;
    }

    bool IsABaseWorker(SDK::APalCharacter* pChar, bool bLocalControlled)
    {
        if (!Helper::IsProbablyValidPtr(pChar)) return false;
        if (!IsAlive(pChar)) return false;

        SDK::UPalUtility* util = nullptr;
        if (!Helper::Try([&] { util = SDK::UPalUtility::GetDefaultObj(); }) ||
            !Helper::IsProbablyValidPtr(util))
            return false;

        bool out = false;
        bool ok = bLocalControlled
            ? Helper::SafeCallRet(out, [&] { return util->IsLocalPlayerCampPal(pChar); })
            : Helper::SafeCallRet(out, [&] { return util->IsBaseCampPal(pChar); });

        return ok && out;
    }

    bool IsTamed(SDK::APalCharacter* pChar)
    {
        if (!Helper::IsProbablyValidPtr(pChar)) return false;
        if (!IsAlive(pChar)) return false;

        SDK::UPalUtility* util = nullptr;
        if (!Helper::Try([&] { util = SDK::UPalUtility::GetDefaultObj(); }) ||
            !Helper::IsProbablyValidPtr(util))
            return false;

        bool isOtomo = false;
        if (!Helper::SafeCallRet(isOtomo, [&] { return util->IsPlayersOtomo(pChar); }))
            return false;

        return isOtomo;
    }

    float GetDistance(const SDK::FVector2D& a, const SDK::FVector2D& b)
    {
        return sqrtf(
            (a.X - b.X) * (a.X - b.X) +
            (a.Y - b.Y) * (a.Y - b.Y)
        );
    }

    bool HasCameraLOS_Kismet(SDK::APlayerController* PC,
        SDK::APalPlayerCharacter* Player,
        SDK::APalCharacter* Pal)
    {
        if (!PC || !Player || !Pal) return false;

        // Camera origin
        FVector camLoc; FRotator camRot;
        PC->GetPlayerViewPoint(&camLoc, &camRot);

        // Target points (remove head if your dump lacks it)
        const FVector head = Pal->GetHPGaugeLocation();
        const FVector base = Pal->K2_GetActorLocation();
        const FVector chest = base + FVector(0.f, 0.f, 40.f);

        // Use TraceTypeQuery1 (commonly Visibility)
        const SDK::ETraceTypeQuery visTrace = SDK::ETraceTypeQuery::TraceTypeQuery1;

        auto TryPoint = [&](const FVector& tgt)->bool
            {
                // Pass 1: ignore only the local player
                SDK::TArray<SDK::AActor*> ignore1;
                ignore1.Add(Player);

                SDK::FHitResult hit1{};
                const bool pass1 =
                    SDK::UKismetSystemLibrary::LineTraceSingle(
                        /*WorldContextObject*/ Player,
                        /*Start*/ camLoc,
                        /*End*/   tgt,
                        /*TraceChannel*/ visTrace,
                        /*bTraceComplex*/ true,
                        /*ActorsToIgnore*/ ignore1,
                        /*DrawDebugType*/ SDK::EDrawDebugTrace::ForDuration,
                        /*OutHit*/ &hit1,
                        /*bIgnoreSelf*/ true,
                        /*TraceColor*/   SDK::FLinearColor(0, 0, 0, 0),
                        /*TraceHitColor*/SDK::FLinearColor(0, 0, 0, 0),
                        /*DrawTime*/     3.0f
                    );

                if (!hit1.bBlockingHit)
                    return true; // clear path

                // Pass 2: also ignore the Pal; if it becomes clear, first hit was the Pal
                SDK::TArray<SDK::AActor*> ignore2 = ignore1;
                ignore2.Add(Pal);

                SDK::FHitResult hit2{};
                const bool pass2 =
                    SDK::UKismetSystemLibrary::LineTraceSingle(
                        Player, camLoc, tgt, visTrace, true, ignore2,
                        SDK::EDrawDebugTrace::None, &hit2, true,
                        SDK::FLinearColor(0, 0, 0, 0), SDK::FLinearColor(0, 0, 0, 0), 0.0f
                    );

                if (!hit2.bBlockingHit)
                    return true; // after ignoring Pal, nothing blocks => Pal was first hit

                return false;    // still blocked by world/other
            };

        if (TryPoint(head))  return true;
        if (TryPoint(chest)) return true;
        if (TryPoint(base))  return true;

        return false;
    }
}
