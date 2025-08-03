#include <pch.h>
#include "Tabs.h"
#include "cheat_state.h"
#include "pal_editor.h"

void TabPalEditor()
{
    static int selectedPalIndex = -1;
    static std::string selectedPalName;

    // Handle empty list
    if (cachedTamedPals.empty())
    {
        ImGui::Text("No tamed Pals found!");
        return;
    }

    // ------------------- TOP SECTION -------------------
    float topHeight = 220.0f;

    // Pal List (Left)
    ImGui::BeginChild("PalList", ImVec2(220, topHeight), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
    ImGui::Text("Tamed Pals: %zu", cachedTamedPals.size());
    ImGui::Separator();

    for (int i = 0; i < cachedTamedPals.size(); i++)
    {
        SDK::APalCharacter* pal = cachedTamedPals[i];
        if (!pal) continue;

        std::string name = GetCleanPalName2(pal->GetName());
        if (ImGui::Selectable(name.c_str(), selectedPalIndex == i))
        {
            selectedPalIndex = i;
            selectedPalName = name;
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();

    // Pal Info (Right)
    ImGui::BeginChild("PalInfo", ImVec2(0, topHeight), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
    DrawPalInfo(selectedPalIndex);
    ImGui::EndChild();

    ImGui::Spacing();

    // ------------------- BOTTOM SECTION -------------------
    ImGui::BeginChild("PalEditorFull", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysUseWindowPadding);

    // Stats Box
    float childWidth = ImGui::GetContentRegionAvail().x * 0.5f;

    ImGui::BeginChild("StatsBox", ImVec2(childWidth - 5, 300), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
    ImGui::TextColored(ImVec4(1, 1, 0.5f, 1), "Stats Editor");
    ImGui::Separator();
    DrawPalStatsEditor(selectedPalIndex);
    ImGui::EndChild();

    ImGui::SameLine();

    // Ranks Box
    ImGui::BeginChild("Ranks", ImVec2(childWidth - 5, 300), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
    ImGui::TextColored(ImVec4(1, 1, 0.5f, 1), "Ranks Editor");
    ImGui::Separator();
    DrawPalRanksEditor(selectedPalIndex);
    ImGui::EndChild();

    ImGui::Spacing();

    // Work Suitability Box (with forceRefresh)
    ImGui::BeginChild("WorkBox", ImVec2(0, 200), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
    DrawPalWorkSuitabilitiesEditor(selectedPalIndex);
    ImGui::EndChild();

    ImGui::Spacing();

    // Passive Skills Box
    ImGui::BeginChild("SkillsBox", ImVec2(0, 350), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
    DrawPalPassiveSkillsEditor(selectedPalIndex);
    ImGui::EndChild();

    ImGui::EndChild();
}
