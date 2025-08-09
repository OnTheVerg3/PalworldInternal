#include <pch.h>
#include "esp.h"
#include <Windows.h>
#include "libs/ImGui/imgui.h"
#include "Engine.h"

#include "Engine_classes.hpp"
#include "Pal_classes.hpp"
#include "GameHelper.h"
#include "database.h"

using namespace Helper;
using namespace SDK;
using namespace DX11Base;


std::string GetCleanPalName(const std::string& rawName) {
    size_t start = 0;

    // Remove "BP_" prefix if present
    if (rawName.find("BP_") == 0)
        start += 3;

    // Remove "NPC_" prefix if present (after BP_ or at start)
    if (rawName.find("NPC_", start) == start)
        start += 4;

    // Find _C (typical UE4/5 suffix)
    size_t end = rawName.find("_C", start);
    std::string coreName = (end != std::string::npos)
        ? rawName.substr(start, end - start)
        : rawName.substr(start);

    // Remove trailing digits (e.g. "_123456")
    while (!coreName.empty() && std::isdigit(coreName.back()))
        coreName.pop_back();

    // Remove trailing underscores after digits
    while (!coreName.empty() && coreName.back() == '_')
        coreName.pop_back();

    return coreName;
}

void DrawRelicESPText(ImVec2 screenPos, const char* nameLabel, float distance)
{
    ImVec2 nameSize = ImGui::CalcTextSize(nameLabel);

    // Center name
    screenPos.x -= nameSize.x / 2.0f;

    // Background + shadow for name
    ImVec2 nameBgMin = ImVec2(screenPos.x - 4, screenPos.y - 2);
    ImVec2 nameBgMax = ImVec2(screenPos.x + nameSize.x + 4, screenPos.y + nameSize.y + 2);
    ImGui::GetBackgroundDrawList()->AddRectFilled(nameBgMin, nameBgMax, IM_COL32(0, 0, 0, 140), 4.0f);
    ImGui::GetBackgroundDrawList()->AddText(ImVec2(screenPos.x + 1, screenPos.y + 1), IM_COL32(0, 0, 0, 200), nameLabel);
    ImGui::GetBackgroundDrawList()->AddText(screenPos, IM_COL32(255, 255, 100, 255), nameLabel);

    // Advance Y for distance label
    if (cheatState.espShowDistance)
    {
        char distLabel[32];
        snprintf(distLabel, sizeof(distLabel), "[%.1fm]", distance / 100.0f);

        ImVec2 distSize = ImGui::CalcTextSize(distLabel);
        ImVec2 distPos = ImVec2(screenPos.x + nameSize.x / 2.0f - distSize.x / 2.0f, screenPos.y + nameSize.y + 4);

        ImVec2 distBgMin = ImVec2(distPos.x - 4, distPos.y - 2);
        ImVec2 distBgMax = ImVec2(distPos.x + distSize.x + 4, distPos.y + distSize.y + 2);
        ImGui::GetBackgroundDrawList()->AddRectFilled(distBgMin, distBgMax, IM_COL32(0, 0, 0, 140), 4.0f);
        ImGui::GetBackgroundDrawList()->AddText(ImVec2(distPos.x + 1, distPos.y + 1), IM_COL32(0, 0, 0, 200), distLabel);
        ImGui::GetBackgroundDrawList()->AddText(distPos, IM_COL32(120, 255, 120, 255), distLabel);
    }
}

void DrawESPText(ImVec2 screenPos, const char* name, float distance = -1.0f, ImU32 color = IM_COL32(255, 255, 255, 255))
{
    // Skip entire ESP if out of range
    if (distance >= 0.0f && distance > cheatState.espDistance)
        return;

    ImVec2 nameSize = ImGui::CalcTextSize(name);
    ImVec2 pos = screenPos;
    pos.x -= nameSize.x / 2.0f;

    // Outline
    ImGui::GetBackgroundDrawList()->AddText(ImVec2(pos.x + 1, pos.y + 1),
        IM_COL32(0, 0, 0, 200), name);

    // Main text with passed color
    ImGui::GetBackgroundDrawList()->AddText(pos, color, name);

    // Optional distance label
    if (cheatState.espShowDistance && distance >= 0.0f && distance <= cheatState.espDistance)
    {
        char distText[32];
        snprintf(distText, sizeof(distText), "[%.0fm]", distance / 100.0f);

        ImVec2 distSize = ImGui::CalcTextSize(distText);
        ImVec2 distPos = ImVec2(screenPos.x - distSize.x / 2.0f, pos.y + nameSize.y + 4);

        ImGui::GetBackgroundDrawList()->AddRectFilled(
            ImVec2(distPos.x - 4, distPos.y - 2),
            ImVec2(distPos.x + distSize.x + 4, distPos.y + distSize.y + 2),
            IM_COL32(0, 0, 0, 140), 4.0f);

        ImGui::GetBackgroundDrawList()->AddText(ImVec2(distPos.x + 1, distPos.y + 1),
            IM_COL32(0, 0, 0, 200), distText);

        ImGui::GetBackgroundDrawList()->AddText(distPos,
            IM_COL32(120, 255, 120, 255), distText);
    }
}

