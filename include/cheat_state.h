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

    // ESP
    bool espEnabled = false;
    bool espBoxes = false;
    bool espShowNames = false;
    bool espShowDistance = false;
    float espDistance = 5000.0f;
    // ESP Filters
    bool espShowJunk = false;
    bool espShowPals = true;
    bool espShowPickupItems = true;
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

inline CheatState cheatState;
inline std::vector<SWaypoint> g_Waypoints;
