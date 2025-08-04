#include <pch.h>

extern std::vector<SDK::APalCharacter*> cachedTamedPals;
extern std::vector<SDK::APalCharacter*> cachedBaseWorkers;

// Function declarations
std::string GetCleanPalName2(const std::string& rawName);
bool GetAllTamedPals(std::vector<SDK::APalCharacter*>& outResult);
bool GetAllBaseWorkers(std::vector<SDK::APalCharacter*>& outResult);

void DrawPalInfo(int selectedPalIndex);
void DrawPalStatsEditor(int selectedPalIndex);
void DrawPalRanksEditor(int selectedPalIndex);
void DrawPalWorkSuitabilitiesEditor(int selectedPalIndex);
void DrawPalPassiveSkillsEditor(int selectedPalIndex);
void DumpAllPassiveSkills();