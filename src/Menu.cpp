#include <pch.h>
#include <Engine.h>
#include <Menu.h>
#include <map>
#include "ui/fonts.h"
#include <src/ui/imgui_style.h>
#include "ui/cheat/esp.h"
#include "ItemList.hpp"
#include "database.h"  
#include "ui/cheat/aimbot.h"
#include "include/hotkeys.h"

namespace DX11Base
{
	namespace Styles 
    {
        void BaseStyle()
        {
            ImGuiStyle& style = ImGui::GetStyle();
            ImVec4* colors = ImGui::GetStyle().Colors;

            style.WindowTitleAlign = ImVec2(0.5f, 0.5f);

            ImGui::StyleColorsDark();
            SetupImGuiStyle();
        }

        //  Hides the Dear ImGui Navigation Interface ( Windowing Mode ) 
        // @TODO: Disable ImGui Navigation
        void SetNavigationMenuViewState(bool bShow)
        {
            ImVec4* colors = ImGui::GetStyle().Colors;
            switch (bShow)
            {
                case true:
                {
                    //  Show Navigation Panel | Default ImGui Dark Style
                    //  Perhaps just call BaseStyle() ?
                    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
                    colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
                    colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
                    colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
                    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
                    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
                    break;
                }
                case false:
                {
                    //  Hide Navigation Panel
                    colors[ImGuiCol_Text] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
                    colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
                    colors[ImGuiCol_Border] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
                    colors[ImGuiCol_NavHighlight] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
                    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
                    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
                    break;
                }
            }
        }
	}

	namespace Tabs 
    {
        void TABMain()
        {
            ImGui::SetNextWindowSize(ImVec2(620, 700), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("##trainer", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar))
            {
                if (ImGui::BeginTabBar("##tabs"))
                {
                    //ESP Tab
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
                        ImGui::SliderFloat("Distance", &cheatState.espDistance, 200.0f, 30000.0f);

                        ImGui::SeparatorEx(1.0f);
                        ImGui::Text("Filters");

                        ImGui::Checkbox("Show Pals", &cheatState.espShowPals);
                        ImGui::Checkbox("Show Relics", &cheatState.espShowRelics);
                        ImGui::Separator();

                        ImGui::EndTabItem();
                    }

                    //Features Tab
                    if (ImGui::BeginTabItem("Features"))
                    {

                        ImGui::SeparatorEx(1.0f);
                        ImGui::Spacing();

                        ImGui::SeparatorEx(1.0f);
                        ImGui::Text("Online");

                        if (ImGui::SliderFloat("World Speed", &cheatState.worldSpeed, 1.0f, 20.0f))
                        {
                            ChangeWorldSpeed(cheatState.worldSpeed);
                        }
                        if (ImGui::SliderInt("Attack Multiply", &cheatState.attack, 1, 1000))
                        {
                            SetPlayerAttackParam();
                        }
                        if (ImGui::SliderFloat("Weight", &cheatState.weight, 1.0f, 100000.0f, "%.0f"))
                        {
                            SetPlayerInventoryWeight();
                        }
                        ImGui::Checkbox("Inf Stamina", &cheatState.infStamina);

                        if (ImGui::Checkbox("[Inf Ammo] Applied to:", &cheatState.infAmmo))
                        {
                            SetInfiniteAmmo();
                        }
                        ImGui::SameLine();
                        if (cheatState.weaponName == "No Weapon found")
                        {
                            ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "%s", cheatState.weaponName.c_str()); // Red
                        }
                        else
                        {
                            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "%s", cheatState.weaponName.c_str()); // Green
                        }

                        ImGui::Spacing();
                        ImGui::SeparatorEx(1.0f);
                        ImGui::Text("Single Player");

                        if (ImGui::SliderFloat("Movement Speed", &cheatState.speed, 400.0f, 5000.0f))
                        {
                            ChangeSpeed(cheatState.speed);
                        }
                        if (ImGui::SliderInt("Defence Multiply", &cheatState.defence, 1, 1000))
                        {
                            SetPlayerDefenceParam();
                        }
                        if (ImGui::SliderFloat("Craft Speed", &cheatState.craftSpeed, 1.0f, 100.0f, "%.0f"))
                        {
                            SetCraftSpeed();
                        }
                        if (ImGui::SliderFloat("Pal Craft Speed", &cheatState.palCraftSpeed, 1.0f, 100.0f, "%.0f"))
                        {
                            SetPalCraftSpeed();
                        }

                        ImGui::InputInt("Tech Points", &cheatState.techPoints, 1, 50);
                        ImGui::SameLine();
                        if (ImGui::Button("Add"))
                        {
                            AddTechPoints();
                        }
                        ImGui::InputInt("Ancient Points", &cheatState.aTechPoints, 1, 50);
                        ImGui::SameLine();
                        if (ImGui::Button("Add"))
                        {
                            AddAncientTechPoints();
                        }

                        ImGui::Checkbox("Godmode", &cheatState.godmode);

                        ImGui::Spacing();
                        ImGui::SeparatorEx(1.0f);
                        ImGui::Text("Camera");

                        if (ImGui::SliderFloat("Fov", &cheatState.cameraFov, 25.0f, 170.0f, "%.0f"))
                        {
                            SetCameraFov();
                        }

                        if (ImGui::SliderFloat("Brightness", &cheatState.cameraBrightness, 0.0f, 5.0f, "%.2f"))
                        {
                            SetCameraBrightness();
                        }

                        ImGui::EndTabItem();
                    }

