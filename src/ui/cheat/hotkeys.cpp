#include <pch.h>
#include "hotkeys.h"
#include "cheat_state.h"
#include <Windows.h>

Hotkeys key;

void TickHotkeys()
{
    // World Speed
    if (GetAsyncKeyState(key.hotkeyToggleWorldSpeed) & 1)
    {
        key.worldSpeedToggled = !key.worldSpeedToggled;
        ChangeWorldSpeed(key.worldSpeedToggled ? 10.0f : 1.0f);
    }

    if (GetAsyncKeyState(key.hotkeyStamina) & 1)
    {
        key.staminaToggled = !key.staminaToggled;
		cheatState.infStamina = key.staminaToggled;
    }

    // ESP
    if (GetAsyncKeyState(key.hotkeyToggleESP) & 1)
    {
        key.espToggled = !key.espToggled;
        cheatState.espEnabled = key.espToggled;
        cheatState.espBoxes = key.espToggled;
        cheatState.espShowDistance = key.espToggled;
        cheatState.espShowNames = key.espToggled;
        cheatState.espShowPalHealth = key.espToggled;
        cheatState.espShowPals = key.espToggled;
        cheatState.espShowRelics = key.espToggled;
    }

    if (GetAsyncKeyState(key.hotkeyToggleRelic) & 1)
    {
        key.relicToggled = !key.relicToggled;
        cheatState.espEnabled = key.relicToggled;
        cheatState.espShowRelics = key.relicToggled;
    }

    if (GetAsyncKeyState(key.hotkeyToggleAttack) & 1)
    {
        key.attackToggled = !key.attackToggled;
        if (key.attackToggled)
        {
            cheatState.attack = 90000;
            SetPlayerAttackParam();
        }
        else
        {
            cheatState.attack = 1;
            SetPlayerAttackParam();
        }
		
    } 
}

void TickHotkeysOneShot()
{
    // Repair weapon: only run once per press
    static bool repairKeyDown = false;

    if (GetAsyncKeyState(key.hotkeyRepairWeapon) & 0x8000) // key is held
    {
        if (!repairKeyDown)
        {
            // First frame key is pressed
            repairKeyDown = true;
            IncreaseAllDurability();
        }
    }
    else
    {
        // Key released → reset flag
        repairKeyDown = false;
    }

    if (GetAsyncKeyState(key.hotkeyTeleportHome) & 1)
    {
		TeleportPlayerToHome();
    }
}

void DrawHotkeys()
{
    struct HotkeyEntry {
        const char* label;
        int* keyBinding;
        bool* waitingFlag;
        bool* activeFlag;
    };

    static bool waitingWorld = false, waitingESP = false, waitingStamina = false, waitingRelic = false, waitingAttack = false, waitingRepair = false, 
        waitingTeleportHome = false, waitingFastTravelMap = false;

    HotkeyEntry hotkeys[] = {
    { "World Speed 1:10", &key.hotkeyToggleWorldSpeed, &waitingWorld, &key.worldSpeedToggled },
    { "Inf Stamina", &key.hotkeyStamina, &waitingStamina, &key.staminaToggled },
    { "Pal ESP", &key.hotkeyToggleESP, &waitingESP, &key.espToggled },
    { "Relic ESP", &key.hotkeyToggleRelic, &waitingRelic, &key.relicToggled },
    { "Attack 1:90000", &key.hotkeyToggleAttack, &waitingAttack, &key.attackToggled },
    { "Repair Current Weapon", &key.hotkeyRepairWeapon, &waitingRepair, nullptr },
    { "Teleport Home", &key.hotkeyTeleportHome, &waitingTeleportHome, nullptr },
    };

    for (auto& entry : hotkeys)
    {
        char keyName[64] = "Unknown";
        DWORD scanCode = MapVirtualKeyA(*entry.keyBinding, MAPVK_VK_TO_VSC);
        if (scanCode)
            GetKeyNameTextA(scanCode << 16, keyName, sizeof(keyName));

        ImGui::BeginGroup();

        ImGui::Text("%s", entry.label);

        ImGui::SameLine(250);

        if (entry.activeFlag)
        {
            if (*entry.activeFlag)
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "ON");
            else
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "OFF");
        }
        else
        {
            ImGui::TextDisabled("-");
        }

        // 3. Hotkey button (far right)
        ImGui::SameLine(350); // align further right
        if (*entry.waitingFlag)
        {
            ImGui::Text("Press any key...");
            for (int vk = 0x08; vk <= 0xFE; ++vk)
            {
                if (GetAsyncKeyState(vk) & 0x8000)
                {
                    *entry.keyBinding = vk;
                    *entry.waitingFlag = false;
                    break;
                }
            }
        }
        else
        {
            if (ImGui::Button(keyName, ImVec2(100, 0)))
                *entry.waitingFlag = true;
        }

        ImGui::EndGroup();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextDisabled("Hotkeys work even if menu is closed");
}

