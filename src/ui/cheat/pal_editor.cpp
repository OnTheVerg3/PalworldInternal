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

std::vector<SDK::APalCharacter*> cachedTamedPals;

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

bool GetAllTamedPals(std::vector<SDK::APalCharacter*>& outResult)
{
    outResult.clear();

    SDK::TArray<SDK::APalCharacter*> allPals;
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


void DrawPalInfo(int selectedPalIndex)
{
    if (selectedPalIndex < 0 || selectedPalIndex >= cachedTamedPals.size())
    {
        ImGui::Text("Select a Pal from the list");
        return;
    }

    APalCharacter* pal = cachedTamedPals[selectedPalIndex];
    if (!pal || !pal->CharacterParameterComponent)
    {
        ImGui::Text("Invalid Pal or missing parameters");
        return;
    }

    auto* params = pal->CharacterParameterComponent;

    // Access UPalIndividualCharacterParameter
    UPalIndividualCharacterParameter* individualParams = params->GetIndividualParameter();
    if (!individualParams)
    {
        ImGui::Text("No individual parameter found!");
        return;
    }

    // Grab Save Data
    FPalIndividualCharacterSaveParameter saveData = individualParams->SaveParameter;

    std::string charID = saveData.CharacterID.ToString();
    std::string uniqueID = saveData.UniqueNPCID.ToString();
    int level = saveData.Level;
    int rank = saveData.Rank;
    int rankExp = saveData.RankUpExp;
    std::string gender = (saveData.Gender == EPalGenderType::Male) ? "Male" : "Female";

    // UI Rendering
    ImGui::TextColored(ImVec4(1, 1, 0.5f, 1), "Selected Pal: %s", charID.c_str());
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Level: %d", level);
    ImGui::Text("Rank: %d", rank);
    ImGui::Text("Exp for Rank Up: %d", rankExp);
    ImGui::Text("Gender: %s", gender.c_str());
    ImGui::Text("Unique NPC ID: %s", uniqueID.c_str());

    ImGui::Spacing();

    // HP bar
    float currentHP = params->GetHP().Value;
    float maxHP = params->GetMaxHP().Value;

    ImGui::Text("HP: %.0f / %.0f", currentHP, maxHP);
    ImGui::ProgressBar(currentHP / maxHP, ImVec2(-1, 0), "");
}

void DrawPalStatsEditor(int selectedPalIndex)
{
    if (selectedPalIndex < 0 || selectedPalIndex >= cachedTamedPals.size())
    {
        ImGui::Text("Select a Pal to edit stats");
        return;
    }

    SDK::APalCharacter* pal = cachedTamedPals[selectedPalIndex];
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

    SDK::FPalIndividualCharacterSaveParameter& saveData = individualParams->SaveParameter;

    // Editable values (persistent)
    static int lastPalIndex = -1;
    static int level, rank;
    static int64_t exp;
    static float hp;
    static int gender; // 0 = Male, 1 = Female

    // Reload when switching Pal
    if (lastPalIndex != selectedPalIndex)
    {
        level = saveData.Level;
        rank = saveData.Rank;
        exp = saveData.Exp;
        hp = saveData.Hp.Value;
        gender = (saveData.Gender == SDK::EPalGenderType::Male) ? 0 : 1;

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
        // Save changes only when Apply is pressed
        saveData.Hp = SDK::FFixedPoint64(hp);
        saveData.Level = level;
        saveData.Rank = rank;
        saveData.Exp = exp;
        saveData.Gender = (gender == 0) ? SDK::EPalGenderType::Male : SDK::EPalGenderType::Female;

        // Trigger update replication
        params->OnRep_IndividualParameter();
        individualParams->OnRep_SaveParameter();

        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "Stats updated!");
    }
}