                    //Tab3
                    if (ImGui::BeginTabItem("Item Spawner"))
                    {

                        static int selectedCategoryIndex = 0;
                        static char searchBuf[128] = "";
                        static std::string selectedItemID = "";

                        // Generate list of category names
                        std::vector<std::string> categoryNames;
                        for (const auto& [name, _] : itemlist::itemCategories)
                            categoryNames.push_back(name);

                        // Draw category combo box
                        if (ImGui::BeginCombo("Category", categoryNames[selectedCategoryIndex].c_str()))
                        {
                            for (int i = 0; i < categoryNames.size(); ++i)
                            {
                                bool isSelected = (i == selectedCategoryIndex);
                                if (ImGui::Selectable(categoryNames[i].c_str(), isSelected))
                                    selectedCategoryIndex = i;
                                if (isSelected)
                                    ImGui::SetItemDefaultFocus();
                            }
                            ImGui::EndCombo();
                        }

                        // Filter input
                        ImGui::InputText("Search", searchBuf, IM_ARRAYSIZE(searchBuf));

                        // Get the selected category list
                        const auto& selectedList = *itemlist::itemCategories.at(categoryNames[selectedCategoryIndex]);

                        // Show filtered list
                        ImGui::BeginChild("ItemList", ImVec2(0, 300), true);
                        for (const auto& entry : selectedList)
                        {
                            std::string str = entry;
                            auto sep = str.find('|');
                            std::string id = str.substr(0, sep);
                            std::string label = (sep != std::string::npos) ? str.substr(sep + 1) : id;

                            // Search filter
                            if (strstr(id.c_str(), searchBuf) || strstr(label.c_str(), searchBuf))
                            {
                                if (ImGui::Selectable(label.c_str(), selectedItemID == id))
                                    selectedItemID = id;
                            }
                        }
                        ImGui::EndChild();

                        static int spawnCount = 1;
                        // Spawn button
                        if (!selectedItemID.empty())
                        {
                            ImGui::Text("Selected: %s", selectedItemID.c_str());

                            ImGui::InputInt("Count", &spawnCount);

                            // Optional: Clamp count to safe values
                            if (spawnCount < 1) spawnCount = 1;
                            if (spawnCount > 9999) spawnCount = 9999;

                            if (ImGui::Button("Spawn Item"))
                            {
                                AddItemToInventoryByName(selectedItemID, spawnCount);
                            }
                        }
                        ImGui::EndTabItem();
                    }

