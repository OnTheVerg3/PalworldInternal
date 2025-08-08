#include <pch.h>
#include "pal_editor.h"
#include "cheat_state.h"
#include <unordered_set>
#include <iostream>
#include "database.h"
#include <vector>
#include <algorithm>

using namespace Helper;
using namespace SDK;

std::vector<APalCharacter*> cachedTamedPals;
std::vector<APalCharacter*> cachedBaseWorkers;

std::string GetCleanPalName2(const std::string& rawName)
{
    size_t start = 0;

    if (rawName.find("BP_") == 0)
        start += 3;

    if (rawName.find("NPC_", start) == start)
        start += 4;

    size_t end = rawName.find("_C", start);
    std::string coreName = (end != std::string::npos)
        ? rawName.substr(start, end - start)
        : rawName.substr(start);

    while (!coreName.empty() && std::isdigit(coreName.back()))
        coreName.pop_back();

    while (!coreName.empty() && coreName.back() == '_')
        coreName.pop_back();

    return coreName;
}

FName MakeFName(const char* name)
{
    FName fname;
    // TEMPORARY: hash (only for testing)
    fname.ComparisonIndex = std::hash<std::string>{}(name) & 0x7FFFFFFF;
    fname.Number = 0;
    return fname;
}

bool GetAllTamedPals(std::vector<APalCharacter*>& outResult)
{
    outResult.clear();

    TArray<APalCharacter*> allPals;
    if (!GetTAllPals(&allPals))
        return false;

    for (int i = 0; i < allPals.Num(); ++i)
    {
        if (!allPals.IsValidIndex(i)) continue;

        SDK::APalCharacter* pal = allPals[i];
        if (!pal) continue;

        bool isTamed = IsTamed(pal);
        bool isAlive = IsAlive(pal);
        bool baseWorker = IsABaseWorker(pal, true);

        std::string name = GetCleanPalName2(pal->GetName());

        if (isTamed && isAlive && !baseWorker)
        {
            outResult.push_back(pal);
        }
    }

    return !outResult.empty();
}

bool GetAllBaseWorkers(std::vector<SDK::APalCharacter*>& outResult)
{
    outResult.clear();

    TArray<SDK::APalCharacter*> allPals;
    if (!GetTAllPals(&allPals))
        return false;

    for (int i = 0; i < allPals.Num(); ++i)
    {
        if (!allPals.IsValidIndex(i)) continue;

        SDK::APalCharacter* pal = allPals[i];
        if (!pal) continue;

        bool baseWorker = IsABaseWorker(pal, true);
        bool isAlive = IsAlive(pal);

        if (baseWorker && isAlive)
        {
            outResult.push_back(pal);
        }
    }

    return !outResult.empty();
}


void DrawPalInfo(int selectedPalIndex)
{
    SDK::APalCharacter* pal = nullptr;

    if (selectedPalIndex < 0)
    {
        ImGui::Text("Select a Pal from the list");
        return;
    }
    else if (selectedPalIndex >= 10000) // Base Worker list
    {
        int index = selectedPalIndex - 10000;
        if (index >= cachedBaseWorkers.size())
        {
            ImGui::Text("Invalid Base Worker index");
            return;
        }
        pal = cachedBaseWorkers[index];
    }
    else // Tamed Pal list
    {
        if (selectedPalIndex >= cachedTamedPals.size())
        {
            ImGui::Text("Invalid Tamed Pal index");
            return;
        }
        pal = cachedTamedPals[selectedPalIndex];
    }

    if (!pal || !pal->CharacterParameterComponent)
    {
        ImGui::Text("Invalid Pal or missing parameters");
        return;
    }

    auto* params = pal->CharacterParameterComponent;

    UPalIndividualCharacterParameter* individualParams = params->GetIndividualParameter();
    if (!individualParams)
    {
        ImGui::Text("No individual parameter found!");
        return;
    }

    FPalIndividualCharacterSaveParameter saveData = individualParams->SaveParameter;

    std::string charID = saveData.CharacterID.ToString();
    std::string uniqueID = saveData.UniqueNPCID.ToString();
    int level = saveData.Level;
    int rank = saveData.Rank;
    int rankExp = saveData.RankUpExp;
    std::string gender = (saveData.Gender == EPalGenderType::Male) ? "Male" : "Female";

    ImGui::TextColored(ImVec4(1, 1, 0.5f, 1), "Selected Pal: %s", charID.c_str());
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Level: %d", level);
    ImGui::Text("Rank: %d", rank);
    ImGui::Text("Exp for Rank Up: %d", rankExp);
    ImGui::Text("Gender: %s", gender.c_str());
    ImGui::Text("Unique NPC ID: %s", uniqueID.c_str());

    ImGui::Spacing();

    float currentHP = params->GetHP().Value;
    float maxHP = params->GetMaxHP().Value;

    ImGui::Text("HP: %.0f / %.0f", currentHP, maxHP);
    ImGui::ProgressBar(currentHP / maxHP, ImVec2(-1, 0), "");
}

