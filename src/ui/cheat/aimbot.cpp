#include <pch.h>
#include "Engine_classes.hpp"
#include "Pal_classes.hpp"
#include "Engine.h"
#include "Aimbot.h"

using namespace SDK;
using namespace Helper;
using namespace DX11Base;


struct AimbotTarget
{
    APalMonsterCharacter* Pal;
    FVector2D ScreenPos;
    float Fov;

    AimbotTarget(APalMonsterCharacter* p, const FVector2D& s, float f)
        : Pal(p), ScreenPos(s), Fov(f) {
    }
};

FRotator CalcLookAtRotation(const SDK::FVector& from, const SDK::FVector& to)
{
    SDK::FVector dir = to - from;
    float yaw = atan2f(dir.Y, dir.X) * (180.0f / 3.14159265f);
    float dist = sqrtf(dir.X * dir.X + dir.Y * dir.Y);
    float pitch = atan2f(dir.Z, dist) * (180.0f / 3.14159265f);
    return FRotator(pitch, yaw, 0.0f); // Roll is usually zero
}

bool IsValidAimbotTarget(APalMonsterCharacter* pal)
{
    if (!pal)
        if (g_Console) g_Console->cLog("[Aimbot] [IsValidAimbotTarget] pal is null\n");
        return false;

    auto params = pal->CharacterParameterComponent;
    if (!params || !params->IndividualParameter)
        return false;

    return true;
}

AimbotTarget FindBestPalTarget(APlayerController* controller, FVector2D screenCenter, float maxFov)
{
    if (g_Console) g_Console->cLog("[Aimbot] Searching for best Pal target...\n");

    UWorld* world = UWorld::GetWorld();
    if (!world || !world->PersistentLevel)
        if (g_Console) g_Console->cLog("[Aimbot] UWorld or PersistentLevel is null\n");
        return { nullptr, {}, FLT_MAX };

    float bestFov = maxFov;
    APalMonsterCharacter* bestPal = nullptr;
    FVector2D bestScreen{};

    for (int i = 0; i < world->PersistentLevel->Actors.Num(); ++i)
    {
        AActor* actor = world->PersistentLevel->Actors[i];
        if (!actor || !actor->IsA(APalMonsterCharacter::StaticClass()))
            continue;

        auto pal = static_cast<APalMonsterCharacter*>(actor);
        if (!IsValidAimbotTarget(pal))
            continue;

        FVector2D screenPos;
        if (!controller->ProjectWorldLocationToScreen(pal->K2_GetActorLocation(), &screenPos, false))
            continue;

        float fov = GetDistance(screenPos, screenCenter);
        if (fov < bestFov)
        {
            bestFov = fov;
            bestPal = pal;
            bestScreen = screenPos;
        }
    }

    if (bestPal && g_Console)
        g_Console->cLog("[Aimbot] Found best target: %s, FOV: %.2f\n", Console::EColor_DEFAULT, bestPal->GetName().c_str(), bestFov);
    else if (g_Console)
        g_Console->cLog("[Aimbot] No valid Pal target found in FOV\n");

    return { bestPal, bestScreen, bestFov };
}

void RunPalAimbot()
{
    if (!cheatState.aimbotEnabled)
        if (g_Console) g_Console->cLog("[Aimbot] Aimbot disabled\n");
        return;

    APalPlayerCharacter* player = GetPalPlayerCharacter();
    if (!player)
        return;

    APlayerController* controller = reinterpret_cast<APlayerController*>(player->Controller);
    if (!controller)
        if (g_Console) g_Console->cLog("[Aimbot] Controller not found\n");
        return;

    FVector2D screenCenter(ImGui::GetIO().DisplaySize.x / 2.0f, ImGui::GetIO().DisplaySize.y / 2.0f);

    // Find the best Pal to aim at
    AimbotTarget target = FindBestPalTarget(controller, screenCenter, cheatState.aimbotFov);

    if (!target.Pal)
        if (g_Console) g_Console->cLog("[Aimbot] No Pal to aim at\n");
        return;

    // Calculate rotation to target
    FVector targetLoc = target.Pal->K2_GetActorLocation();
    FVector cameraLoc;
    FRotator cameraRot;
    controller->GetPlayerViewPoint(&cameraLoc, &cameraRot);
    FRotator desiredRot = CalcLookAtRotation(cameraLoc, targetLoc);

    if (g_Console)
        g_Console->cLog("[Aimbot] Aiming at Pal: %s\n", Console::EColor_DEFAULT, target.Pal->GetName().c_str());

    // Smooth or snap
    if (cheatState.aimbotSmooth > 0.0f && cheatState.aimbotSmooth < 1.0f)
    {
        FRotator newRot;
        newRot.Pitch = cameraRot.Pitch + (desiredRot.Pitch - cameraRot.Pitch) * cheatState.aimbotSmooth;
        newRot.Yaw = cameraRot.Yaw + (desiredRot.Yaw - cameraRot.Yaw) * cheatState.aimbotSmooth;
        newRot.Roll = cameraRot.Roll + (desiredRot.Roll - cameraRot.Roll) * cheatState.aimbotSmooth;
        controller->SetControlRotation(newRot);
        if (g_Console)
            g_Console->cLog("[Aimbot] Smooth aim applied (%.2f)\n", Console::EColor_DEFAULT, cheatState.aimbotSmooth);
    }
    else
    {
        controller->SetControlRotation(desiredRot);
    }
}

