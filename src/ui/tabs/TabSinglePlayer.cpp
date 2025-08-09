#include <pch.h>
#include "Tabs.h"
#include "cheat_state.h"
#include "src/ui/imgui_style.h"

void TabSinglePlayer()
{
    ImVec4 headerColor = ImVec4(1.0f, 0.9f, 0.6f, 1.0f);
    ImGui::Spacing();
    ColoredSeparatorText("Single Player Features", headerColor);

    // === Fly ===
    ImGui::Checkbox("Fly", &cheatState.isFly);
    if (ImGui::IsItemDeactivatedAfterEdit())
        ExploitFly();

    ImGui::Spacing();

    ImGui::SliderInt("##playerlevel", &cheatState.playerLevel, 1, 100, "Level: %d");
    if (ImGui::IsItemDeactivatedAfterEdit())
        SetPlayerLevel();

    ImGui::SliderFloat("##speedmult", &cheatState.speedMultiplier, 100.0f, 5000.0f, "Speed: %.0f", ImGuiSliderFlags_AlwaysClamp);
    if (ImGui::IsItemDeactivatedAfterEdit())
        SetPlayerSpeed();

    ImGui::Spacing();

    // === Defense ===
    ImGui::SliderInt("##defense", &cheatState.defence, 0, 5000, "Defense: %d");
    if (ImGui::IsItemDeactivatedAfterEdit())
        SetPlayerDefenseParam();

    ImGui::Spacing();

    // === Crafting Speed ===
    ImGui::SliderFloat("##craftspeed", &cheatState.craftingSpeed, 0.1f, 100.0f, "Craft Speed: %.1f", ImGuiSliderFlags_AlwaysClamp);
    if (ImGui::IsItemDeactivatedAfterEdit())
        SetCraftingSpeed();

    ImGui::Spacing();
    ColoredSeparatorText("Technology Points", headerColor);

    // === Add Tech Points ===
    ImGui::InputInt("Add Tech Points", &cheatState.addTechPoints);
    ImGui::SameLine();
    if (ImGui::Button("Add"))
        AddTechPoints();

    // === Add Ancient Tech Points ===
    ImGui::InputInt("Add Ancient Tech Points", &cheatState.addAnchientTechPoints);
    ImGui::SameLine();
    if (ImGui::Button("Add Ancient"))
        AddAncientTechPoints();

    ImGui::Spacing();

    // === Remove Tech Points ===
    ImGui::InputInt("Remove Tech Points", &cheatState.removeTechPoints);
    ImGui::SameLine();
    if (ImGui::Button("Remove"))
        RemoveTechPoints();

    // === Remove Ancient Tech Points ===
    ImGui::InputInt("Remove Ancient Tech Points", &cheatState.removeAnchientTechPoints);
    ImGui::SameLine();
    if (ImGui::Button("Remove Ancient"))
        RemoveAncientTechPoint();
}