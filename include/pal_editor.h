#include <pch.h>

extern std::vector<SDK::APalCharacter*> cachedTamedPals;

// Function declarations
std::string GetCleanPalName2(const std::string& rawName);
bool GetAllTamedPals(std::vector<SDK::APalCharacter*>& outResult);