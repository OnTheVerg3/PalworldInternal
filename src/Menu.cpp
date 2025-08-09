#include <pch.h>
#include <Engine.h>
#include <Menu.h>
#include <map>
#include "ui/fonts.h"
#include <src/ui/imgui_style.h>
#include "ui/cheat/esp.h"
#include "ui/cheat/aimbot.h"
#include "include/hotkeys.h"
#include "ConfigManager.h"
#include "Tabs.h"
#include "pal_editor.h"

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
            static int selectedMenu = 0;
            const char* menuItems[] = {
                "Aimbot & ESP",
                "Online Features",
                "Single Player",
                "Pal Editor",
                "Item Spawner",
                "Teleporter",
                "Settings",
                "Changelog"
            };

            ImGui::SetNextWindowSize(ImVec2(950, 700), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowPos(ImVec2(80, 60), ImGuiCond_FirstUseEver);

            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 12));

            if (ImGui::Begin("##main", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar))
            {
                // Header
                ImGui::TextColored(ImVec4(0.85f, 0.95f, 1.0f, 1.0f), "Palworld Trainer v1.5");
                ImGui::Spacing();
                ImGui::Separator();

                ImGui::Columns(2, nullptr, false);
                ImGui::SetColumnWidth(0, 180);

                // ==== SIDEBAR ====
                ImGui::BeginChild("Sidebar", ImVec2(0, 0), true);

                for (int i = 0; i < IM_ARRAYSIZE(menuItems); ++i)
                {
                    if (CustomButton(menuItems[i], ImVec2(-1, 36), selectedMenu == i))
                        selectedMenu = i;
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // Exit button with red theme
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.75f, 0.1f, 0.1f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));

                if (ImGui::Button("EXIT", ImVec2(-1, 32)))
                    g_KillSwitch = TRUE;

                ImGui::PopStyleColor(3);
                ImGui::EndChild();
                ImGui::NextColumn();

                // ==== CONTENT ====
                ImGui::BeginChild("Content", ImVec2(0, 0), true);

                switch (selectedMenu)
                {
                case 0: TabAimbotESP(); break;
                case 1: TabFeatures(); break;
                case 2: TabSinglePlayer(); break;
                case 3:
                    cachedTamedPals.clear();
                    cachedBaseWorkers.clear();
                    GetAllTamedPals(cachedTamedPals);
                    GetAllBaseWorkers(cachedBaseWorkers);
                    TabPalEditor();
                    break;
                case 4: TabItemSpawner(); break;
                case 5: TabTeleporter(); break;
                case 6:
                    DrawHotkeys();
                    ImGui::Separator();
                    if (ImGui::Button("Save Config")) SaveConfig("config.txt");
                    ImGui::SameLine();
                    if (ImGui::Button("Load Config")) LoadConfig("config.txt");
                    break;
                case 7: TabChangeLog(); break;
                
                }

                ImGui::EndChild();
                ImGui::Columns(1);
            }

            ImGui::End();
            ImGui::PopStyleVar(3);
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
        TickHotkeys();
        TickHotkeysOneShot();
        FastTravelRetryStep();

        if (cheatState.aimbotEnabled && (GetAsyncKeyState(cheatState.aimbotHotkey) & 0x8000))
        {
            RunPalAimbot();
        }

        DrawPalESP();
        DrawRelicESP();

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

        Tabs::TABMain();

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