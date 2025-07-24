#include <pch.h>
#include "Engine_classes.hpp"
#include "Pal_classes.hpp"
#include "Engine.h"
#include "Aimbot.h"
#include <Windows.h>

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
    SDK::TArray<SDK::APalCharacter*> pals;
    if (!GetTAllPals(&pals)) {
        return { nullptr, {}, FLT_MAX };
    }

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

    return { bestPal, bestScreen, bestFov };
}

void MoveMouseRelative(int dx, int dy)
{
    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dx = dx;
    input.mi.dy = dy;
    input.mi.dwFlags = MOUSEEVENTF_MOVE;
    SendInput(1, &input, sizeof(INPUT));
}

void RunPalAimbot()
{
    APalPlayerCharacter* player = GetPalPlayerCharacter();
    if (!player) return;

    APalPlayerController* controller = GetPalPlayerController();
    if (!controller) return;

    FVector2D screenCenter(ImGui::GetIO().DisplaySize.x / 2.0f, ImGui::GetIO().DisplaySize.y / 2.0f);
    AimbotTarget target = FindBestPalTarget(controller, screenCenter, cheatState.aimbotFov);
    if (!target.Pal) return;

    USkeletalMeshComponent* mesh = target.Pal->Mesh;
    FVector targetLoc;

    if (mesh)
    {
        FName headBoneName = mesh->GetBoneName(6); // index 6 = Head
        if (mesh->DoesSocketExist(headBoneName))
        {
            targetLoc = mesh->GetSocketLocation(headBoneName);
        }
    }
    else
    {
        targetLoc = target.Pal->K2_GetActorLocation(); // Fallback
    }


    FVector cameraLoc;
    FRotator cameraRot;
    controller->GetPlayerViewPoint(&cameraLoc, &cameraRot);

    FRotator desiredRot = CalcLookAtRotation(cameraLoc, targetLoc);
    desiredRot.Roll = 0.0f;

    float smooth = cheatState.aimbotSmooth;
    if (smooth < 0.0f) smooth = 0.0f;
    if (smooth > 1.0f) smooth = 1.0f;

    float deltaYaw = desiredRot.Yaw - cameraRot.Yaw;
    float deltaPitch = desiredRot.Pitch - cameraRot.Pitch;

    // Normalize
    if (deltaYaw > 180.f) deltaYaw -= 360.f;
    if (deltaYaw < -180.f) deltaYaw += 360.f;

    if (deltaPitch > 180.f) deltaPitch -= 360.f;
    if (deltaPitch < -180.f) deltaPitch += 360.f;

    float snapFactor = powf(1.0f - cheatState.aimbotSmooth, 0.5f);
    if (snapFactor < 0.01f) snapFactor = 0.01f;
    else if (snapFactor > 1.0f) snapFactor = 1.0f;

    deltaYaw *= snapFactor;
    deltaPitch *= snapFactor;

    // Mouse sensitivity conversion
    float sensitivity = 0.25f; // adjust to your game sensitivity

    float rawX = deltaYaw / sensitivity;
    float rawY = -deltaPitch / sensitivity;

    int mouseX = static_cast<int>(rawX);
    int mouseY = static_cast<int>(rawY);

    // ✅ If smooth is 0 and we're supposed to snap — force at least 1px movement
    if (cheatState.aimbotSmooth <= 0.001f)
    {
        if (mouseX == 0 && fabsf(rawX) > 0.01f)
            mouseX = (rawX > 0.0f ? 1 : -1);

        if (mouseY == 0 && fabsf(rawY) > 0.01f)
            mouseY = (rawY > 0.0f ? 1 : -1);
    }

    MoveMouseRelative(mouseX, mouseY);
}







