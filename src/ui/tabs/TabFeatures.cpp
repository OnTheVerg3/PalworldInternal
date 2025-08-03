#include <pch.h>
#include "Tabs.h"
#include "cheat_state.h"

void TabFeatures()
{
	ImGui::SeparatorEx(1.0f);
	ImGui::Spacing();
	ImGui::SeparatorText("Player Features");

	if (ImGui::SliderFloat("World Speed", &cheatState.worldSpeed, 1.0f, 20.0f))
	{
		ChangeWorldSpeed(cheatState.worldSpeed);
	}
	if (ImGui::SliderInt("Attack Multiplier", &cheatState.attack, 1, 1000))
	{
		SetPlayerAttackParam();
	}
	if (ImGui::SliderFloat("Weight", &cheatState.weight, 1.0f, 100000.0f, "%.0f"))
	{
		SetPlayerInventoryWeight();
	}
	ImGui::Checkbox("Inf Stamina", &cheatState.infStamina);

	ImGui::Spacing();

	if (ImGui::Button("Collect Relic/Lifemunk"))
	{
		CollectAllRelicsInMap();
	}

	ImGui::Spacing();
	ImGui::SeparatorText("Weapon Features");
	ImGui::Spacing();

	ImGui::TextDisabled("Increases damage against resources (trees, stones, etc.)");

	// Slider
	if (ImGui::SliderInt("Weapon Damage Multiplier", &cheatState.weaponDamage, 1, 400))
	{
		SetWeaponDamage();
	}

	// Buttons below
	if (ImGui::Button("Add Inf Durability Current Weapon", ImVec2(-1, 0)))
	{
		IncreaseAllDurability();
	}


	if (ImGui::Checkbox("Infinite Ammo", &cheatState.infAmmo))
	{
		SetInfiniteAmmo();
	}


	if (ImGui::Checkbox("Infinite Magazine", &cheatState.infMag))
	{
		SetInfiniteMagazine();
	}

	ImGui::Spacing();
	ImGui::SeparatorEx(1.0f);
	ImGui::SeparatorText("Camera");

	if (ImGui::SliderFloat("Fov", &cheatState.cameraFov, 25.0f, 170.0f, "%.0f"))
	{
		SetCameraFov();
	}

	if (ImGui::SliderFloat("Brightness", &cheatState.cameraBrightness, 0.0f, 5.0f, "%.2f"))
	{
		SetCameraBrightness();
	}
}