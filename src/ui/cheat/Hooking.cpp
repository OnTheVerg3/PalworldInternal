#include <pch.h>
#include "Engine.h"

using namespace SDK;
using namespace DX11Base;

using tProcessEvent = void(*)(UObject*, UFunction*, void*);
static tProcessEvent oProcessEvent = nullptr;

static UFunction* gFnCreateParam = nullptr; // "Function Pal.PalHUDService.CreateDispatchParameterForK2Node"
static UFunction* gFnPush = nullptr; // "Function Pal.PalHUDService.Push"

// -----------------------------------------------------
// Helpers
// -----------------------------------------------------
static inline bool InRangePtr(const void* p) {
    const uintptr_t a = reinterpret_cast<uintptr_t>(p);
    return (a > 0x10000ULL && a < 0x00007FFFFFFFFFFFULL);
}

static UObject* GetSafeUObjectForVtable() {
    if (!UObject::GObjects) return nullptr;
    const int32 n = UObject::GObjects->Num();
    for (int32 i = 0; i < n; ++i) {
        UObject* o = UObject::GObjects->GetByIndex(i);
        if (!o || !o->Class) continue;
        if (!InRangePtr(o->Class)) continue;
        void** vt = *reinterpret_cast<void***>(o);
        if (!vt || !InRangePtr(vt)) continue;
        return o;
    }
    return nullptr;
}

template<typename T = UObject>
static T* FindFirstObjectByFullName(const char* fullName) {
    if (!UObject::GObjects) return nullptr;
    const int32 n = UObject::GObjects->Num();
    for (int32 i = 0; i < n; ++i) {
        UObject* o = UObject::GObjects->GetByIndex(i);
        if (!o) continue;
        try {
            if (o->GetFullName() == fullName)
                return (T*)o;
        }
        catch (...) { /* ignore bad names during GC */ }
    }
    return nullptr;
}

// -----------------------------------------------------
// Params structs (adjust names if SDK differs)
// Inspect your dump for exact names in *_parameters.hpp
// -----------------------------------------------------
struct UPalHUDService_CreateDispatchParameterForK2Node_Params
{
    FName K2NodeName;                                    // confirm in your SDK
    UPalHUDDispatchParameterBase* ReturnValue;           // confirm field name is ReturnValue
};

struct UPalHUDService_Push_Params
{
    UPalHUDDispatchParameterBase* Param;                 // Some dumps call it "DispatchParameter" or similar
    // If your SDK shows extra params, add them here (we only need the first pointer).
};

// Fallback: in case field name differs, treat first 8 bytes as pointer
static UPalHUDDispatchParameterBase* FirstPtr(void* params) {
    if (!params) return nullptr;
    if (!InRangePtr(params)) return nullptr;
    return *reinterpret_cast<UPalHUDDispatchParameterBase**>(params);
}

static void PatchAllWorldMapParamsOnce()
{
    if (!SDK::UObject::GObjects) return;
    int patched = 0;
    for (int i = 0; i < SDK::UObject::GObjects->Num(); ++i)
    {
        auto* o = SDK::UObject::GObjects->GetByIndex(i);
        if (!o || !o->Class) continue;
        if (o->IsA(SDK::UPalHUDDispatchParameter_WorldMap::StaticClass()))
        {
            auto* p = static_cast<SDK::UPalHUDDispatchParameter_WorldMap*>(o);
            p->CanFastTravel = true;
            p->ForRespawn = false;
            p->IsInitSelect = false;
            ++patched;
        }
    }
    printf("[HOOK] Patched %d UPalHUDDispatchParameter_WorldMap instance(s).\n", patched);
}

static void PatchIfWorldMap(UPalHUDDispatchParameterBase* base) {
    if (!base) return;
    UClass* WorldMapParamClass = UPalHUDDispatchParameter_WorldMap::StaticClass();
    if (!WorldMapParamClass) return;

    if (base->IsA(WorldMapParamClass)) {
        auto* wm = static_cast<UPalHUDDispatchParameter_WorldMap*>(base);
        wm->CanFastTravel = true;
        wm->ForRespawn = false;
        wm->IsInitSelect = false;
        // wm->FocusQuestId = FName(); // optional
        printf("[HOOK] Patched WorldMap params: CanFastTravel=1 ForRespawn=0 IsInitSelect=0\n");
    }
}

// -----------------------------------------------------
// ProcessEvent hook
// -----------------------------------------------------
static void hkProcessEvent(SDK::UObject* obj, SDK::UFunction* func, void* params)
{
    // super defensive
    if (!func) { oProcessEvent(obj, func, params); return; }

    bool isMapOnSetup = false;
    try {
        const std::string& full = func->GetFullName(); // Dumper7 returns std::string
        // match the BP function by name ¨C no stored pointers needed
        isMapOnSetup = (full.find("WBP_Map_Base.WBP_Map_Base_C:OnSetup") != std::string::npos);
    }
    catch (...) {}

    if (isMapOnSetup)
    {
        // run original first so the widget finishes its setup
        oProcessEvent(obj, func, params);

        // now flip the map params that were just created
        PatchAllWorldMapParamsOnce();
        return;
    }

    // everything else
    oProcessEvent(obj, func, params);
}

// -----------------------------------------------------
// Install
// -----------------------------------------------------
void InitHooks()
{
    printf("[HOOK] Initializing ProcessEvent hook...\n");

    // Resolve targets
    gFnCreateParam = FindFirstObjectByFullName<UFunction>("Function Pal.PalHUDService.CreateDispatchParameterForK2Node");
    if (gFnCreateParam)
        printf("[HOOK] Found: %s\n", gFnCreateParam->GetFullName().c_str());
    else
        printf("[HOOK] WARN: CreateDispatchParameterForK2Node not found.\n");

    gFnPush = FindFirstObjectByFullName<UFunction>("Function Pal.PalHUDService.Push");
    if (gFnPush)
        printf("[HOOK] Found: %s\n", gFnPush->GetFullName().c_str());
    else
        printf("[HOOK] WARN: HUDService.Push not found.\n");

    if (!gFnCreateParam && !gFnPush) {
        printf("[HOOK] ERROR: No target functions resolved yet. Defer InitHooks.\n");
        return;
    }

    UObject* probe = GetSafeUObjectForVtable();
    if (!probe) {
        printf("[HOOK] ERROR: No safe UObject for vtable. Defer InitHooks.\n");
        return;
    }

    void** vtable = *reinterpret_cast<void***>(probe);
    constexpr int kProcessEventIndex = 0x4B; // same as you used

    if (Hooking::CreateHook(vtable[kProcessEventIndex], &hkProcessEvent, reinterpret_cast<void**>(&oProcessEvent)))
        printf("[HOOK] ProcessEvent hook installed.\n");
    else
        printf("[HOOK] ERROR: Failed to install ProcessEvent hook.\n");
}