void DrawPalESP()
{
    if (!cheatState.espEnabled || !cheatState.espShowPals)
        return;

    auto drawList = ImGui::GetBackgroundDrawList(); // Making sure that ImGui is ready
    if (!drawList)
        return;

    APalPlayerCharacter* player = GetPalPlayerCharacter();
    if (!player)
        return;

    auto controller = reinterpret_cast<APlayerController*>(player->Controller);
    if (!controller)
        return;

    // Get all Pals efficiently
    SDK::TArray<SDK::APalCharacter*> pals;
    if (!Helper::GetTAllPals(&pals))
        return;

    FVector cameraLocation;
    FRotator cameraRotation;
    controller->GetPlayerViewPoint(&cameraLocation, &cameraRotation);

    for (int i = 0; i < pals.Num(); ++i)
    {
        if (!pals.IsValidIndex(i))
            continue;

        SDK::APalCharacter* pal = pals[i];
        if (!pal)
            continue;

        if (pal == player)
            continue;

        bool baseWorker = IsABaseWorker(pal, true);
        bool isAlive = IsAlive(pal);
        bool isTamed = IsTamed(pal);

        if (baseWorker || !isAlive || isTamed)
            continue;

        // Get location and distance
        FVector palLocation = pal->K2_GetActorLocation();
        float distance = palLocation.GetDistanceTo(cameraLocation);

        if (distance > cheatState.espDistance)
            continue;

        // Project to screen
        FVector2D screenPos;
        if (!controller->ProjectWorldLocationToScreen(palLocation, &screenPos, false))
            continue;

        UPalCharacterParameterComponent* params = pal->CharacterParameterComponent;

        const bool isVisible = Helper::HasCameraLOS_Kismet(controller, player, pal);

        if (cheatState.espShowPalHealth && params)
        {
            float currentHealth = 0.0f;
            float maxHealth = 1.0f;

            if (params)
            {
                auto hp = params->GetHP();
                auto maxHp = params->GetMaxHP();

                currentHealth = (hp.Value >= 0) ? static_cast<float>(hp.Value) : 0.0f;
                maxHealth = (maxHp.Value > 0) ? static_cast<float>(maxHp.Value) : 1.0f;
            }

            float healthFrac = maxHealth > 0 ? (currentHealth / maxHealth) : 0.0f;
            float barWidth = 100.0f;
            float barHeight = 6.0f;
            ImVec2 barPos(screenPos.X - barWidth / 2, screenPos.Y + 16); // 16 px below text

            // Background (grey)
            ImGui::GetBackgroundDrawList()->AddRectFilled(
                barPos, ImVec2(barPos.x + barWidth, barPos.y + barHeight),
                IM_COL32(60, 60, 60, 200), 3.0f);

            // Foreground (green->red)
            ImU32 hpColor = IM_COL32(
                static_cast<int>((1.0f - healthFrac) * 220),
                static_cast<int>(healthFrac * 220),
                30, 230);

            ImGui::GetBackgroundDrawList()->AddRectFilled(
                barPos, ImVec2(barPos.x + barWidth * healthFrac, barPos.y + barHeight),
                hpColor, 3.0f);
        }

        if (cheatState.espBoxes)
        {
            float boxHeight = 100.0f;
            float boxWidth = 40.0f;

            ImVec2 topLeft = ImVec2(screenPos.X - boxWidth / 2.0f, screenPos.Y - boxHeight / 2.0f);
            ImVec2 bottomRight = ImVec2(screenPos.X + boxWidth / 2.0f, screenPos.Y + boxHeight / 2.0f);

            ImGui::GetBackgroundDrawList()->AddRect(
                topLeft, bottomRight, IM_COL32(255, 255, 255, 180), 2.0f, 0, 1.5f);
        }


        std::string palName = "Unknown";
        if (pal)
        {
            std::string rawName = pal->GetName(); //Valid check
            if (!rawName.empty())
                palName = GetCleanPalName(rawName);
        }

        // Show name
        char label[128];
        snprintf(label, sizeof(label), "%s", palName.c_str());
        DrawESPText(ImVec2(screenPos.X, screenPos.Y),
            palName.c_str(),
            distance,
            isVisible ? IM_COL32(200, 255, 200, 255) : IM_COL32(160, 160, 160, 200));

    }
}

std::vector<AActor*> cachedRelics;
float lastRelicScan = 0.0f;
const float scanInterval = 5.0f;