void DrawPalStatsEditor(int selectedPalIndex)
{
    SDK::APalCharacter* pal = nullptr;

    if (selectedPalIndex < 0)
    {
        ImGui::Text("Select a Pal from the list");
        return;
    }
    else if (selectedPalIndex >= 10000) // Base Worker list
    {
        int index = selectedPalIndex - 10000;
        if (index >= cachedBaseWorkers.size())
        {
            ImGui::Text("Invalid Base Worker index");
            return;
        }
        pal = cachedBaseWorkers[index];
    }
    else // Tamed Pal list
    {
        if (selectedPalIndex >= cachedTamedPals.size())
        {
            ImGui::Text("Invalid Tamed Pal index");
            return;
        }
        pal = cachedTamedPals[selectedPalIndex];
    }

    if (!pal || !pal->CharacterParameterComponent)
    {
        ImGui::Text("Invalid Pal or missing parameters");
        return;
    }

    auto* params = pal->CharacterParameterComponent;
    auto* individualParams = params->GetIndividualParameter();
    if (!individualParams)
    {
        ImGui::Text("No individual parameter found!");
        return;
    }

    FPalIndividualCharacterSaveParameter& saveData = individualParams->SaveParameter;

    static int lastPalIndex = -1;
    static int level, rank;
    static int64_t exp;
    static float hp;
    static int gender; // 0 = Male, 1 = Female

    if (lastPalIndex != selectedPalIndex)
    {
        level = saveData.Level;
        rank = saveData.Rank;
        exp = saveData.Exp;
        hp = saveData.Hp.Value;
        gender = (saveData.Gender == EPalGenderType::Male) ? 0 : 1;

        lastPalIndex = selectedPalIndex;
    }

    // --- Core Stats UI ---
    ImGui::InputFloat("Current HP", &hp);
    ImGui::InputInt("Level", &level);
    ImGui::InputInt("Skill Rank", &rank);
    ImGui::InputScalar("Exp", ImGuiDataType_S64, &exp);

    // Gender Combo
    const char* genderOptions[] = { "Male", "Female" };
    ImGui::Combo("Gender", &gender, genderOptions, IM_ARRAYSIZE(genderOptions));

    ImGui::Spacing();

    if (ImGui::Button("Apply Stats Changes", ImVec2(-1, 30)))
    {
        saveData.Hp = FFixedPoint64(hp);
        saveData.Level = level;
        saveData.Rank = rank;
        saveData.Exp = exp;
        saveData.Gender = (gender == 0) ? EPalGenderType::Male : EPalGenderType::Female;

        // Trigger update replication
        params->OnRep_IndividualParameter();
        individualParams->OnRep_SaveParameter();

        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "Stats updated!");
    }
}

void DrawPalRanksEditor(int selectedPalIndex)
{
    SDK::APalCharacter* pal = nullptr;

    if (selectedPalIndex < 0)
    {
        ImGui::Text("Select a Pal from the list");
        return;
    }
    else if (selectedPalIndex >= 10000) // Base Worker list
    {
        int index = selectedPalIndex - 10000;
        if (index >= cachedBaseWorkers.size())
        {
            ImGui::Text("Invalid Base Worker index");
            return;
        }
        pal = cachedBaseWorkers[index];
    }
    else // Tamed Pal list
    {
        if (selectedPalIndex >= cachedTamedPals.size())
        {
            ImGui::Text("Invalid Tamed Pal index");
            return;
        }
        pal = cachedTamedPals[selectedPalIndex];
    }

    if (!pal || !pal->CharacterParameterComponent)
    {
        ImGui::Text("Invalid Pal or missing parameters");
        return;
    }

    auto* params = pal->CharacterParameterComponent;
    auto* individualParams = params->GetIndividualParameter();
    if (!individualParams)
    {
        ImGui::Text("No individual parameter found!");
        return;
    }

    FPalIndividualCharacterSaveParameter& saveData = individualParams->SaveParameter;

    static int rankHp, rankAtk, rankDef, rankCraft;
    static int lastPalIndex = -1;

    if (lastPalIndex != selectedPalIndex)
    {
        rankHp = saveData.Rank_HP;
        rankAtk = saveData.Rank_Attack;
        rankDef = saveData.Rank_Defence;
        rankCraft = saveData.Rank_CraftSpeed;
        lastPalIndex = selectedPalIndex;
    }

    ImGui::SetNextItemWidth(-1);
    ImGui::SliderInt("##rankhp", &rankHp, 0, 255, "Rank HP = %d");
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderInt("##rankatk", &rankAtk, 0, 255, "Rank Attack = %d");
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderInt("##rankdef", &rankDef, 0, 255, "Rank Defence = %d");
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderInt("##rankcraft", &rankCraft, 0, 255, "Rank Craft = %d");

    ImGui::Spacing();

    if (ImGui::Button("Apply Rank Changes", ImVec2(-1, 30)))
    {
        saveData.Rank_HP = rankHp;
        saveData.Rank_Attack = rankAtk;
        saveData.Rank_Defence = rankDef;
        saveData.Rank_CraftSpeed = rankCraft;

		params->OnRep_IndividualParameter();
        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "Rank stats updated!");
    }
}

