#pragma once
#include <pch.h>
#include <string>
#include <Windows.h>

extern SDK::APalPlayerCharacter* selectedPlayer;
extern SDK::APalWeaponBase* playerWeapon;

void SetInfiniteAmmo();
void ChangeWorldSpeed(float speed);
void SetPlayerAttackParam();
void SetPlayerInventoryWeight();
void SetInfiniteAmmo();
void ResetStamina();
void AddItemToInventoryByName(std::string itemName, int count);
void TeleportPlayerTo(const SDK::FVector& pos);
void SetCameraFov();
void SetCameraBrightness();
void AddWaypointLocation(const std::string& wpName);
bool RemoveWaypointLocationByName(const std::string& wpName);
void IncreaseAllDurability();
void CollectAllRelicsInMap();
void SetWeaponDamage();
void SetInfiniteMagazine();
void TeleportPlayerToHome();
void RevealMapAroundPlayer();
void UnlockAllFastTravelPoints();

//Debug
void DebugBuildOverlap();

//SinglePlayer
void ExploitFly();
void SetPlayerDefenseParam();
void SetCraftingSpeed();
void AddTechPoints();
void AddAncientTechPoints();
void RemoveTechPoints();
void RemoveAncientTechPoint();
void SetPlayerSpeed();
void SetPlayerLevel();

struct BulkItem { std::string id; int count; };

struct CheatState
{ 

    //Features
    float worldSpeed = 1.0f;
    int weaponDamage = 1;
    int attack = 1;
    float weight = 600.0f;
    bool infAmmo = false;
    bool infMag = false;
    bool infStamina = false;
    float cameraFov = 90.0f;
    float cameraBrightness = 1.0f;

    //Aimbot
    bool isSilent = true;
	bool aimbotEnabled = false;
	bool aimbotShowFov = false;
	bool aimbotDrawFOV = false;
	float aimbotFov = 120.0f;
	float aimbotSmooth = 0.1f;
    int aimbotHotkey = VK_MENU;
    bool aimbotVisibilityCheck = true;

    // ESP
    bool espEnabled = true;
    bool espBoxes = false;
    bool espShowNames = true;
    bool espShowDistance = false;
    float espDistance = 10000.0f;
    // ESP Filters
    bool espShowPalHealth = false;
    bool espShowPals = true;
    bool espShowRelics = true;
    bool espShowWaypoints = false;

    //SinglePlayer
    bool isFly = false;
    float craftingSpeed = 1.0f;
    __int32 addTechPoints = 0;
    __int32 addAnchientTechPoints = 0;
    __int32 removeTechPoints = 0;
    __int32 removeAnchientTechPoints = 0;
    int defence = 0;
    float speedMultiplier = 600.0f;
    __int32 playerLevel = 0;
    __int32 palCaptureCount = 0;
    int expToAdd = 0;

    // Misc
    bool showMenu = true;

    std::vector<BulkItem> bulkItems;
};

struct SWaypoint
{
    std::string waypointName;
    SDK::FVector waypointLocation;
    SDK::FRotator waypointRotation;

    SWaypoint(const std::string& name, const SDK::FVector& loc, const SDK::FRotator& rot)
        : waypointName(name), waypointLocation(loc), waypointRotation(rot) {
    }
};

struct Hotkeys
{
    int hotkeyToggleWorldSpeed = VK_F1;
    int hotkeyStamina = VK_F2;
    int hotkeyToggleESP = VK_F3;
    int hotkeyToggleRelic = VK_F4;
    int hotkeyToggleAttack = VK_F5;
    int hotkeyRepairWeapon = VK_F6;
	int hotkeyTeleportHome = VK_F7;
    int hotkeyRefreshWeight = VK_F8;
    int hotkeyFastTravelMap = 0x4D;

    bool worldSpeedToggled = false;
	bool staminaToggled = false;
    bool espToggled = false;
	bool relicToggled = false;
	bool attackToggled = false;
    bool repairWeapon = false;

};

namespace gSilent
{
    inline SDK::APalCharacter* targetPal = nullptr;
}

inline CheatState cheatState;
extern Hotkeys key;
inline std::vector<SWaypoint> g_Waypoints;
