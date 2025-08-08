// imgui_style.h
#pragma once

void SetupImGuiStyle();
bool CustomButton(const char* label, ImVec2 size = ImVec2(0, 36), bool active = false);
void ColoredSeparatorText(const char* text, ImVec4 textColor, float thickness = 1.5f, float padding = 6.0f);
