#include <pch.h>
#include <algorithm>
#include "Tabs.h"
#include "cheat_state.h"
#include "ItemList.hpp"
#include "src/ui/imgui_style.h"

void TabItemSpawner()
{
    static int selectedCategoryIndex = 0;
    static char searchBuf[128] = "";
    static std::string selectedItemID = "";
    static int spawnCount = 1;

    ImVec4 headerColor = ImVec4(1.0f, 0.9f, 0.6f, 1.0f);
    ColoredSeparatorText("Item Spawner", headerColor);
    ImGui::Spacing();

    // === CATEGORY COMBO ===
    std::vector<std::string> categoryNames;
    categoryNames.push_back("All");

    for (const auto& [name, _] : itemlist::itemCategories)
        categoryNames.push_back(name);

    ImGui::Text("Category");
    if (ImGui::BeginCombo("##Category", categoryNames[selectedCategoryIndex].c_str(), ImGuiComboFlags_HeightLarge))
    {
        for (int i = 0; i < categoryNames.size(); ++i)
        {
            bool isSelected = (i == selectedCategoryIndex);
            if (ImGui::Selectable(categoryNames[i].c_str(), isSelected))
                selectedCategoryIndex = i;
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::Spacing();

    // === SEARCH FIELD ===
    ImGui::SetNextItemWidth(-1);
    ImGui::InputTextWithHint("##SearchBox", "Search for items...", searchBuf, IM_ARRAYSIZE(searchBuf));
    ImGui::Spacing();

    // === BUILD ITEM LIST ===
    std::vector<const char*> selectedList;
    if (selectedCategoryIndex == 0) // All
    {
        for (const auto& [_, items] : itemlist::itemCategories)
            selectedList.insert(selectedList.end(), items->begin(), items->end());
    }
    else
    {
        const auto& selectedCatName = categoryNames[selectedCategoryIndex];
        selectedList = *itemlist::itemCategories.at(selectedCatName);
    }

    // === ITEM LIST WINDOW ===
    ImGui::BeginChild("ItemListChild", ImVec2(0, 400), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
    for (const char* entry : selectedList)
    {
        std::string str = entry;
        auto sep = str.find('|');
        std::string id = str.substr(0, sep);
        std::string label = (sep != std::string::npos) ? str.substr(sep + 1) : id;

        std::string idLower = id;
        std::string labelLower = label;
        std::string searchLower = searchBuf;

        std::transform(idLower.begin(), idLower.end(), idLower.begin(), ::tolower);
        std::transform(labelLower.begin(), labelLower.end(), labelLower.begin(), ::tolower);
        std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);

        if (idLower.find(searchLower) != std::string::npos || labelLower.find(searchLower) != std::string::npos)
        {
            if (ImGui::Selectable(label.c_str(), selectedItemID == id))
                selectedItemID = id;
        }
    }
    ImGui::EndChild();

    ImGui::Spacing();

    // === ITEM SELECTED + COUNT + SPAWN ===
    if (!selectedItemID.empty())
    {
        ImGui::TextColored(headerColor, "Selected Item: %s", selectedItemID.c_str());
        ImGui::SetNextItemWidth(160);
        ImGui::SliderInt("##countslider", &spawnCount, 1, 9999, "Count: %d");

        if (ImGui::Button("Spawn Item", ImVec2(-1, 0)))
        {
            AddItemToInventoryByName(selectedItemID, spawnCount);
        }
    }

    ImGui::Separator();
}
