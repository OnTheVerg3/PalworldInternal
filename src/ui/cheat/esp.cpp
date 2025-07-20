#include <pch.h>
#include "esp.h"
#include <Windows.h>
#include "libs/ImGui/imgui.h"

#include "Engine_classes.hpp"
#include "Pal_classes.hpp"
#include "GameHelper.h"
#include "database.h"

using namespace Helper;
using namespace SDK;

bool ShouldSkipESPActor(const std::string& rawName);
bool IsJunkActor(const std::string& rawName);

void DrawESP()
{
    if (!cheatState.espEnabled)
        return;

    auto player = GetPalPlayerCharacter();
    if (!player)
        return;

    auto controller = reinterpret_cast<APlayerController*>(player->Controller);

    FVector cameraLocation;
    FRotator cameraRotation;
    controller->GetPlayerViewPoint(&cameraLocation, &cameraRotation);

    UWorld* world = UWorld::GetWorld();
    if (!world || !world->PersistentLevel)
        return;

    for (int i = 0; i < world->PersistentLevel->Actors.Num(); ++i)
    {
        AActor* actor = world->PersistentLevel->Actors[i];
        if (!actor || actor == player)
            continue;

        // Get location and distance
        FVector actorLocation = actor->K2_GetActorLocation();
        float distance = actorLocation.GetDistanceTo(cameraLocation);

        if (distance > cheatState.espDistance)
            continue;

        // Raw name
        std::string rawName = ((UObject*)actor)->GetName();
        OutputDebugStringA(("[DEBUG] Actor name: " + rawName + "\n").c_str());

        if (rawName.find("BP_") == std::string::npos || rawName.find("_C") == std::string::npos)
            continue;

        // FILTERS

        if (ShouldSkipESPActor(rawName))
            continue;


        // Screen projection
        FVector2D screenPos;
        if (!controller->ProjectWorldLocationToScreen(actorLocation, &screenPos, false)) {
            OutputDebugStringA("[-] ProjectWorldLocationToScreen failed\n");
            continue;
        }

        // just making sure we get only BPs shown
        std::string cleanName = rawName;
        size_t bpStart = cleanName.find("BP_");
        if (bpStart != std::string::npos) {
            cleanName = cleanName.substr(bpStart);
            size_t suffix = cleanName.find("_C");
            if (suffix != std::string::npos)
                cleanName = cleanName.substr(0, suffix);
        }

        // Show Boxes
        if (cheatState.espBoxes && !IsJunkActor(rawName) && rawName.find("MapObject") == std::string::npos && rawName.find("Item") == std::string::npos && rawName.find("PalSphere") == std::string::npos)
        {
            // Get actor bounds in world space
            FVector actorLocation = actor->K2_GetActorLocation();
            float halfHeight = 100.0f;

            FVector top = actorLocation + FVector(0, 0, halfHeight);
            FVector bottom = actorLocation - FVector(0, 0, halfHeight);

            // Project to screen space
            FVector2D screenTop, screenBottom;
            if (!controller->ProjectWorldLocationToScreen(top, &screenTop, false)) continue;
            if (!controller->ProjectWorldLocationToScreen(bottom, &screenBottom, false)) continue;

            float height = screenBottom.Y - screenTop.Y;
            float width = height * 0.4f; // aspect ratio approximation

            ImVec2 boxTopLeft(screenTop.X - width / 2, screenTop.Y);
            ImVec2 boxBottomRight(screenTop.X + width / 2, screenBottom.Y);

            ImGui::GetBackgroundDrawList()->AddRect(
                boxTopLeft,
                boxBottomRight,
                IM_COL32(255, 0, 0, 255),
                0.0f,
                0,
                1.5f
            );
        }

        // Show Name and Distance
        if (cheatState.espShowNames || cheatState.espShowDistance && !ShouldSkipESPActor(rawName))
        {
            std::string labelText;

            if (cheatState.espShowNames)
                labelText += cleanName;

            if (cheatState.espShowDistance)
            {
                if (!labelText.empty())
                    labelText += " ";
                char distanceText[32];
                sprintf_s(distanceText, "[%.0f m]", distance / 100.0f);
                labelText += distanceText;
            }

            ImVec2 textSize = ImGui::CalcTextSize(labelText.c_str());

            ImVec2 textPos = ImVec2(
                screenPos.X - textSize.x / 2.0f, // center horizontally
                screenPos.Y                     // draw from Y as-is
            );

            ImGui::GetBackgroundDrawList()->AddText(
                textPos,
                IM_COL32(255, 255, 255, 255),
                labelText.c_str()
            );
        }

    }

    OutputDebugStringA("[+] DrawESP() finished\n");
}

