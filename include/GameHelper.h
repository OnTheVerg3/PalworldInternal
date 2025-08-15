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
    bool IsTamed(SDK::APalCharacter* pChar);

    bool GetTAllNPC(SDK::TArray<class SDK::APalCharacter*>* outResult);
    SDK::UPalCharacterImportanceManager* GetCharacterImpManager();
    float GetDistance(const SDK::FVector2D& a, const SDK::FVector2D& b);
    bool HasCameraLOS_Kismet(SDK::APlayerController* PC, SDK::APalPlayerCharacter* Player, SDK::APalCharacter* Pal);

    inline bool IsProbablyValidPtr(const void* p) {
        if (!p) return false;
        const uintptr_t a = reinterpret_cast<uintptr_t>(p);
#ifdef _WIN64
        if (a < 0x10000 || a > 0x00007FFFFFFF0000ull) return false;
#else
        if (a < 0x10000 || a > 0x7FFF0000u) return false;
#endif
        MEMORY_BASIC_INFORMATION mbi{};
        if (!VirtualQuery(reinterpret_cast<LPCVOID>(a), &mbi, sizeof(mbi))) return false;
        if (mbi.State != MEM_COMMIT) return false;
        if (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD)) return false;
        __try { volatile const void* vt = *reinterpret_cast<void* const*>(p); (void)vt; }
        __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
        return true;
    }

    template <typename Fn>
    inline bool Try(Fn&& fn) {
#if defined(_MSC_VER)
        __try { fn(); return true; }
        __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
#else
        fn(); return true;
#endif
    }

    template <class Ret, class Fn>
    inline bool SafeCallRet(Ret& out, Fn&& fn) {
        Ret tmp{};
        if (!Try([&] { tmp = fn(); })) return false;
        out = tmp;
        return true;
    }

}
