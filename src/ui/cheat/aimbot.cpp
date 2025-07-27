#include <pch.h>
#include "Engine_classes.hpp"
#include "Pal_classes.hpp"
#include "Aimbot.h"
#include <Windows.h>

using namespace SDK;
using namespace Helper;

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
        return false;
    }

    auto params = pal->CharacterParameterComponent;
    if (!params) {
        return false;
    }

    if (!params->IndividualParameter) {
        return false;
    }

    if (!IsAlive(pal))
    {
        return false;
    }

    // Passes all checks
    return true;
}

AimbotTarget FindBestPalTarget(APlayerController* controller, FVector2D screenCenter, float maxFov)
{
    SDK::TArray<SDK::APalCharacter*> pals;
    if (!GetTAllPals(&pals))
        return { nullptr, {}, FLT_MAX };

    float maxDistance = 35000.0f;
    float bestFov = maxFov;
    APalCharacter* bestPal = nullptr;
    FVector2D bestScreen{};

    for (int i = 0; i < pals.Num(); ++i)
    {
        APalCharacter* pal = pals[i];
        if (!pal || !IsAlive(pal))
            continue;

        if (!IsValidAimbotTarget(pal))
            continue;

        // Validate controller pawn
        if (!controller || !controller->K2_GetPawn())
            continue;

        // Validate distance
        float distance = controller->K2_GetPawn()->GetDistanceTo(pal);
        if (distance > maxDistance)
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
    // Get player and controller
    APalPlayerCharacter* player = GetPalPlayerCharacter();
    if (!player) return;

    APalPlayerController* controller = GetPalPlayerController();
    if (!controller) return;

    // Screen center for FOV
    FVector2D screenCenter(ImGui::GetIO().DisplaySize.x / 2.0f, ImGui::GetIO().DisplaySize.y / 2.0f);

    // Find best target
    AimbotTarget target = FindBestPalTarget(controller, screenCenter, cheatState.aimbotFov);

    // Ensure target is valid and alive
    if (!target.Pal || !IsAlive(target.Pal))
        return;

    // Try to get the head bone location
    FVector targetLoc = FVector(0, 0, 0);
    targetLoc = target.Pal->K2_GetActorLocation(); // default fallback
    USkeletalMeshComponent* mesh = target.Pal->Mesh;

    if (mesh)
    {
        int boneIndex = 6; // Head bone index (adjust if needed)
        if (boneIndex >= 0 && boneIndex < mesh->GetNumBones())
        {
            FName headBoneName = mesh->GetBoneName(boneIndex);

            // Make sure the bone name is valid and the socket exists
            if (headBoneName.ToString().length() > 0 && mesh->DoesSocketExist(headBoneName))
            {
                targetLoc = mesh->GetSocketLocation(headBoneName);
            }
        }
    }

    if (!controller) return;

    // Get current camera location & rotation
    FVector cameraLoc;
    FRotator cameraRot;
    controller->GetPlayerViewPoint(&cameraLoc, &cameraRot);

    // Calculate desired aim rotation
    FRotator desiredRot = CalcLookAtRotation(cameraLoc, targetLoc);
    desiredRot.Roll = 0.0f;

    // Smooth factor
    float smooth = cheatState.aimbotSmooth;
    if (smooth < 0.0f) smooth = 0.0f;
    if (smooth > 1.0f) smooth = 1.0f;

    // Calculate pitch & yaw delta
    float deltaYaw = desiredRot.Yaw - cameraRot.Yaw;
    float deltaPitch = desiredRot.Pitch - cameraRot.Pitch;

    // Normalize angle deltas
    if (deltaYaw > 180.f) deltaYaw -= 360.f;
    if (deltaYaw < -180.f) deltaYaw += 360.f;

    if (deltaPitch > 180.f) deltaPitch -= 360.f;
    if (deltaPitch < -180.f) deltaPitch += 360.f;

    // Apply smooth factor (snapFactor closer to 1 = faster)
    float snapFactor = powf(1.0f - smooth, 0.5f);
    if (snapFactor < 0.01f) snapFactor = 0.01f;
    else if (snapFactor > 1.0f) snapFactor = 1.0f;

    deltaYaw *= snapFactor;
    deltaPitch *= snapFactor;

    // Mouse sensitivity conversion (lower = faster)
    float sensitivity = 0.15f;

    float rawX = deltaYaw / sensitivity;
    float rawY = -deltaPitch / sensitivity;

    int mouseX = static_cast<int>(rawX);
    int mouseY = static_cast<int>(rawY);

    // If smooth = 0 (snap mode) force movement
    if (smooth <= 0.001f)
    {
        if (mouseX == 0 && fabsf(rawX) > 0.01f)
            mouseX = (rawX > 0.0f ? 1 : -1);

        if (mouseY == 0 && fabsf(rawY) > 0.01f)
            mouseY = (rawY > 0.0f ? 1 : -1);
    }

    // Move mouse relative to delta
    MoveMouseRelative(mouseX, mouseY);
}