void UpdateRelicCache(UWorld* world)
{
    cachedRelics.clear();
    if (!world) return;

    const auto& levels = world->Levels;
    if (levels.Num() == 0)
        return;

    for (int32 i = 0; i < levels.Num(); ++i)
    {
        ULevel* level = levels[i];
        if (!level) continue;

        const auto& actors = level->Actors;
        for (int32 j = 0; j < actors.Num(); ++j)
        {

            AActor* actor = actors[j];
            if (!actor || !actor->Class) continue;

            if(actor->bHidden)
				continue;

            std::string className = actor->Class->GetName();
            if (className == "BP_LevelObject_Relic_C")
            {
                cachedRelics.push_back(actor);
            }
        }
    }
}

void DrawRelicESP()
{
    if (!cheatState.espEnabled || !cheatState.espShowRelics)
        return;

    UWorld* world = UWorld::GetWorld();
    if (!world) return;

    if (world->Levels.Num() == 0) {
        cachedRelics.clear();
        return;
    }

    if (!world->OwningGameInstance)
    {
        cachedRelics.clear();
        return;
    }

    double currentTime = UKismetSystemLibrary::GetGameTimeInSeconds(world);
    if (currentTime - lastRelicScan > scanInterval)
    {
        UpdateRelicCache(world);
        lastRelicScan = currentTime;
    }



    APalPlayerCharacter* player = GetPalPlayerCharacter();
    if (!player)
    {
        cachedRelics.clear();
        return;
    }

    APlayerController* controller = reinterpret_cast<APlayerController*>(player->Controller);
    if (!controller)
    {
        cachedRelics.clear();
        return;
    }

    FVector cameraLoc;
    FRotator cameraRot;
    controller->GetPlayerViewPoint(&cameraLoc, &cameraRot);

    // Remove null actors before using them
    // Cleanup only null pointers
    cachedRelics.erase(
        std::remove_if(
            cachedRelics.begin(),
            cachedRelics.end(),
            [](AActor* actor) {
                return actor == nullptr;
            }
        ),
        cachedRelics.end()
    );


    for (auto it = cachedRelics.begin(); it != cachedRelics.end(); )
    {
        AActor* actor = *it;

        if (!actor || reinterpret_cast<uintptr_t>(actor) < 0x10000)
        {
            it = cachedRelics.erase(it);
            continue;
        }

        // Check if Class pointer is valid
        uintptr_t classPtr = reinterpret_cast<uintptr_t>(actor->Class);
        if (classPtr < 0x10000)
        {
            it = cachedRelics.erase(it);
            continue;
        }

        //trying to get the name safely because it crashes always
        std::string className;
        try
        {
            className = actor->Class->GetName();
        }
        catch (...)
        {
            it = cachedRelics.erase(it);
            continue;
        }

        if (className == "BP_LevelObject_Relic_C" || className.find("PalLevelObjectRelic") != std::string::npos)
        {
            APalLevelObjectObtainable* obtainable = reinterpret_cast<APalLevelObjectObtainable*>(actor);
            if (obtainable && obtainable->bPickedInClient)
            {
                ++it;
                continue;
            }
        }


        FVector relicLoc = actor->K2_GetActorLocation();
        float distance = relicLoc.GetDistanceTo(cameraLoc);

        if (distance <= cheatState.espDistance)
        {
            FVector2D screenPos;
            if (controller->ProjectWorldLocationToScreen(relicLoc, &screenPos, false))
            {
                char label[64];
                snprintf(label, sizeof(label), "Relic");
                DrawRelicESPText(ImVec2(screenPos.X, screenPos.Y), label, distance);
            }
        }
        ++it;
    }
}

void DebugNearbyActors(float radius)
{
    uintptr_t moduleBase = (uintptr_t)GetModuleHandle(NULL); // Only works if injected into game process
    uintptr_t GObjectsAddress = moduleBase + Offsets::GObjects;
    TUObjectArray* GObjects = reinterpret_cast<TUObjectArray*>(GObjectsAddress);
    if (!GObjects) return;

    int found = 0;
    for (int i = 0; i < GObjects->Num(); ++i)
    {
        UObject* obj = GObjects->GetByIndex(i);
        if (!obj) continue;

        std::string objName = obj->GetName();
        std::string className = "Unknown";
        if (obj->Class)
            className = obj->Class->GetName();

        // Search for keywords
        if (
            objName.find("Relic") != std::string::npos ||
            className.find("Relic") != std::string::npos ||
            objName.find("LevelObject") != std::string::npos ||
            className.find("LevelObject") != std::string::npos ||
            objName.find("Obtainable") != std::string::npos ||
            className.find("Obtainable") != std::string::npos ||
            objName.find("Pickup") != std::string::npos ||
            className.find("Pickup") != std::string::npos
            )
        {
            g_Console->cLog("[RelicDebug] Name: %s | Class: %s | Ptr: 0x%p\n",
                Console::EColor_yellow,
                objName.c_str(),
                className.c_str(),
                obj);
            found++;
        }
    }
    g_Console->cLog("[RelicDebug] Total matching objects: %d\n", Console::EColor_green, found);
}