void DrawPalWorkSuitabilitiesEditor(int selectedPalIndex)
{
    SDK::APalCharacter* pal = nullptr;

    if (selectedPalIndex < 0)
    {
        ImGui::Text("Select a Pal from the list");
        return;
    }
    else if (selectedPalIndex >= 10000) // Base Worker list
    {
        int index = selectedPalIndex - 10000;
        if (index >= cachedBaseWorkers.size())
        {
            ImGui::Text("Invalid Base Worker index");
            return;
        }
        pal = cachedBaseWorkers[index];
    }
    else // Tamed Pal list
    {
        if (selectedPalIndex >= cachedTamedPals.size())
        {
            ImGui::Text("Invalid Tamed Pal index");
            return;
        }
        pal = cachedTamedPals[selectedPalIndex];
    }

    if (!pal || !pal->CharacterParameterComponent)
    {
        ImGui::Text("Invalid Pal or missing parameters");
        return;
    }

    auto* params = pal->CharacterParameterComponent;
    auto* individualParams = params->GetIndividualParameter();
    if (!individualParams)
    {
        ImGui::Text("No individual parameter found!");
        return;
    }

    FPalIndividualCharacterSaveParameter& saveData = individualParams->SaveParameter;

    static const char* suitabilityNames[] = {
        "None", "Kindling", "Watering", "Planting", "Generate Electricity",
        "Handcraft", "Gathering", "Cooling", "Mining", "Oil Extraction",
        "Medicine", "Cool", "Transporting", "Farming", "Anyone"
    };

    auto& suitabilities = saveData.CraftSpeeds;
    auto& extraSuitabilities = saveData.GotWorkSuitabilityAddRankList;

    ImGui::TextColored(ImVec4(0.8f, 0.9f, 1, 1), "Current Work Suitabilities");

    bool changed = false;

    static std::vector<int> tempRanks(100, 0);

    for (int i = 0; i < suitabilities.Num(); i++)
    {
        auto& suitabilityInfo = suitabilities[i];
        int baseRank = suitabilityInfo.Rank;

        // Find extra rank
        int extraRank = 0;
        for (int j = 0; j < extraSuitabilities.Num(); j++)
        {
            if (extraSuitabilities[j].WorkSuitability == suitabilityInfo.WorkSuitability)
            {
                extraRank = extraSuitabilities[j].Rank;
                break;
            }
        }

        int finalRank = baseRank + extraRank;

        // Only display if rank > 0
        if (finalRank > 0)
        {
            const char* name = suitabilityNames[(int)suitabilityInfo.WorkSuitability];
            ImGui::Text("%s", name);
            ImGui::SameLine(200);

            if (tempRanks.size() <= i)
                tempRanks.resize(i + 1, finalRank);
            if (tempRanks[i] == 0)
                tempRanks[i] = finalRank;

            ImGui::InputInt(("Rank##" + std::to_string(i)).c_str(), &tempRanks[i]);

            // Apply changes if value changed
            if (tempRanks[i] != finalRank)
            {
                int desiredRank = tempRanks[i];

                // If user sets rank <= baseRank, directly modify CraftSpeeds (base)
                if (desiredRank <= baseRank)
                {
                    suitabilityInfo.Rank = desiredRank;
                }
                else
                {
                    for (int j = 0; j < extraSuitabilities.Num(); j++)
                    {
                        if (extraSuitabilities[j].WorkSuitability == suitabilityInfo.WorkSuitability)
                        {
                            // Remove old entry
                            extraSuitabilities.Remove(j);
                            break;
                        }
                    }

                    // Now set the new value
                    individualParams->SetWorkSuitabilityAddRank(
                        suitabilityInfo.WorkSuitability, desiredRank - baseRank
                    );
                }

                changed = true;
            }
        }
    }

    if (changed)
    {
        params->OnRep_IndividualParameter();
        individualParams->OnRep_SaveParameter();
    }
}


