#include <pch.h>
#include <algorithm>
#include "Tabs.h"
#include "cheat_state.h"
#include "ItemList.hpp"
#include "src/ui/imgui_style.h"
#include "ConfigManager.h"

void TabItemSpawner()
{
    static int selectedCategoryIndex = 0;
    static char searchBuf[128] = "";
    static std::string selectedItemID = "";
    static int spawnCount = 1;

    // tiny inline help marker
    auto HelpMarker = [](const char* desc)
        {
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
            {
                ImGui::BeginTooltip();
                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                ImGui::TextUnformatted(desc);
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }
        };

    ImVec4 headerColor = ImVec4(1.0f, 0.9f, 0.6f, 1.0f);
    ColoredSeparatorText("Item Spawner", headerColor);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.35f, 1.0f));
    ImGui::TextWrapped("Note: Temporary client-side items. Server may replace/remove them on inventory sync or relog.");
    ImGui::PopStyleColor();
    ImGui::Spacing();

    // === CATEGORY COMBO ===
    std::vector<std::string> categoryNames;
    categoryNames.reserve(itemlist::itemCategories.size() + 1);
    categoryNames.push_back("All");
    for (const auto& [name, _] : itemlist::itemCategories) categoryNames.push_back(name);

    ImGui::Text("Category");
    if (ImGui::BeginCombo("##Category", categoryNames[selectedCategoryIndex].c_str(), ImGuiComboFlags_HeightLarge))
    {
        for (int i = 0; i < (int)categoryNames.size(); ++i)
        {
            bool isSelected = (i == selectedCategoryIndex);
            if (ImGui::Selectable(categoryNames[i].c_str(), isSelected))
                selectedCategoryIndex = i;
            if (isSelected) ImGui::SetItemDefaultFocus();
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
    ImGui::BeginChild("ItemListChild", ImVec2(0, 250), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
    for (const char* entry : selectedList)
    {
        std::string str = entry;
        const auto sep = str.find('|');
        std::string id = str.substr(0, sep);
        std::string label = (sep != std::string::npos) ? str.substr(sep + 1) : id;

        std::string idLower = id, labelLower = label, searchLower = searchBuf;
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

    // === ITEM SELECTED + COUNT + SPAWN / ADD TO LIST ===
    if (!selectedItemID.empty())
    {
        ImGui::TextColored(headerColor, "Selected Item: %s", selectedItemID.c_str());
        ImGui::SetNextItemWidth(160);
        ImGui::SliderInt("##countslider", &spawnCount, 1, 9999, "Count: %d");

        if (ImGui::Button("Give Item", ImVec2(-1, 0)))
        {
            AddItemToInventoryByName(selectedItemID, spawnCount);
        }

        if (ImGui::Button("Add to Quick Re-Spawn", ImVec2(-1, 0)))
        {
            bool merged = false;
            for (auto& it : cheatState.bulkItems)
            {
                if (it.id == selectedItemID)
                {
                    it.count += spawnCount;
                    merged = true;
                    break;
                }
            }
            if (!merged) cheatState.bulkItems.push_back({ selectedItemID, spawnCount });
        }
    }

    ImGui::Separator();
    ImGui::Spacing();

    // === QUICK RE-SPAWN (client-side reminder) ===
    ImGui::TextColored(headerColor, "Quick Re-Spawn (Temporary)");
    ImGui::SameLine();
    HelpMarker(
        "Client-side temporary items (not saved, not server-authoritative).\n"
        "\n"
        "Works well:\n"
        "- Ammo and Pal Spheres: fully usable.\n"
        "- Blueprints: unlock as usual.\n"
        "- Equipment: if the slot is empty it will auto-equip and you can use it.\n"
        "\n"
        "Limitations:\n"
        "- Inventory is not synced to the server. Any sync (pick up, drop, sort, fast travel, or relog)\n"
        "  can overwrite or remove these items.\n"
        "- Moving them around may be reverted on the next sync.\n"
        "\n"
        "Use this list to quickly re-add your usual items each session."
    );

    if (ImGui::BeginTable("QuickRespawnTable", 4,
        ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders))
    {
        ImGui::TableSetupColumn("Item ID", ImGuiTableColumnFlags_WidthStretch, 0.55f);
        ImGui::TableSetupColumn("Qty", ImGuiTableColumnFlags_WidthStretch, 0.20f);
        ImGui::TableSetupColumn("Give", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("Remove", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableHeadersRow();

        for (int i = 0; i < (int)cheatState.bulkItems.size(); ++i)
        {
            auto& it = cheatState.bulkItems[i];

            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(it.id.c_str());

            ImGui::TableSetColumnIndex(1);
            int cnt = it.count;
            ImGui::SetNextItemWidth(-1);
            if (ImGui::InputInt(("##cnt" + std::to_string(i)).c_str(), &cnt, 1, 10))
                it.count = (cnt < 1) ? 1 : cnt;

            ImGui::TableSetColumnIndex(2);
            if (ImGui::Button(("Give##" + std::to_string(i)).c_str()))
                AddItemToInventoryByName(it.id, it.count);

            ImGui::TableSetColumnIndex(3);
            if (ImGui::Button(("Remove##" + std::to_string(i)).c_str()))
            {
                cheatState.bulkItems.erase(cheatState.bulkItems.begin() + i);
                --i; // keep loop stable after erase
            }
        }
        ImGui::EndTable();
    }

    ImGui::Spacing();

    if (ImGui::Button("Give All (Temporary)", ImVec2(-1, 0)))
    {
        for (const auto& it : cheatState.bulkItems)
            AddItemToInventoryByName(it.id, it.count);

        SaveConfig("config.txt");
    }

    if (ImGui::Button("Clear Quick Re-Spawn List", ImVec2(-1, 0)))
        cheatState.bulkItems.clear();

    ImGui::Separator();
}