void DrawPalRanksEditor(int selectedPalIndex)
{
    if (selectedPalIndex < 0 || selectedPalIndex >= cachedTamedPals.size())
    {
        ImGui::Text("Select a Pal to edit ranks");
        return;
    }

    SDK::APalCharacter* pal = cachedTamedPals[selectedPalIndex];
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

    // Editable Rank stats
    SDK::FPalIndividualCharacterSaveParameter& saveData = individualParams->SaveParameter;

    static int rankHp, rankAtk, rankDef, rankCraft;
    static int lastPalIndex = -1;

    // Load when Pal changes
    if (lastPalIndex != selectedPalIndex)
    {
        rankHp = saveData.Rank_HP;
        rankAtk = saveData.Rank_Attack;
        rankDef = saveData.Rank_Defence;
        rankCraft = saveData.Rank_CraftSpeed;
        lastPalIndex = selectedPalIndex;
    }

    // UI
    ImGui::InputInt("Rank HP", &rankHp);
    ImGui::InputInt("Rank Attack", &rankAtk);
    ImGui::InputInt("Rank Defence", &rankDef);
    ImGui::InputInt("Rank Craft Speed", &rankCraft);

    ImGui::Spacing();

    if (ImGui::Button("Apply Rank Changes", ImVec2(-1, 30)))
    {
        saveData.Rank_HP = rankHp;
        saveData.Rank_Attack = rankAtk;
        saveData.Rank_Defence = rankDef;
        saveData.Rank_CraftSpeed = rankCraft;

        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "Rank stats updated!");
    }
}

void DrawPalWorkSuitabilitiesEditor(int selectedPalIndex)
{
    if (selectedPalIndex < 0 || selectedPalIndex >= cachedTamedPals.size())
    {
        ImGui::Text("Select a Pal to edit work suitabilities");
        return;
    }

    SDK::APalCharacter* pal = cachedTamedPals[selectedPalIndex];
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

    SDK::FPalIndividualCharacterSaveParameter& saveData = individualParams->SaveParameter;

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
        // Trigger replication/update
        params->OnRep_IndividualParameter();
        individualParams->OnRep_SaveParameter();
    }
}


void DumpAllPassiveSkills();

void DrawPalPassiveSkillsEditor(int selectedPalIndex)
{
    if (selectedPalIndex < 0 || selectedPalIndex >= cachedTamedPals.size())
    {
        ImGui::Text("Select a Pal to edit passive skills");
        return;
    }

    SDK::APalCharacter* pal = cachedTamedPals[selectedPalIndex];
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

    auto& saveData = individualParams->SaveParameter;
    auto& passiveSkills = saveData.PassiveSkillList;

    // --- Show current passive skills ---
    ImGui::TextColored(ImVec4(1, 1, 0.5f, 1), "Passive Skills:");
    for (int i = 0; i < passiveSkills.Num(); i++)
    {
        std::string skillName = passiveSkills[i].ToString();
        ImGui::BulletText("%s", skillName.c_str());

        ImGui::SameLine();
        if (ImGui::Button(("Remove##" + std::to_string(i)).c_str()))
        {
            passiveSkills.Remove(i);
            break; // break to avoid iterator issues
        }
    }

    ImGui::Separator();

    // --- Search bar ---
    static char searchBuffer[64] = "";
    ImGui::InputText("Search", searchBuffer, IM_ARRAYSIZE(searchBuffer));
    std::string searchText = searchBuffer;
    std::transform(searchText.begin(), searchText.end(), searchText.begin(), ::tolower);

    // --- Select skill ---
    static std::string selectedKey;
    const auto& skillDB = database::PassiveSkillDatabase;

    ImGui::BeginChild("SkillList", ImVec2(0, 150), true);
    for (auto& kv : skillDB)
    {
        std::string display = kv.second;
        std::string lowerDisplay = display;
        std::transform(lowerDisplay.begin(), lowerDisplay.end(), lowerDisplay.begin(), ::tolower);

        // Filter
        if (!searchText.empty() && lowerDisplay.find(searchText) == std::string::npos)
            continue;

        bool isSelected = (selectedKey == kv.first);
        if (ImGui::Selectable(display.c_str(), isSelected))
        {
            selectedKey = kv.first;
        }
    }
    ImGui::EndChild();

    // --- Add button ---
    if (ImGui::Button("Add Passive Skill"))
    {
        if (!selectedKey.empty())
        {
            // Check if already exists
            bool exists = false;
            for (int i = 0; i < passiveSkills.Num(); i++)
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
                SDK::FName fname = lib->Conv_StringToName(
                    SDK::FString(std::wstring(selectedKey.begin(), selectedKey.end()).c_str())
                );

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

