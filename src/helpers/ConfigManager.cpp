#include <pch.h>
#include "cheat_state.h"
#include "ConfigManager.h"

#include <windows.h>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

// Get this DLL's directory (no need to touch DllMain)
extern "C" IMAGE_DOS_HEADER __ImageBase;
static std::string GetDllDirA() {
    char path[MAX_PATH];
    GetModuleFileNameA((HINSTANCE)&__ImageBase, path, MAX_PATH);
    return fs::path(path).parent_path().string();
}

void SaveConfig(const std::string& filename)
{
    const std::string baseDir = GetDllDirA();
    const std::string cfgDir = baseDir + "\\c_settings";

    // Ensure folder exists
    DWORD attribs = GetFileAttributesA(cfgDir.c_str());
    if (attribs == INVALID_FILE_ATTRIBUTES || !(attribs & FILE_ATTRIBUTE_DIRECTORY)) {
        CreateDirectoryA(cfgDir.c_str(), NULL);
    }

    // Build full path next to the DLL
    const std::string fullPath = cfgDir + "\\" + filename;

    // Create or overwrite file
    std::ofstream file(fullPath, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "[Error] Could not open " << fullPath << " for writing.\n";
        return;
    }

    // Features
    file << "worldSpeed=" << cheatState.worldSpeed << "\n";
    file << "infAmmo=" << cheatState.infAmmo << "\n";
    file << "infStamina=" << cheatState.infStamina << "\n";
    file << "cameraFov=" << cheatState.cameraFov << "\n";
    file << "cameraBrightness=" << cheatState.cameraBrightness << "\n";
    file << "attack=" << cheatState.attack << "\n";
    file << "weight=" << cheatState.weight << "\n";

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

    // Bulk Item Spawner
    for (const auto& bi : cheatState.bulkItems) {
        file << "bulkItem=" << bi.id << "," << bi.count << "\n";
    }
}

void ApplyCheatState()
{
    // Features that need functions called
    SetInfiniteAmmo();
    ChangeWorldSpeed(cheatState.worldSpeed);
    SetPlayerAttackParam();
    SetCameraFov();
    SetCameraBrightness();

    if (cheatState.weight != 600.0f) {
        SetPlayerInventoryWeight();
    }
}

void LoadConfig(const std::string& filename)
{
    const std::string baseDir = GetDllDirA();
    const std::string fullPath = baseDir + "\\c_settings\\" + filename;

    // Check if file exists
    DWORD attribs = GetFileAttributesA(fullPath.c_str());
    if (attribs == INVALID_FILE_ATTRIBUTES) {
        std::cerr << "[Error] Config file does not exist: " << fullPath << "\n";
        return;
    }

    std::ifstream file(fullPath);
    if (!file.is_open()) {
        std::cerr << "[Error] Could not open " << fullPath << " for reading.\n";
        return;
    }

    cheatState.bulkItems.clear();

    std::string line;
    while (std::getline(file, line)) {
        const size_t sep = line.find('=');
        if (sep == std::string::npos) continue;

        const std::string key = line.substr(0, sep);
        const std::string value = line.substr(sep + 1);

        // Features
        if (key == "worldSpeed")        cheatState.worldSpeed = std::stof(value);
        else if (key == "infAmmo")           cheatState.infAmmo = std::stoi(value);
        else if (key == "infStamina")        cheatState.infStamina = std::stoi(value);
        else if (key == "cameraFov")         cheatState.cameraFov = std::stof(value);
        else if (key == "cameraBrightness")  cheatState.cameraBrightness = std::stof(value);
        else if (key == "attack")            cheatState.attack = std::stoi(value);
        else if (key == "weight")            cheatState.weight = std::stof(value);

        // Aimbot
        else if (key == "aimbotEnabled")     cheatState.aimbotEnabled = std::stoi(value);
        else if (key == "aimbotShowFov")     cheatState.aimbotShowFov = std::stoi(value);
        else if (key == "aimbotDrawFOV")     cheatState.aimbotDrawFOV = std::stoi(value);
        else if (key == "aimbotFov")         cheatState.aimbotFov = std::stof(value);
        else if (key == "aimbotSmooth")      cheatState.aimbotSmooth = std::stof(value);
        else if (key == "aimbotHotkey")      cheatState.aimbotHotkey = std::stoi(value);

        // ESP
        else if (key == "espEnabled")        cheatState.espEnabled = std::stoi(value);
        else if (key == "espBoxes")          cheatState.espBoxes = std::stoi(value);
        else if (key == "espShowNames")      cheatState.espShowNames = std::stoi(value);
        else if (key == "espShowDistance")   cheatState.espShowDistance = std::stoi(value);
        else if (key == "espDistance")       cheatState.espDistance = std::stof(value);
        else if (key == "espShowPalHealth")  cheatState.espShowPalHealth = std::stoi(value);
        else if (key == "espShowPals")       cheatState.espShowPals = std::stoi(value);
        else if (key == "espShowRelics")     cheatState.espShowRelics = std::stoi(value);
        else if (key == "espShowWaypoints")  cheatState.espShowWaypoints = std::stoi(value);

        // Misc
        else if (key == "showMenu")          cheatState.showMenu = std::stoi(value);

        // Bulk ItemSpawn: value = "id,count"
        else if (key == "bulkItem") {
            const size_t comma = value.find(',');
            if (comma != std::string::npos) {
                const std::string id = value.substr(0, comma);
                int count = std::atoi(value.substr(comma + 1).c_str());
                if (count < 1) count = 1;
                cheatState.bulkItems.push_back({ id, count });
            }
        }
    }

    ApplyCheatState();
}
