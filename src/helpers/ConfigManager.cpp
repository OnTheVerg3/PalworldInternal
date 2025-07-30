#include <pch.h>
#include "cheat_state.h"
#include "ConfigManager.h"



namespace fs = std::filesystem;

void SaveConfig(const std::string& filename)
{
    // Check if the folder exists, if not, create it
    DWORD attribs = GetFileAttributesA("c_settings");
    if (attribs == INVALID_FILE_ATTRIBUTES || !(attribs & FILE_ATTRIBUTE_DIRECTORY))
    {
        CreateDirectoryA("c_settings", NULL);
    }

    // Build full path manually
    std::string fullPath = std::string("c_settings\\") + filename;

    // Create or overwrite file
    std::ofstream file(fullPath, std::ios::out | std::ios::trunc);
    if (!file.is_open())
    {
        std::cerr << "[Error] Could not open " << fullPath << " for writing.\n";
        return;
    }

    // Features
    file << "speed=" << cheatState.speed << "\n";
    file << "worldSpeed=" << cheatState.worldSpeed << "\n";
    file << "infAmmo=" << cheatState.infAmmo << "\n";
    file << "infStamina=" << cheatState.infStamina << "\n";
    file << "godmode=" << cheatState.godmode << "\n";
    file << "cameraFov=" << cheatState.cameraFov << "\n";
    file << "cameraBrightness=" << cheatState.cameraBrightness << "\n";
    file << "craftSpeed=" << cheatState.craftSpeed << "\n";
    file << "palCraftSpeed=" << cheatState.palCraftSpeed << "\n";
    file << "techPoints=" << cheatState.techPoints << "\n";
    file << "aTechPoints=" << cheatState.aTechPoints << "\n";
    file << "weaponName=" << cheatState.weaponName << "\n";
	file << "attack=" << cheatState.attack << "\n";
	file << "defence=" << cheatState.defence << "\n";

    // Aimbot
    file << "aimbotEnabled=" << cheatState.aimbotEnabled << "\n";
    file << "aimbotShowFov=" << cheatState.aimbotShowFov << "\n";
    file << "aimbotDrawFOV=" << cheatState.aimbotDrawFOV << "\n";
    file << "aimbotFov=" << cheatState.aimbotFov << "\n";
    file << "aimbotSmooth=" << cheatState.aimbotSmooth << "\n";
    file << "aimbotHotkey=" << cheatState.aimbotHotkey << "\n";

    // ESP
    file << "espEnabled=" << cheatState.espEnabled << "\n";
    file << "espBoxes=" << cheatState.espBoxes << "\n";
    file << "espShowNames=" << cheatState.espShowNames << "\n";
    file << "espShowDistance=" << cheatState.espShowDistance << "\n";
    file << "espDistance=" << cheatState.espDistance << "\n";
    file << "espShowPalHealth=" << cheatState.espShowPalHealth << "\n";
    file << "espShowPals=" << cheatState.espShowPals << "\n";
    file << "espShowRelics=" << cheatState.espShowRelics << "\n";
    file << "espShowWaypoints=" << cheatState.espShowWaypoints << "\n";

    // Misc
    file << "showMenu=" << cheatState.showMenu << "\n";

    file.close();
}

void ApplyCheatState()
{
    // Features that need functions called
    SetInfiniteAmmo();
    ChangeWorldSpeed(cheatState.worldSpeed);
    SetPlayerAttackParam();
    SetPlayerDefenceParam();
    SetCameraFov();
    SetCameraBrightness();
    SetCraftSpeed();
    SetPalCraftSpeed();

    if (!cheatState.weight == 600)
    {
        SetPlayerInventoryWeight();
    }

    if (!cheatState.speed == 600)
    {
        ChangeSpeed(cheatState.speed);
    }
    
}

void LoadConfig(const std::string& filename)
{
    std::string fullPath = std::string("c_settings\\") + filename;

    // Check if file exists
    DWORD attribs = GetFileAttributesA(fullPath.c_str());
    if (attribs == INVALID_FILE_ATTRIBUTES)
    {
        std::cerr << "[Error] Config file does not exist: " << fullPath << "\n";
        return;
    }

    std::ifstream file(fullPath);
    if (!file.is_open())
    {
        std::cerr << "[Error] Could not open " << fullPath << " for reading.\n";
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        size_t sep = line.find('=');
        if (sep == std::string::npos) continue;

        std::string key = line.substr(0, sep);
        std::string value = line.substr(sep + 1);

        if (key == "speed") cheatState.speed = std::stof(value);
        else if (key == "worldSpeed") cheatState.worldSpeed = std::stof(value);
        else if (key == "infAmmo") cheatState.infAmmo = std::stoi(value);
        else if (key == "infStamina") cheatState.infStamina = std::stoi(value);
        else if (key == "godmode") cheatState.godmode = std::stoi(value);
        else if (key == "cameraFov") cheatState.cameraFov = std::stof(value);
        else if (key == "cameraBrightness") cheatState.cameraBrightness = std::stof(value);
        else if (key == "craftSpeed") cheatState.craftSpeed = std::stof(value);
        else if (key == "palCraftSpeed") cheatState.palCraftSpeed = std::stof(value);
        else if (key == "techPoints") cheatState.techPoints = std::stoi(value);
        else if (key == "aTechPoints") cheatState.aTechPoints = std::stoi(value);
        else if (key == "weaponName") cheatState.weaponName = value;
        else if (key == "attack") cheatState.attack = std::stoi(value);
		else if (key == "defence") cheatState.defence = std::stoi(value);

        // Aimbot
        else if (key == "aimbotEnabled") cheatState.aimbotEnabled = std::stoi(value);
        else if (key == "aimbotShowFov") cheatState.aimbotShowFov = std::stoi(value);
        else if (key == "aimbotDrawFOV") cheatState.aimbotDrawFOV = std::stoi(value);
        else if (key == "aimbotFov") cheatState.aimbotFov = std::stof(value);
        else if (key == "aimbotSmooth") cheatState.aimbotSmooth = std::stof(value);
        else if (key == "aimbotHotkey") cheatState.aimbotHotkey = std::stoi(value);

        // ESP
        else if (key == "espEnabled") cheatState.espEnabled = std::stoi(value);
        else if (key == "espBoxes") cheatState.espBoxes = std::stoi(value);
        else if (key == "espShowNames") cheatState.espShowNames = std::stoi(value);
        else if (key == "espShowDistance") cheatState.espShowDistance = std::stoi(value);
        else if (key == "espDistance") cheatState.espDistance = std::stof(value);
        else if (key == "espShowPalHealth") cheatState.espShowPalHealth = std::stoi(value);
        else if (key == "espShowPals") cheatState.espShowPals = std::stoi(value);
        else if (key == "espShowRelics") cheatState.espShowRelics = std::stoi(value);
        else if (key == "espShowWaypoints") cheatState.espShowWaypoints = std::stoi(value);

        // Misc
        else if (key == "showMenu") cheatState.showMenu = std::stoi(value);
    }

    file.close();

	ApplyCheatState();
}