                    static char wpNameBuf[64] = "";

                    if (ImGui::BeginTabItem("Teleporter"))
                    {
                        ImGui::Separator();
                        ImGui::Text("Custom Waypoints:");

                        // Add waypoint input field + button
                        ImGui::InputText("##wpInput", wpNameBuf, IM_ARRAYSIZE(wpNameBuf));
                        ImGui::SameLine();
                        if (ImGui::Button("Add Waypoint"))
                        {
                            if (strlen(wpNameBuf) > 0)
                            {
                                AddWaypointLocation(wpNameBuf);
                                wpNameBuf[0] = '\0'; // Clear input after adding
                            }
                        }

                        // Show saved waypoints
                        for (int i = 0; i < g_Waypoints.size(); ++i)
                        {
                            auto& wp = g_Waypoints[i];

                            ImGui::Text("%s", wp.waypointName.c_str());
                            ImGui::SameLine();

                            std::string teleportBtn = "TP##" + std::to_string(i);
                            if (ImGui::Button(teleportBtn.c_str()))
                            {
                                TeleportPlayerTo(wp.waypointLocation);
                            }

                            ImGui::SameLine();
                            std::string removeBtn = "Remove##" + std::to_string(i);
                            if (ImGui::Button(removeBtn.c_str()))
                            {
                                RemoveWaypointLocationByName(wp.waypointName);
                                break; // Safe break after modifying the vector
                            }
                        }



                        ImGui::Text("Select a boss to teleport:");

                        ImGui::Separator();
                        ImGui::Spacing();

                        int count = 0;
                        for (const auto& [name, coords] : database::locationMap)
                        {
                            if (ImGui::Button(name.c_str(), ImVec2(200, 0)))
                            {
                                if (coords.size() == 3)
                                {
                                    SDK::FVector target(coords[0], coords[1], coords[2]);
                                    TeleportPlayerTo(target);
                                }
                            }

                            // Only do SameLine every 2 buttons
                            if (++count % 2 != 0)
                                ImGui::SameLine();
                        }

                        ImGui::EndTabItem();
                    }

