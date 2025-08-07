#include <pch.h>
#include "Tabs.h"
#include "cheat_state.h"
#include "database.h"

void TabTeleporter()
{
	static char wpNameBuf[64] = "";

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

	if(ImGui::Button("Teleport Home(F7)"))
	{
		TeleportPlayerToHome();
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
}