void DumpAllPassiveSkills();

void DrawPalPassiveSkillsEditor(int selectedPalIndex)
{
    SDK::APalCharacter* pal = nullptr;

    if (selectedPalIndex < 0)
    {
        ImGui::TextColored(ImVec4(1, 0.5f, 0.5f, 1), "Select a Pal from the list.");
        return;
    }

    // Determine pal type
    if (selectedPalIndex >= 10000)
    {
        int index = selectedPalIndex - 10000;
        if (index >= cachedBaseWorkers.size()) return;
        pal = cachedBaseWorkers[index];
    }
    else
    {
        if (selectedPalIndex >= cachedTamedPals.size()) return;
        pal = cachedTamedPals[selectedPalIndex];
    }

    if (!pal || !pal->CharacterParameterComponent) return;

    auto* params = pal->CharacterParameterComponent;
    auto* individualParams = params->GetIndividualParameter();
    if (!individualParams) return;

    auto& saveData = individualParams->SaveParameter;
    auto& passiveSkills = saveData.PassiveSkillList;

    // === CURRENT SKILLS LIST ===
    if (passiveSkills.Num() > 0)
    {
        ImGui::BeginChild("CurrentSkills", ImVec2(0, 110), true);
        for (int i = 0; i < passiveSkills.Num(); ++i)
        {
            const std::string skillName = passiveSkills[i].ToString();

            ImGui::Columns(2, nullptr, false);
            ImGui::Text("• %s", skillName.c_str());
            ImGui::NextColumn();

            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);
            std::string btnLabel = "Remove##" + std::to_string(i);
            if (ImGui::SmallButton(btnLabel.c_str()))
            {
                passiveSkills.Remove(i);
                break;
            }

            ImGui::Columns(1);
        }
        ImGui::EndChild();
    }
    else
    {
        ImGui::TextDisabled("No passive skills assigned.");
    }

    ImGui::Spacing();

    // === SEARCH + SELECT ===
    static char searchBuffer[64] = "";
    ImGui::SetNextItemWidth(-1);
    ImGui::InputTextWithHint("##SearchSkill", "Search for a skill...", searchBuffer, IM_ARRAYSIZE(searchBuffer));

    std::string searchText = searchBuffer;
    std::transform(searchText.begin(), searchText.end(), searchText.begin(), ::tolower);

    static std::string selectedKey;
    const auto& skillDB = database::PassiveSkillDatabase;

    ImGui::Spacing();
    ImGui::BeginChild("SkillListSelectable", ImVec2(0, 170), true);

    for (const auto& kv : skillDB)
    {
        std::string label = kv.second;
        std::string lower = label;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

        if (!searchText.empty() && lower.find(searchText) == std::string::npos)
            continue;

        bool selected = (selectedKey == kv.first);
        if (ImGui::Selectable(label.c_str(), selected))
            selectedKey = kv.first;
    }

    ImGui::EndChild();
    ImGui::Spacing();

    // === ADD BUTTON ===
    ImVec2 buttonSize = ImVec2(-1, 30);
    if (ImGui::Button("Add Selected Skill", buttonSize))
    {
        if (!selectedKey.empty())
        {
            bool exists = false;
            for (int i = 0; i < passiveSkills.Num(); ++i)
            {
                if (passiveSkills[i].ToString() == selectedKey)
                {
                    exists = true;
                    break;
                }
            }

            if (!exists)
            {
                static UKismetStringLibrary* lib = UKismetStringLibrary::GetDefaultObj();
                SDK::FName fname = lib->Conv_StringToName(SDK::FString(std::wstring(selectedKey.begin(), selectedKey.end()).c_str()));
                passiveSkills.Add(fname);
            }
        }
    }
}


void DumpAllPassiveSkills()
{
    std::unordered_set<std::string> uniqueSkills;

    for (auto* pal : cachedTamedPals)
    {
        if (!pal || !pal->CharacterParameterComponent)
            continue;

        auto* params = pal->CharacterParameterComponent->GetIndividualParameter();
        if (!params)
            continue;

        auto& passiveSkills = params->SaveParameter.PassiveSkillList;

        for (int i = 0; i < passiveSkills.Num(); i++)
        {
            std::string skillName = passiveSkills[i].ToString();
            uniqueSkills.insert(skillName);
        }
    }

    // Output the results
    std::cout << "[Passive Skill Dump] Found " << uniqueSkills.size() << " unique skills:\n";
    for (const auto& skill : uniqueSkills)
    {
        std::cout << "  " << skill << "\n";
    }
}

