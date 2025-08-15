#include <pch.h>
#include "Tabs.h"
#include "cheat_state.h"
#include "src/ui/imgui_style.h"

void TabAimbotESP()
{
	ImGui::Spacing();

	// ===== Aimbot Section =====
	ImGui::BeginGroup();
	ColoredSeparatorText("Aimbot", ImVec4(1.0f, 0.9f, 0.6f, 1.0f));

	ImGui::Checkbox("Enable Aimbot", &cheatState.aimbotEnabled);

	// Hotkey selector
	static bool waitingForKey = false;
	char keyName[64] = "RMB";
	DWORD scanCode = MapVirtualKeyA(cheatState.aimbotHotkey, MAPVK_VK_TO_VSC);
	if (scanCode) GetKeyNameTextA(scanCode << 16, keyName, sizeof(keyName));

	ImGui::Text("Hotkey:"); 
	ImGui::SameLine();
	if (ImGui::Button(keyName, ImVec2(80, 0)))
		waitingForKey = true;

	if (waitingForKey)
	{
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Press a key...");
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
	ImGui::Checkbox("Visibility Check", &cheatState.aimbotVisibilityCheck);
	ImGui::SliderFloat("##aimfov", &cheatState.aimbotFov, 1.0f, 280.0f, "FOV: %.0f");
	ImGui::SliderFloat("##aimsmooth", &cheatState.aimbotSmooth, 0.0f, 1.0f, "Smooth: %.2f");
	ImGui::EndGroup();

	ImGui::Spacing();
	ImGui::Separator();

	// ===== ESP Section =====
	ImGui::BeginGroup();
	ColoredSeparatorText("ESP", ImVec4(1.0f, 0.9f, 0.6f, 1.0f));

	ImGui::Checkbox("Enable ESP", &cheatState.espEnabled);
	ImGui::Checkbox("Draw Boxes", &cheatState.espBoxes);
	ImGui::Checkbox("Show Names", &cheatState.espShowNames);
	ImGui::Checkbox("Show Distance", &cheatState.espShowDistance);
	ImGui::Checkbox("Show Health", &cheatState.espShowPalHealth);
	ImGui::SliderFloat("##espdist", &cheatState.espDistance, 200.0f, 40000.0f, "Distance: %.0f");
	ImGui::EndGroup();

	ImGui::Spacing();
	ImGui::Separator();

	// ===== Filters Section =====
	ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.6f, 1.0f), "ESP Filters");

	ImGui::Columns(2, nullptr, false);
	ImGui::Checkbox("Show Pals", &cheatState.espShowPals);
	ImGui::NextColumn();
	ImGui::Checkbox("Show Relics", &cheatState.espShowRelics);
	ImGui::Columns(1);

	ImGui::Separator();
}