void DrawUActorComponent(TArray<UActorComponent*> Comps, ImColor color)
{
    ImGui::GetBackgroundDrawList()->AddText(nullptr, 16, ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 2), color, "Drawing...");
    if (!Comps.IsValid())
        return;
    for (int i = 0; i < Comps.Num(); i++)
    {

        if (!Comps[i])
            continue;

        ImGui::GetBackgroundDrawList()->AddText(nullptr, 16, ImVec2(10, 10 + (i * 30)), color, Comps[i]->GetFullName().c_str());
    }
}

bool ShouldSkipESPActor(const std::string& rawName)
{
    //(static junk filter)
    if (
        rawName.find("AIController") != std::string::npos ||
        rawName.find("BackWeapon") != std::string::npos ||
        rawName.find("Registra") != std::string::npos ||
        rawName.find("VisualActor") != std::string::npos
        )
        return true;

    if (!cheatState.espShowPals) {
        // Skip actors whose name matches any Pal
        for (auto name : database::palNames) {
            if (rawName.find(name) != std::string::npos) {
                return true;
            }
        }
    }

    if (!cheatState.espShowPickupItems &&
        (
            rawName.find("PickupItem") != std::string::npos ||
            rawName.find("PalSphere") != std::string::npos ||
            rawName.find("Paldium") != std::string::npos ||
            rawName.find("Meat") != std::string::npos ||
            rawName.find("Berry") != std::string::npos ||
            rawName.find("Arrow") != std::string::npos ||
            rawName.find("Egg") != std::string::npos ||
            rawName.find("Wood") != std::string::npos ||
            rawName.find("Stone") != std::string::npos ||
            rawName.find("Ore") != std::string::npos ||
            rawName.find("Leather") != std::string::npos ||
            rawName.find("Fiber") != std::string::npos ||
            rawName.find("Fur") != std::string::npos ||
            rawName.find("Milk") != std::string::npos ||
            rawName.find("Honey") != std::string::npos ||
            rawName.find("Trasure") != std::string::npos ||
            rawName.find("BP_MapObject") != std::string::npos ||
            rawName.find("Seed") != std::string::npos
            ))
    {
        return true;
    }
    if (!cheatState.espShowJunk && IsJunkActor(rawName))
        return true;


    return false;
}

bool HasKeyword(const std::string& str, const std::initializer_list<const char*> keywords) {
    for (const char* key : keywords)
        if (str.find(key) != std::string::npos) return true;
    return false;
}

bool IsJunkActor(const std::string& rawName)
{
    return
        rawName.find("Lamp") != std::string::npos ||
        rawName.find("NPCSpawner") != std::string::npos ||
        rawName.find("Dummy") != std::string::npos ||
        rawName.find("Glider") != std::string::npos ||
        rawName.find("Grappling") != std::string::npos ||
        rawName.find("Pickaxe") != std::string::npos ||
        rawName.find("gun") != std::string::npos ||
        rawName.find("Rifle") != std::string::npos ||
        rawName.find("Tower") != std::string::npos ||
        rawName.find("Adjust") != std::string::npos ||
        rawName.find("Dungeon") != std::string::npos ||
        rawName.find("Builder") != std::string::npos ||
        rawName.find("BuildObject") != std::string::npos ||
        rawName.find("PalEgg") != std::string::npos || 
        rawName.find("Farm") != std::string::npos ||
        rawName.find("SkillFruit") != std::string::npos ||
        rawName.find("Mining") != std::string::npos ||
        rawName.find("FishShadow") != std::string::npos ||
        rawName.find("NPC") != std::string::npos ||
        rawName.find("Hammer") != std::string::npos ||
        rawName.find("Axe") != std::string::npos ||
        rawName.find("BP_Bat") != std::string::npos ||
        rawName.find("DamagableRock") != std::string::npos;
}
