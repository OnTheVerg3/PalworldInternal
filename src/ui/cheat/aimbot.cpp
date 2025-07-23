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
    APalCharacter* Pal;
    FVector2D ScreenPos;
    float Fov;

    AimbotTarget(APalCharacter* p, const FVector2D& s, float f)
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

bool IsValidAimbotTarget(APalCharacter* pal)
{
    if (!pal) {
        if (g_Console) g_Console->cLog("[Aimbot] Rejected: pal is nullptr\n");
        return false;
    }

    auto params = pal->CharacterParameterComponent;
    if (!params) {
        if (g_Console) g_Console->cLog("[Aimbot] Rejected: No CharacterParameterComponent: %s\n", Console::EColor_DEFAULT, pal->GetName().c_str());
        return false;
    }

    if (!params->IndividualParameter) {
        if (g_Console) g_Console->cLog("[Aimbot] Rejected: No IndividualParameter: %s\n", Console::EColor_DEFAULT, pal->GetName().c_str());
        return false;
    }

    // Passes all checks
    return true;
}

AimbotTarget FindBestPalTarget(APlayerController* controller, FVector2D screenCenter, float maxFov)
{
    if (g_Console) g_Console->cLog("[Aimbot] Searching for best Pal target...\n");

    SDK::TArray<SDK::APalCharacter*> pals;
    if (!GetTAllPals(&pals)) {
        if (g_Console) g_Console->cLog("[Aimbot] GetAllPals failed!\n");
        return { nullptr, {}, FLT_MAX };
    }

    if (g_Console) g_Console->cLog("[Aimbot] pals.Num(): %d\n", Console::EColor_DEFAULT, pals.Num());

    float bestFov = maxFov;
    APalCharacter* bestPal = nullptr;
    FVector2D bestScreen{};

    int palCount = 0;

    for (int i = 0; i < pals.Num(); ++i)
    {
        APalCharacter* pal = pals[i];
        if (!pal)
            continue;

        if (!IsValidAimbotTarget(pal))
            continue;
        palCount++;

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

    if (g_Console) g_Console->cLog("[Aimbot] Found %d valid APalCharacters\n", Console::EColor_DEFAULT, palCount);

    if (bestPal && g_Console)
        g_Console->cLog("[Aimbot] Best target: %s | FOV: %.2f\n", Console::EColor_DEFAULT, bestPal->GetName().c_str(), bestFov);
    else if (g_Console)
        g_Console->cLog("[Aimbot] No valid Pal target found in FOV\n");

    // You may want to cast to APalMonsterCharacter* here if your AimbotTarget struct expects that type.
    // If so, check: 
    //    APalMonsterCharacter* monster = dynamic_cast<APalMonsterCharacter*>(bestPal);

    return { bestPal, bestScreen, bestFov };
}



void RunPalAimbot()
{
    if (!cheatState.aimbotEnabled) {
        if (g_Console) g_Console->cLog("[Aimbot] Aimbot disabled\n");
        return;
    }

    APalPlayerCharacter* player = GetPalPlayerCharacter();
    if (!player) {
        if (g_Console) g_Console->cLog("[Aimbot] Player not found\n");
        return;
    }

    // Make sure the controller is the player controller
    APalPlayerController* controller = GetPalPlayerController();
    if (!controller) {
        if (g_Console) g_Console->cLog("[Aimbot] Controller not found\n");
        return;
    }

    // Screen center for FOV calc
    FVector2D screenCenter(ImGui::GetIO().DisplaySize.x / 2.0f, ImGui::GetIO().DisplaySize.y / 2.0f);

    // Find best pal to aim at
    AimbotTarget target = FindBestPalTarget(controller, screenCenter, cheatState.aimbotFov);

    if (!target.Pal) {
        if (g_Console) g_Console->cLog("[Aimbot] No Pal to aim at\n");
        return;
    }

    FVector targetLoc = target.Pal->K2_GetActorLocation();

    // Use controller's current view for calculation
    FVector cameraLoc;
    FRotator cameraRot;
    controller->GetPlayerViewPoint(&cameraLoc, &cameraRot);

    // Calculate desired aim
    FRotator desiredRot = CalcLookAtRotation(cameraLoc, targetLoc);
    desiredRot.Roll = 0.0f; // Only aim Pitch/Yaw

    float smooth = cheatState.aimbotSmooth;
    if (smooth < 0.0f) smooth = 0.0f;
    if (smooth > 1.0f) smooth = 1.0f;

    FRotator newRot;
    if (smooth > 0.0f && smooth < 1.0f) {
        newRot.Pitch = cameraRot.Pitch + (desiredRot.Pitch - cameraRot.Pitch) * smooth;
        newRot.Yaw = cameraRot.Yaw + (desiredRot.Yaw - cameraRot.Yaw) * smooth;
        newRot.Roll = 0.0f;
        controller->SetControlRotation(newRot);

        if (g_Console) g_Console->cLog(
            "[Aimbot] Smooth aim: Pitch %.1f Yaw %.1f (Smooth: %.2f)\n",
            Console::EColor_green, newRot.Pitch, newRot.Yaw, smooth
        );
    }
    else {
        controller->SetControlRotation(desiredRot);

        if (g_Console) g_Console->cLog(
            "[Aimbot] Snap aim: Pitch %.1f Yaw %.1f\n",
            Console::EColor_green, desiredRot.Pitch, desiredRot.Yaw
        );
    }
}




