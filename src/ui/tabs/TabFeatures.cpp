#include <pch.h>
#include "Tabs.h"
#include "cheat_state.h"
#include "src/ui/imgui_style.h"

void TabFeatures()
{
	ImVec4 headerColor = ImVec4(1.0f, 0.9f, 0.6f, 1.0f);
	ImGui::Spacing();

	// === PLAYER FEATURES ===
	ColoredSeparatorText("Player Features", headerColor);
	ImGui::BeginGroup();

	ImGui::Checkbox("Infinite Stamina", &cheatState.infStamina);
	ImGui::Spacing();

	// World Speed
	ImGui::SliderFloat("##worldspeed", &cheatState.worldSpeed, 1.0f, 20.0f, "World Speed: %.1f", ImGuiSliderFlags_AlwaysClamp);
	if (ImGui::IsItemDeactivatedAfterEdit()) ChangeWorldSpeed(cheatState.worldSpeed);

	// Attack Multiplier
	ImGui::SliderInt("##attackmult", &cheatState.attack, 1, 5000, "Attack: %d");
	if (ImGui::IsItemDeactivatedAfterEdit()) SetPlayerAttackParam();

	// Weight
	ImGui::SliderFloat("##weight", &cheatState.weight, 1.0f, 100000.0f, "Weight: %.0f");
	if (ImGui::IsItemDeactivatedAfterEdit()) SetPlayerInventoryWeight();
	ImGui::SameLine();

	ImGui::Spacing();

	if (ImGui::Button("Collect Relics/Lifemunks", ImVec2(-1, 0)))
		CollectAllRelicsInMap();

	if (ImGui::Button("Reveal Map [Requires Rejoin Server]", ImVec2(-1, 0)))
		RevealMapAroundPlayer();

	ImGui::EndGroup();

	ImGui::Spacing();
	ImGui::Separator();

	// === WEAPON FEATURES ===
	ColoredSeparatorText("Weapon Features", headerColor);
	ImGui::BeginGroup();

	ImGui::TextDisabled("Increases damage to trees, rocks, etc.");

	ImGui::SliderInt("##weaponmult", &cheatState.weaponDamage, 1, 400, "Weapon DMG: %d");
	if (ImGui::IsItemDeactivatedAfterEdit()) SetWeaponDamage();
	ImGui::SameLine();

	if (ImGui::Button("Inf Durability (Current Weapon)", ImVec2(-1, 0)))
		IncreaseAllDurability();

	if (ImGui::Checkbox("Infinite Ammo", &cheatState.infAmmo))
		SetInfiniteAmmo();

	if (ImGui::Checkbox("Infinite Magazine", &cheatState.infMag))
		SetInfiniteMagazine();

	ImGui::EndGroup();

	ImGui::Spacing();
	ImGui::Separator();

	// === CAMERA SETTINGS ===
	ColoredSeparatorText("Camera", headerColor);
	ImGui::BeginGroup();

	ImGui::SliderFloat("##fov", &cheatState.cameraFov, 25.0f, 170.0f, "FOV: %.0f");
	if (ImGui::IsItemDeactivatedAfterEdit()) SetCameraFov();

	ImGui::SliderFloat("##brightness", &cheatState.cameraBrightness, 0.0f, 5.0f, "Brightness: %.2f");
	if (ImGui::IsItemDeactivatedAfterEdit()) SetCameraBrightness();

	ImGui::EndGroup();
	ImGui::Separator();
}
