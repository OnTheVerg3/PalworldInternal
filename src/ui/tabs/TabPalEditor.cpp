#include <pch.h>
#include "Tabs.h"
#include "cheat_state.h"
#include "pal_editor.h"
#include "src/ui/imgui_style.h"

void TabPalEditor()
{
	static int selectedPalIndex = -1;
	static std::string selectedPalName;
	ImVec4 headerColor = ImVec4(1.0f, 0.9f, 0.6f, 1.0f);

	ColoredSeparatorText("Pal Editor", headerColor);
	ImGui::Spacing();

	// === TOP PANEL ===
	float leftPanelWidth = 320.0f;

	ImGui::BeginGroup();

	// -- Pal List (Tamed + Base Workers)
	ImGui::BeginChild("PalList", ImVec2(leftPanelWidth, 250.0f), true);
	ImGui::TextColored(headerColor, "Tamed Pals");
	for (int i = 0; i < cachedTamedPals.size(); ++i)
	{
		auto* pal = cachedTamedPals[i];
		if (!pal || !pal->CharacterParameterComponent) continue;
		auto* individualParams = pal->CharacterParameterComponent->GetIndividualParameter();
		if (!individualParams) continue;

		std::string name = GetCleanPalName2(pal->GetName());
		int level = individualParams->SaveParameter.Level;
		std::string label = name + " [Lv. " + std::to_string(level) + "]##t" + std::to_string(i);

		if (ImGui::Selectable(label.c_str(), selectedPalIndex == i))
		{
			selectedPalIndex = i;
			selectedPalName = name;
		}
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::TextColored(headerColor, "Base Workers");
	for (int i = 0; i < cachedBaseWorkers.size(); ++i)
	{
		auto* pal = cachedBaseWorkers[i];
		if (!pal || !pal->CharacterParameterComponent) continue;
		auto* individualParams = pal->CharacterParameterComponent->GetIndividualParameter();
		if (!individualParams) continue;

		std::string name = GetCleanPalName2(pal->GetName());
		int level = individualParams->SaveParameter.Level;
		int actualIndex = 10000 + i;
		std::string label = name + " [Lv. " + std::to_string(level) + "]##b" + std::to_string(i);

		if (ImGui::Selectable(label.c_str(), selectedPalIndex == actualIndex))
		{
			selectedPalIndex = actualIndex;
			selectedPalName = name;
		}
	}
	ImGui::EndChild();
	ImGui::EndGroup();

	ImGui::SameLine();

	// -- Pal Info (Smaller Box)
	ImGui::BeginGroup();
	ImGui::BeginChild("PalInfo", ImVec2(0, 250.0f), true);
	ImGui::TextColored(headerColor, "Pal Info");
	DrawPalInfo(selectedPalIndex);
	ImGui::EndChild();
	ImGui::EndGroup();

	ImGui::Spacing();

	// === MIDDLE PANEL ===
	float half = ImGui::GetContentRegionAvail().x * 0.5f;

	ImGui::BeginChild("StatsBox", ImVec2(half - 5, 240), true);
	ImGui::TextColored(headerColor, "Stats");
	DrawPalStatsEditor(selectedPalIndex);
	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild("RanksBox", ImVec2(half - 5, 240), true);
	ImGui::TextColored(headerColor, "Ranks");
	DrawPalRanksEditor(selectedPalIndex);
	ImGui::EndChild();

	ImGui::Spacing();

	// === Work Suitability ===
	ImGui::BeginChild("WorkSuitabilityBox", ImVec2(0, 170), true);
	ImGui::TextColored(headerColor, "Work Suitability");
	DrawPalWorkSuitabilitiesEditor(selectedPalIndex);
	ImGui::EndChild();

	ImGui::Spacing();

	// === Passive Skills ===
	ImGui::BeginChild("SkillsBox", ImVec2(0, 320), true);
	ImGui::TextColored(headerColor, "Passive Skills");
	DrawPalPassiveSkillsEditor(selectedPalIndex);
	ImGui::EndChild();
}

