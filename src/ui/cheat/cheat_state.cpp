#include <pch.h>
#include "cheat_state.h"
#include "Engine_classes.hpp"
#include "Pal_classes.hpp"
#include "Menu.h"

using namespace SDK;
using namespace Helper;

std::vector<SDK::APalPlayerCharacter*> g_PlayerList;
SDK::APalPlayerCharacter* selectedPlayer = nullptr;


float GetDistanceToActor(AActor* pLocal, AActor* pTarget)
{
	if (!pLocal || !pTarget)
		return -1.f;

	FVector pLocation = pLocal->K2_GetActorLocation();
	FVector pTargetLocation = pTarget->K2_GetActorLocation();
	double distance = sqrt(pow(pTargetLocation.X - pLocation.X, 2.0) + pow(pTargetLocation.Y - pLocation.Y, 2.0) + pow(pTargetLocation.Z - pLocation.Z, 2.0));

	return distance / 100.0f;
}

void ChangeSpeed(float speed)
{
    APalPlayerCharacter* pCharacter = GetPalPlayerCharacter();

    auto movement = pCharacter->CharacterMovement;

    movement->MaxWalkSpeed = speed;
    movement->MaxAcceleration = 1000 * speed;
}

void ChangeWorldSpeed(float speed)
{
	UWorld* pWorld = UWorld::GetWorld();
	if (!pWorld)
		return;

	ULevel* pLevel = pWorld->PersistentLevel;
	if (!pLevel)
		return;

	AWorldSettings* pWorldSettings = pLevel->WorldSettings;
	if (!pWorldSettings)
		return;

	pWorld->PersistentLevel->WorldSettings->TimeDilation = speed;
}


void SetPlayerAttackParam()
{
	APalPlayerCharacter* pPalPlayerCharacter = GetPalPlayerCharacter();
	if (!pPalPlayerCharacter)
		return;

	UPalCharacterParameterComponent* pParams = pPalPlayerCharacter->CharacterParameterComponent;
	if (!pParams)
		return;

	int value = cheatState.attack * 50;
	pParams->AttackUp = value;
}


void SetPlayerDefenceParam()
{
	APalPlayerCharacter* pPalPlayerCharacter = GetPalPlayerCharacter();
	if (!pPalPlayerCharacter)
		return;

	UPalCharacterParameterComponent* pParams = pPalPlayerCharacter->CharacterParameterComponent;
	if (!pParams)
		return;

	int value = cheatState.defence * 50;

	pParams->DefenseUp = cheatState.defence;
}

void SetPlayerInventoryWeight()
{
	UPalPlayerInventoryData* pInventory = GetInventoryComponent();
	if (!pInventory)
		return;

	pInventory->MaxInventoryWeight= cheatState.weight;
}

void SetInfiniteAmmo()
{
	APalPlayerCharacter* pPalCharacter = GetPalPlayerCharacter();
	if (!pPalCharacter)
		return;

	UPalShooterComponent* pShootComponent = pPalCharacter->ShooterComponent;
	if (!pShootComponent)
		return;

	APalWeaponBase* pWeapon = pShootComponent->HasWeapon;

	pWeapon->IsRequiredBullet = cheatState.infAmmo ? false : true;

}

void ResetStamina()
{
	//TODO: Crashes game on multiplayer
	if (!cheatState.infStamina)
		return;

	APalPlayerCharacter* pPalCharacter = GetPalPlayerCharacter();
	if (!pPalCharacter)
		return;

	UPalCharacterParameterComponent* pParams = pPalCharacter->CharacterParameterComponent;
	if (!pParams)
		return;

	pParams->ResetSP();
}

void AddItemToInventoryByName(std::string itemName, int count)
{
	// obtain lib instance
	static UKismetStringLibrary* lib = UKismetStringLibrary::GetDefaultObj();

	APalPlayerCharacter* pPalPlayerCharacter = GetPalPlayerCharacter();
	APalPlayerState* pPalPlayerState = GetPalPlayerState();
	if (!pPalPlayerCharacter || !pPalPlayerState)
		return;

	SDK::UPalPlayerInventoryData* pInventoryData = pPalPlayerState->GetInventoryData();
	if (!pInventoryData)
		return;

	FName Name = lib->Conv_StringToName(FString(std::wstring(itemName.begin(), itemName.end()).c_str()));
	pInventoryData->RequestAddItem(Name, count, true);
	pInventoryData->AddItem_ServerInternal(Name, count, true, 10.0f);

}

