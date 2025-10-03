#include <pch.h>
#include "Tabs.h"
#include "cheat_state.h"
#include "src/ui/imgui_style.h"
#include "ConfigManager.h"

void TabFeatures()
{
    ImVec4 headerColor = ImVec4(1.0f, 0.9f, 0.6f, 1.0f);
    ImGui::Spacing();

    // === PLAYER FEATURES ===
    ColoredSeparatorText("Player Features", headerColor);
    ImGui::BeginGroup();

    if (ImGui::Checkbox("Infinite Stamina", &cheatState.infStamina)) {
        // Call your stamina apply func here if you have one, e.g. SetInfiniteStamina();
        SaveConfig("config.txt");
    }

    ImGui::Spacing();

    // World Speed
    ImGui::SliderFloat("##worldspeed", &cheatState.worldSpeed, 1.0f, 20.0f, "World Speed: %.1f", ImGuiSliderFlags_AlwaysClamp);
    if (ImGui::IsItemDeactivatedAfterEdit()) {
        ChangeWorldSpeed(cheatState.worldSpeed);
        SaveConfig("config.txt");
    }

    // Attack Multiplier
    ImGui::SliderInt("##attackmult", &cheatState.attack, 1, 5000, "Attack: %d");
    if (ImGui::IsItemDeactivatedAfterEdit()) {
        SetPlayerAttackParam();
        SaveConfig("config.txt");
    }

    // Weight
    ImGui::SliderFloat("##weight", &cheatState.weight, 1.0f, 100000.0f, "Weight: %.0f");
    if (ImGui::IsItemDeactivatedAfterEdit()) {
        SetPlayerInventoryWeight();
        SaveConfig("config.txt");
    }

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
    if (ImGui::IsItemDeactivatedAfterEdit()) {
        SetWeaponDamage();
        SaveConfig("config.txt");
    }

    if (ImGui::Button("Inf Durability (Current Weapon)", ImVec2(-1, 0)))
        IncreaseAllDurability();

    if (ImGui::Checkbox("Infinite Ammo", &cheatState.infAmmo)) {
        SetInfiniteAmmo();
        SaveConfig("config.txt");
    }

    if (ImGui::Checkbox("Infinite Magazine", &cheatState.infMag)) {
        SetInfiniteMagazine();
        SaveConfig("config.txt");
    }

    ImGui::EndGroup();

    ImGui::Spacing();
    ImGui::Separator();

    // === CAMERA SETTINGS ===
    ColoredSeparatorText("Camera", headerColor);
    ImGui::BeginGroup();

    ImGui::SliderFloat("##fov", &cheatState.cameraFov, 25.0f, 170.0f, "FOV: %.0f");
    if (ImGui::IsItemDeactivatedAfterEdit()) {
        SetCameraFov();
        SaveConfig("config.txt");
    }

    ImGui::SliderFloat("##brightness", &cheatState.cameraBrightness, 0.0f, 5.0f, "Brightness: %.2f");
    if (ImGui::IsItemDeactivatedAfterEdit()) {
        SetCameraBrightness();
        SaveConfig("config.txt");
    }

    ImGui::EndGroup();
    ImGui::Separator();
}
