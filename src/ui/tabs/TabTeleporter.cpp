#include <pch.h>
#include <algorithm>
#include "Tabs.h"
#include "cheat_state.h"
#include "database.h"
#include "src/ui/imgui_style.h"

void TabTeleporter()
{
    static char wpNameBuf[64] = "";
    static char bossSearchBuf[64] = "";

    ImVec4 headerColor = ImVec4(1.0f, 0.9f, 0.6f, 1.0f);

    // === CUSTOM WAYPOINTS ===
    ColoredSeparatorText("Custom Waypoints", headerColor);
    ImGui::Spacing();

    // Input field and Add button
    ImGui::SetNextItemWidth(200);
    ImGui::InputTextWithHint("##wpInput", "Waypoint name", wpNameBuf, IM_ARRAYSIZE(wpNameBuf));
    ImGui::SameLine();
    if (ImGui::Button("Add Waypoint"))
    {
        if (strlen(wpNameBuf) > 0)
        {
            AddWaypointLocation(wpNameBuf);
            wpNameBuf[0] = '\0'; // Clear input
        }
    }

    ImGui::Spacing();
    if (ImGui::Button("Teleport Home (F7)", ImVec2(-1, 0)))
    {
        TeleportPlayerToHome();
    }

    ImGui::Spacing();
    ImGui::BeginChild("WaypointList", ImVec2(0, 120), true);
    for (int i = 0; i < g_Waypoints.size(); ++i)
    {
        auto& wp = g_Waypoints[i];

        ImGui::Text("%s", wp.waypointName.c_str());
        ImGui::SameLine();

        std::string tpBtn = "Teleport##" + std::to_string(i);
        if (ImGui::Button(tpBtn.c_str()))
        {
            TeleportPlayerTo(wp.waypointLocation);
        }

        ImGui::SameLine();
        std::string delBtn = "Delete##" + std::to_string(i);
        if (ImGui::Button(delBtn.c_str()))
        {
            RemoveWaypointLocationByName(wp.waypointName);
            break;
        }
    }
    ImGui::EndChild();

    ImGui::Spacing();
    ImGui::Separator();

    // === BOSS TELEPORTER ===
    ColoredSeparatorText("Boss Teleport Locations", headerColor);

    ImGui::SetNextItemWidth(-1);
    ImGui::InputTextWithHint("##bossSearch", "Search for boss name...", bossSearchBuf, IM_ARRAYSIZE(bossSearchBuf));

    ImGui::Spacing();
    ImGui::BeginChild("BossGrid", ImVec2(0, 330), true);

    int columns = 3;
    int count = 0;

    for (const auto& [name, coords] : database::locationMap)
    {
        std::string lowerName = name;
        std::string lowerSearch = bossSearchBuf;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        std::transform(lowerSearch.begin(), lowerSearch.end(), lowerSearch.begin(), ::tolower);

        if (!lowerSearch.empty() && lowerName.find(lowerSearch) == std::string::npos)
            continue;

        if (ImGui::Button(name.c_str(), ImVec2(200, 32)))
        {
            if (coords.size() == 3)
            {
                SDK::FVector target(coords[0], coords[1], coords[2]);
                TeleportPlayerTo(target);
            }
        }

        if (++count % columns != 0)
            ImGui::SameLine();
    }

    ImGui::EndChild();
}
