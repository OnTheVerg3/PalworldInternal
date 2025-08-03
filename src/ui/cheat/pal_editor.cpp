#include <pch.h>
#include "pal_editor.h"
#include "cheat_state.h"

using namespace Helper;

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


bool GetAllTamedPals(std::vector<SDK::APalCharacter*>& outResult)
{
    outResult.clear();

    SDK::TArray<SDK::APalCharacter*> allPals;
    if (!GetTAllPals(&allPals))
    {
        printf("[DEBUG] GetTAllPals() failed!\n");
        return false;
    }

    printf("[DEBUG] Found %d total pals\n", allPals.Num());

    for (int i = 0; i < allPals.Num(); ++i)
    {
        if (!allPals.IsValidIndex(i)) continue;

        SDK::APalCharacter* pal = allPals[i];
        if (!pal) continue;

        bool isTamed = IsTamed(pal);
        bool isAlive = IsAlive(pal);
        bool baseWorker = IsABaseWorker(pal, true);

        std::string name = GetCleanPalName2(pal->GetName());

        printf("[DEBUG] %s - Alive:%d Tamed:%d BaseWorker:%d\n",
            name.c_str(), isAlive, isTamed, baseWorker);

        if (isTamed && isAlive && !baseWorker)
        {
            printf("[DEBUG] -> Adding tamed pal: %s\n", name.c_str());
            outResult.push_back(pal);
        }
    }

    printf("[DEBUG] Final vector size: %zu\n", outResult.size());
    return !outResult.empty();
}