					//Settings Tab
                    if (ImGui::BeginTabItem("Settings"))
                    {
                        DrawHotkeys();
                        ImGui::EndTabItem();
                    }

                    
                    ImGui::EndTabBar();
                }
            }

            if (ImGui::Button("UNHOOK", ImVec2(ImGui::GetContentRegionAvail().x, 20))) {
#if CONSOLE_OUTPUT
                g_Console->printdbg("\n\n[+] UNHOOK INITIALIZED\n\n", Console::Colors::red);
#endif
                g_KillSwitch = TRUE;
            }
            
            ImGui::End();
        }
	}

    //----------------------------------------------------------------------------------------------------
    //										MENU
    //-----------------------------------------------------------------------------------
	void Menu::Draw()
	{
        if (g_Engine->bShowMenu)
            MainMenu();

        ResetStamina();
        SetDemiGodMode();
        DrawPalESP();
        DrawRelicESP();
        TickHotkeys();

        if (cheatState.aimbotEnabled && (GetAsyncKeyState(cheatState.aimbotHotkey) & 0x8000))
        {
            RunPalAimbot();
        }
		//Draw aimbot FOV circle if enabled
        if (cheatState.aimbotEnabled && cheatState.aimbotDrawFOV)
        {
            ImVec2 screenCenter = ImVec2(ImGui::GetIO().DisplaySize.x / 2.0f,
                ImGui::GetIO().DisplaySize.y / 2.0f);

            ImDrawList* drawList = ImGui::GetBackgroundDrawList();
            drawList->AddCircle(
                screenCenter,
                cheatState.aimbotFov,
                IM_COL32(180, 180, 180, 180), // Yellow-ish, semi-transparent
                64,                         // Segments (smoother circle)
                2.0f                        // Thickness
            );
        }

        if (g_Engine->bShowHud && !g_Engine->bShowMenu)
        {
            Styles::SetNavigationMenuViewState(false);
            Menu::HUD();
        }

        if (g_Engine->bShowDemoWindow && g_Engine->bShowMenu)
            ImGui::ShowDemoWindow();

        if (g_Engine->bShowStyleEditor && g_Engine->bShowMenu)
            ImGui::ShowStyleEditor();
	}

	void Menu::MainMenu()
	{
        if (!g_Engine->bShowDemoWindow && !g_Engine->bShowStyleEditor)
            Styles::BaseStyle();

        if (!ImGui::Begin("2", &g_Engine->bShowMenu, 96))
        {
            ImGui::End();
            return;
        }
        
        Tabs::TABMain();

        ImGui::End();
	}

	void Menu::HUD()
	{
        ImVec2 draw_size = g_D3D11Window->pViewport->WorkSize;
        ImVec2 draw_pos = g_D3D11Window->pViewport->WorkSize;
        ImGui::SetNextWindowPos(draw_pos);
        ImGui::SetNextWindowSize(draw_size);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.00f, 0.00f, 0.00f, 0.00f));
        if (!ImGui::Begin("##HUDWINDOW", (bool*)true, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs))
        {
            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
            ImGui::End();
            return;
        }
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();

        ImDrawList* ImDraw = ImGui::GetWindowDrawList();
        auto center = ImVec2({ draw_size.x * .5f, draw_size.y * .5f });
        auto top_center = ImVec2({ draw_size.x * .5f, draw_size.y * 0.0f });
        ImDraw->AddText(top_center, ImColor(1.0f, 1.0f, 1.0f, 1.0f), "https://github.com/NightFyre/DX11-ImGui-Internal-Hook");

        ImGui::End();
	}

	void Menu::Loops()
	{

	}

    //----------------------------------------------------------------------------------------------------
    //										GUI
    //-----------------------------------------------------------------------------------

    void GUI::TextCentered(const char* pText)
    {
        ImVec2 textSize = ImGui::CalcTextSize(pText);
        ImVec2 windowSize = ImGui::GetWindowSize();
        ImVec2 textPos = ImVec2((windowSize.x - textSize.x) * 0.5f, (windowSize.y - textSize.y) * 0.5f);
        ImGui::SetCursorPos(textPos);
        ImGui::Text("%s", pText);
    }

    //  @ATTN: max buffer is 256chars
    void GUI::TextCenteredf(const char* pText, ...)
    {
        va_list args;
        va_start(args, pText);
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), pText, args);
        va_end(args);

        TextCentered(buffer);
    }

    void GUI::DrawText_(ImVec2 pos, ImColor color, const char* pText, float fontSize)
    {
        ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), fontSize, pos, color, pText, pText + strlen(pText), 800, 0);
    }

    //  @ATTN: max buffer is 256chars
    void GUI::DrawTextf(ImVec2 pos, ImColor color, const char* pText, float fontSize, ...)
    {
        va_list args;
        va_start(args, fontSize);
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), pText, args);
        va_end(args);

        DrawText_(pos, color, buffer, fontSize);
    }

    void GUI::DrawTextCentered(ImVec2 pos, ImColor color, const char* pText, float fontSize)
    {
        float textSize = ImGui::CalcTextSize(pText).x;
        ImVec2 textPosition = ImVec2(pos.x - (textSize * 0.5f), pos.y);
        DrawText_(textPosition, color, pText, fontSize);
    }

    //  @ATTN: max buffer is 256chars
    void GUI::DrawTextCenteredf(ImVec2 pos, ImColor color, const char* pText, float fontSize, ...)
    {
        va_list args;
        va_start(args, fontSize);
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), pText, args);
        va_end(args);

        DrawTextCentered(pos, color, pText, fontSize);
    }
}