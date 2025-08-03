#pragma once
#include <pch.h>
#include <string>
#include <Windows.h>

extern std::vector<SDK::APalPlayerCharacter*> g_PlayerList;
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
    float cameraBrightness = 0.0f;

    //Aimbot
	bool aimbotEnabled = false;
	bool aimbotShowFov = false;
	bool aimbotDrawFOV = false;
	float aimbotFov = 120.0f;
	float aimbotSmooth = 0.1f;
    int aimbotHotkey = VK_MENU;

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

    // Misc
    bool showMenu = true;
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

    bool worldSpeedToggled = false;
	bool staminaToggled = false;
    bool espToggled = false;
	bool relicToggled = false;
	bool attackToggled = false;
    bool repairWeapon = false;

};

inline CheatState cheatState;
extern Hotkeys key;
inline std::vector<SWaypoint> g_Waypoints;
