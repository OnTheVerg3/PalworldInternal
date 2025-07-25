#pragma once


extern std::vector<SDK::APalPlayerCharacter*> g_PlayerList;
extern SDK::APalPlayerCharacter* selectedPlayer;


void SetInfiniteAmmo(bool bInfAmmo);
void ChangeSpeed(float speed);
void ChangeWorldSpeed(float speed);
void SetPlayerAttackParam();
void SetPlayerDefenceParam();
void SetPlayerInventoryWeight();
void SetInfiniteAmmo();
void ResetStamina();
void AddItemToInventoryByName(std::string itemName, int count);
void TeleportPlayerTo(const SDK::FVector& pos);
void SetDemiGodMode();
void SetCameraFov();
void SetCraftSpeed();
void SetPalCraftSpeed();
void AddTechPoints();
void AddAncientTechPoints();
void AddWaypointLocation(const std::string& wpName);
bool RemoveWaypointLocationByName(const std::string& wpName);
void InfHP();

struct CheatState
{ 
    //Features
    float speed = 600.0f;
    float worldSpeed = 1.0f;
    int attack = 1;
    int defence = 1;
    float weight = 600.0f;
    bool infAmmo = false;
    bool infStamina = false;
    bool godmode = false;
    float cameraFov = 90.0f;
    float craftSpeed = 1.0f;
    float palCraftSpeed = 1.0f;
    __int32 techPoints = 0;
    __int32 aTechPoints = 0;

    //Aimbot
	bool aimbotEnabled = false;
	bool aimbotShowFov = false;
	bool aimbotDrawFOV = false;
	float aimbotFov = 60.0f;
	float aimbotSmooth = 0.2f;
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

    bool worldSpeedToggled = false;
	bool staminaToggled = false;
    bool espToggled = false;
	bool relicToggled = false;
	bool attackToggled = false;

};


inline CheatState cheatState;
extern Hotkeys key;
inline std::vector<SWaypoint> g_Waypoints;
