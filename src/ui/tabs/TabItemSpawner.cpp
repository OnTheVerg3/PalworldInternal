#include <pch.h>
#include "Tabs.h"
#include "cheat_state.h"
#include "ItemList.hpp"


void TabItemSpawner()
{
    if (ImGui::BeginTabItem("Item Spawner"))
    {

        static int selectedCategoryIndex = 0;
        static char searchBuf[128] = "";
        static std::string selectedItemID = "";

        // Generate list of category names
        std::vector<std::string> categoryNames;
        for (const auto& [name, _] : itemlist::itemCategories)
            categoryNames.push_back(name);

        // Draw category combo box
        if (ImGui::BeginCombo("Category", categoryNames[selectedCategoryIndex].c_str()))
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

        // Filter input
        ImGui::InputText("Search", searchBuf, IM_ARRAYSIZE(searchBuf));

        // Get the selected category list
        const auto& selectedList = *itemlist::itemCategories.at(categoryNames[selectedCategoryIndex]);

        // Show filtered list
        ImGui::BeginChild("ItemList", ImVec2(0, 300), true);
        for (const auto& entry : selectedList)
        {
            std::string str = entry;
            auto sep = str.find('|');
            std::string id = str.substr(0, sep);
            std::string label = (sep != std::string::npos) ? str.substr(sep + 1) : id;

            // Search filter
            if (strstr(id.c_str(), searchBuf) || strstr(label.c_str(), searchBuf))
            {
                if (ImGui::Selectable(label.c_str(), selectedItemID == id))
                    selectedItemID = id;
            }
        }
        ImGui::EndChild();

        static int spawnCount = 1;
        // Spawn button
        if (!selectedItemID.empty())
        {
            ImGui::Text("Selected: %s", selectedItemID.c_str());

            ImGui::InputInt("Count", &spawnCount);

            // Optional: Clamp count to safe values
            if (spawnCount < 1) spawnCount = 1;
            if (spawnCount > 9999) spawnCount = 9999;

            if (ImGui::Button("Spawn Item"))
            {
                AddItemToInventoryByName(selectedItemID, spawnCount);
            }
        }
        ImGui::EndTabItem();
    }
}