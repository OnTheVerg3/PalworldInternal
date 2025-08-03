#include <pch.h>
#include "Tabs.h"
#include "cheat_state.h"
#include "pal_editor.h"

void TabPalEditor()
{
    static int selectedPalIndex = -1;
    static std::string selectedPalName;

    if (cachedTamedPals.empty())
    {
        ImGui::Text("No tamed Pals found!");
        if (ImGui::Button("Refresh"))
        {
            cachedTamedPals.clear();
            GetAllTamedPals(cachedTamedPals);
        }
        return;
    }

    ImGui::Text("Tamed Pals: %zu", cachedTamedPals.size());

    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, 200);

    ImGui::BeginChild("PalList", ImVec2(0, 300), true);
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

    ImGui::NextColumn();

    ImGui::BeginChild("PalEditor", ImVec2(0, 0), true);
    if (selectedPalIndex >= 0 && selectedPalIndex < cachedTamedPals.size())
    {
        ImGui::Text("Selected Pal: %s", selectedPalName.c_str());
    }
    else
    {
        ImGui::Text("Select a Pal");
    }
    ImGui::EndChild();

    ImGui::Columns(1);
}