void TeleportPlayerTo(const FVector& pos)
{
	APalPlayerState* pPalPlayerState = GetPalPlayerState();
	APalPlayerController* pPalPlayerController = GetPalPlayerController();

	if (!pPalPlayerController || !pPalPlayerState)
		return;

	// Adjust to avoid spawning inside terrain
	FVector safeLocation = FVector(pos.X, pos.Y + 100.0f, pos.Z);
	FQuat defaultRotation(0.f, 0.f, 0.f, 1.f);
	// Get player unique ID (needed for server call)
	FGuid guid = pPalPlayerState->PlayerUId;

	// Register the new location as respawn point
	pPalPlayerController->Transmitter->Player->RegisterRespawnPoint_ToServer(guid, safeLocation, defaultRotation);

	// Trigger the respawn to teleport
	pPalPlayerState->RequestRespawn();
}

void SetDemiGodMode()
{
	APalPlayerCharacter* pPalPlayerCharacter = GetPalPlayerCharacter();
	if (!pPalPlayerCharacter)
		return;

	UPalCharacterParameterComponent* pParams = pPalPlayerCharacter->CharacterParameterComponent;
	if (!pParams)
		return;

	UPalIndividualCharacterParameter* mIVs = pParams->IndividualParameter;
	if (!mIVs)
		return;

	auto sParams = mIVs->SaveParameter;

	sParams.Hp.Value = sParams.MaxHP.Value;
	sParams.MP.Value = sParams.MaxMP.Value;
	sParams.FullStomach = sParams.MaxFullStomach;
	sParams.PhysicalHealth = EPalStatusPhysicalHealthType::Healthful;
	sParams.SanityValue = 100.f;
	sParams.HungerType = EPalStatusHungerType::Default;
}

void SetCameraFov()
{
	APalPlayerCharacter* player = GetPalPlayerCharacter();

	auto cameraComp = player->FollowCamera;
	cameraComp->WalkFOV = cheatState.cameraFov;
	cameraComp->SprintFOV = cheatState.cameraFov;
	cameraComp->AimFOV = cheatState.cameraFov;
}

void SetCraftSpeed()
{
	APalPlayerCharacter* pPalCharacter = GetPalPlayerCharacter();
	if (!pPalCharacter)
		return;

	UPalCharacterParameterComponent* pParams = pPalCharacter->CharacterParameterComponent;
	if (!pParams)
		return;

	UPalIndividualCharacterParameter* ivParams = pParams->IndividualParameter;
	if (!ivParams)
		return;

	FPalIndividualCharacterSaveParameter sParams = ivParams->SaveParameter;
	TArray<FFloatContainer_FloatPair> mCraftSpeedArray = sParams.CraftSpeedRates.Values;


	if (mCraftSpeedArray.Num() > 0)
		mCraftSpeedArray[0].Value = cheatState.craftSpeed;

}

void SetPalCraftSpeed()
{

	SDK::TArray<SDK::APalCharacter*> mPals;
	if (GetTAllPals(&mPals))
		return;

	DWORD palsCount = mPals.Num();
	for (int i = 0; i < palsCount; i++)
	{
		SDK::APalCharacter* obj = mPals[i];
		if (!obj || !obj->IsA(SDK::APalMonsterCharacter::StaticClass()) || IsABaseWorker(obj))
			continue;

		UPalCharacterParameterComponent* pParams = obj->CharacterParameterComponent;
		if (!pParams)
			return;

		UPalIndividualCharacterParameter* ivParams = pParams->IndividualParameter;
		if (!ivParams)
			return;

		FPalIndividualCharacterSaveParameter sParams = ivParams->SaveParameter;
		TArray<FFloatContainer_FloatPair> mCraftSpeedArray = sParams.CraftSpeedRates.Values;

		if (mCraftSpeedArray.Num() > 0)
			mCraftSpeedArray[0].Value = cheatState.palCraftSpeed;
	}
}

void AddTechPoints()
{
	APalPlayerState* mPlayerState = GetPalPlayerState();
	if (!mPlayerState)
		return;

	UPalTechnologyData* pTechData = mPlayerState->TechnologyData;
	if (!pTechData)
		return;

	pTechData->TechnologyPoint += cheatState.techPoints;
}

//	
void AddAncientTechPoints()
{
	APalPlayerState* mPlayerState = GetPalPlayerState();
	if (!mPlayerState)
		return;

	UPalTechnologyData* pTechData = mPlayerState->TechnologyData;
	if (!pTechData)
		return;

	pTechData->bossTechnologyPoint += cheatState.aTechPoints;
}

