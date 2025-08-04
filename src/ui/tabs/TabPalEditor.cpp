#include <pch.h>
#include "Tabs.h"
#include "cheat_state.h"
#include "pal_editor.h"

void TabPalEditor()
{
    static int selectedPalIndex = -1;
    static std::string selectedPalName;

    // ------------------- TOP SECTION -------------------
    float topHeight = 220.0f;

    ImGui::BeginChild("TopSection", ImVec2(0, topHeight), false);

    // Split into two columns: Tamed and Base Workers
    ImGui::Columns(3, nullptr, false);

    // ------------------- Tamed Pals -------------------
    ImGui::BeginChild("PalList", ImVec2(0, topHeight), true);
    ImGui::Text("Tamed Pals: %zu", cachedTamedPals.size());
    ImGui::Separator();

    for (int i = 0; i < cachedTamedPals.size(); i++)
    {
        SDK::APalCharacter* pal = cachedTamedPals[i];
        if (!pal || !pal->CharacterParameterComponent) continue;

        auto* params = pal->CharacterParameterComponent;
        auto* individualParams = params->GetIndividualParameter();
        if (!individualParams) continue;

        int level = individualParams->SaveParameter.Level;
        std::string name = GetCleanPalName2(pal->GetName());
        std::string label = name + " [Lv. " + std::to_string(level) + "]##tamed" + std::to_string(i);

        if (ImGui::Selectable(label.c_str(), selectedPalIndex == i))
        {
            selectedPalIndex = i;
            selectedPalName = name;
        }
    }
    ImGui::EndChild();
    ImGui::NextColumn();

    // ------------------- Base Workers -------------------
    ImGui::BeginChild("BaseWorkers", ImVec2(0, topHeight), true);
    ImGui::Text("Base Workers: %zu", cachedBaseWorkers.size());
    ImGui::Separator();

    for (int i = 0; i < cachedBaseWorkers.size(); i++)
    {
        SDK::APalCharacter* pal = cachedBaseWorkers[i];
        if (!pal || !pal->CharacterParameterComponent) continue;

        auto* params = pal->CharacterParameterComponent;
        auto* individualParams = params->GetIndividualParameter();
        if (!individualParams) continue;

        int level = individualParams->SaveParameter.Level;
        std::string name = GetCleanPalName2(pal->GetName());
        std::string label = name + " [Lv. " + std::to_string(level) + "]##bw" + std::to_string(i);

        // NOTE: You may want to offset index (e.g., +10000) to avoid clashing with tamed selection
        int actualIndex = 10000 + i;

        if (ImGui::Selectable(label.c_str(), selectedPalIndex == actualIndex))
        {
            selectedPalIndex = actualIndex;
            selectedPalName = name;
        }
    }
    ImGui::EndChild();
    ImGui::NextColumn();

    // ------------------- Pal Info -------------------
    ImGui::BeginChild("PalInfo", ImVec2(0, topHeight), true);
    DrawPalInfo(selectedPalIndex);
    ImGui::EndChild();

    ImGui::Columns(1);
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
