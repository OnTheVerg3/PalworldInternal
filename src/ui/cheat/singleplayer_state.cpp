#include <pch.h>
#include "cheat_state.h"

using namespace SDK;
using namespace Helper;


void ExploitFly()
{
	SDK::APalPlayerCharacter* pPalPlayerCharacter = GetPalPlayerCharacter();
	if (!pPalPlayerCharacter)
		return;

	APalPlayerController* pPalPlayerController = pPalPlayerCharacter->GetPalPlayerController();
	if (!pPalPlayerController)
		return;

	cheatState.isFly ? pPalPlayerController->StartFlyToServer() : pPalPlayerController->EndFlyToServer();
}

void SetPlayerDefenseParam()
{
	APalPlayerCharacter* pPalPlayerCharacter = GetPalPlayerCharacter();
	if (!pPalPlayerCharacter)
		return;

	UPalCharacterParameterComponent* pParams = pPalPlayerCharacter->CharacterParameterComponent;
	if (!pParams)
		return;

	if (pParams->DefenseUp != cheatState.defence)
		pParams->DefenseUp = cheatState.defence;
}

void SetCraftingSpeed()
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
		mCraftSpeedArray[0].Value = cheatState.craftingSpeed;
}

void AddTechPoints()
{
	APalPlayerState* mPlayerState = GetPalPlayerState();
	if (!mPlayerState)
		return;

	UPalTechnologyData* pTechData = mPlayerState->TechnologyData;
	if (!pTechData)
		return;

	pTechData->TechnologyPoint += cheatState.addTechPoints;
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

	pTechData->bossTechnologyPoint += cheatState.addAnchientTechPoints;
}

void RemoveTechPoints()
{
	APalPlayerState* mPlayerState = GetPalPlayerState();
	if (!mPlayerState)
		return;

	UPalTechnologyData* pTechData = mPlayerState->TechnologyData;
	if (!pTechData)
		return;

	pTechData->TechnologyPoint -= cheatState.removeTechPoints;
}

//	
void RemoveAncientTechPoint()
{
	APalPlayerState* mPlayerState = GetPalPlayerState();
	if (!mPlayerState)
		return;

	UPalTechnologyData* pTechData = mPlayerState->TechnologyData;
	if (!pTechData)
		return;

	pTechData->bossTechnologyPoint -= cheatState.removeAnchientTechPoints;
}

void SetPlayerSpeed()
{
	APalPlayerCharacter* pPalPlayerCharacter = GetPalPlayerCharacter();
	if (!pPalPlayerCharacter)
		return;

	// APalPlayerCharacter inherits from APalCharacter
	UPalCharacterMovementComponent* pMovement = pPalPlayerCharacter->GetPalCharacterMovementComponent();
	if (!pMovement)
		return;

	// Apply speed multiplier
	pMovement->MaxWalkSpeed = cheatState.speedMultiplier;
	pMovement->MaxFlySpeed = cheatState.speedMultiplier;  // optional
	pMovement->MaxSwimSpeed = cheatState.speedMultiplier;  // optional
}

void SetPlayerLevel()
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

	ivParams->SetOverrideLevel(cheatState.playerLevel);
}


static SDK::UPalExpDatabase* GetExpDatabase()
{
	if (SDK::APalPlayerCharacter* me = GetPalPlayerCharacter())
	{
		if (SDK::UWorld* world = UWorld::GetWorld())
		{
			// Use whatever your SDK exposes (OwningGameInstance or GetGameInstance())
			if (SDK::UGameInstance* giBase = world->OwningGameInstance) // or world->GetGameInstance()
				return static_cast<SDK::UPalExpDatabase*>(static_cast<void*>(
					reinterpret_cast<SDK::UPalGameInstance*>(giBase)->ExpDatabase));
		}
	}
	return nullptr;
}

// Are we authority? (must be true for direct server calls to work)
static bool IsAuthority()
{
	if (SDK::APalPlayerCharacter* me = GetPalPlayerCharacter())
	{
		// UE4/5 style: actor role check
		return me->GetLocalRole() == SDK::ENetRole::ROLE_Authority;
	}
	return false;
}