void AddWaypointLocation(const std::string& wpName)
{
	APalCharacter* pPalCharacter = GetPalPlayerCharacter();
	if (!pPalCharacter) return;

	SDK::FVector wpLocation = pPalCharacter->K2_GetActorLocation();
	SDK::FRotator wpRotation = pPalCharacter->K2_GetActorRotation();

	g_Waypoints.emplace_back("[WAYPOINT] " + wpName, wpLocation, wpRotation);
}

bool RemoveWaypointLocationByName(const std::string& wpName)
{
	for (auto it = g_Waypoints.begin(); it != g_Waypoints.end(); ++it)
	{
		if (it->waypointName == wpName)
		{
			g_Waypoints.erase(it);
			return true;
		}
	}
	return false;
}


//TODO: Implement in the future
/*void RenderWaypointsToScreen()
{
	if (!cheatState.espShowWaypoints)
		return;
	APalCharacter* pPalCharacater = GetPalPlayerCharacter();
	APalPlayerController* pPalController = GetPalPlayerController();
	if (!pPalCharacater || !pPalController)
		return;

	ImDrawList* draw = ImGui::GetWindowDrawList();

	for (auto waypoint : g_Waypoints)
	{
		FVector2D vScreen;
		if (!pPalController->ProjectWorldLocationToScreen(waypoint.waypointLocation, &vScreen, false))
			continue;

		auto color = ImColor(1.0f, 1.0f, 1.0f, 1.0f);

		draw->AddText(
			nullptr,                   
			24.0f,                  
			ImVec2(vScreen.X, vScreen.Y), 
			ImColor(155, 255, 255),   
			waypoint.waypointName.c_str()
		);
	}
}*/



//Work in Progress
/*void SpawnSimplePal()
{
	// Step 1: References
	APalPlayerCharacter* playerChar = GetPalPlayerCharacter();
	APalPlayerController* controller = GetPalPlayerController();
	UWorld* world = UWorld::GetWorld();
	UPalUtility* utility = SDK::UPalUtility::GetDefaultObj();

	if (!controller || !controller->Transmitter || !controller->Transmitter->NetworkIndividualComponent)
	{
		OutputDebugStringA("Invalid controller or transmitter!\n");
		return;
	}

	// Step 2: Setup basic parameters
	FGuid playerId = controller->GetPlayerUId();
	FGuid guid = UKismetGuidLibrary::GetDefaultObj()->NewGuid();
	FPalInstanceID instanceId;
	instanceId.InstanceId = guid;

	FPalIndividualCharacterSaveParameter initParams;
	initParams.CharacterID = UKismetStringLibrary::GetDefaultObj()->Conv_StringToName(FString(L"Foxparks")); // Pal name here
	initParams.Level = 1;

	// Step 3: Instance ID
	auto charParam = playerChar->CharacterParameterComponent;
	if (!charParam || !charParam->IndividualHandle)
	{
		OutputDebugStringA("CharacterParameterComponent or IndividualHandle is null!\n");
		return;
	}

	

	// Step 4: Spawn parameters
	FNetworkActorSpawnParameters spawnParams;
	spawnParams.Name = UKismetStringLibrary::GetDefaultObj()->Conv_StringToName(FString(L"None"));
	spawnParams.Owner = playerChar;
	spawnParams.NetworkOwner = playerChar;
	spawnParams.SpawnLocation = playerChar->K2_GetActorLocation() + FVector(100, 0, 0); // Offset spawn
	spawnParams.SpawnRotation = playerChar->K2_GetActorRotation();
	spawnParams.SpawnScale = FVector(1.f, 1.f, 1.f);
	spawnParams.ControllerClass = utility->GetNPCManager(world)->NPCAIControllerBaseClass.Get();
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

	if (!controller || !controller->Transmitter || !controller->Transmitter->NetworkIndividualComponent)
	{
		OutputDebugStringA("One or more components are NULL!\n");
		return;
	}

	// Step 5: Spawn networked Pal
	controller->Transmitter->NetworkIndividualComponent->CreateIndividualID_ServerInternal(initParams, guid, playerId.A); 
	controller->Transmitter->NetworkIndividualComponent->CreateFixedIndividualID_ServerInternal(instanceId, initParams, guid, playerId.A);
	controller->Transmitter->NetworkIndividualComponent->SpawnIndividualActor_ServerInternal(instanceId, spawnParams, guid);
}*/

