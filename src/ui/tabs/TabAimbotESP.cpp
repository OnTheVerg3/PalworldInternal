#include <pch.h>
#include "Tabs.h"
#include "cheat_state.h"

void TabAimbotESP()
{
    if (ImGui::BeginTabItem("Aimbot & ESP"))
    {
        ImGui::Spacing();
        ImGui::Checkbox("Enable aimbot", &cheatState.aimbotEnabled);

        static bool waitingForKey = false;

        // Get key name
        char keyName[64] = "RMB";
        DWORD scanCode = MapVirtualKeyA(cheatState.aimbotHotkey, MAPVK_VK_TO_VSC);
        if (scanCode)
            GetKeyNameTextA(scanCode << 16, keyName, sizeof(keyName));

        // Display key selector
        ImGui::Text("Aimbot Hotkey: ");
        ImGui::SameLine();
        if (ImGui::Button(keyName))
            waitingForKey = true;

        // Wait for new key press
        if (waitingForKey)
        {
            ImGui::Text("Press any key...");
            for (int vk = 0x01; vk <= 0xFE; ++vk)
            {
                if (GetAsyncKeyState(vk) & 0x8000)
                {
                    cheatState.aimbotHotkey = vk;
                    waitingForKey = false;
                    break;
                }
            }
        }
        ImGui::Checkbox("Draw FOV", &cheatState.aimbotDrawFOV);
        ImGui::SliderFloat("Aimbot FOV", &cheatState.aimbotFov, 1.0f, 280.0f);
        ImGui::SliderFloat("Aimbot Smooth", &cheatState.aimbotSmooth, 0.0f, 1.0f);

        ImGui::SeparatorEx(1.0f);

        ImGui::Checkbox("Enable ESP", &cheatState.espEnabled);
        ImGui::SeparatorEx(1.0f);

        ImGui::Checkbox("Draw Boxes", &cheatState.espBoxes);
        ImGui::Checkbox("Show Names", &cheatState.espShowNames);
        ImGui::Checkbox("Show Distance", &cheatState.espShowDistance);
        ImGui::Checkbox("Show Health", &cheatState.espShowPalHealth);
        ImGui::SliderFloat("Distance", &cheatState.espDistance, 200.0f, 40000.0f);

        ImGui::SeparatorEx(1.0f);
        ImGui::Text("Filters");

        ImGui::Checkbox("Show Pals", &cheatState.espShowPals);
        ImGui::Checkbox("Show Relics", &cheatState.espShowRelics);
        ImGui::Separator();

        ImGui::EndTabItem();
    }
}