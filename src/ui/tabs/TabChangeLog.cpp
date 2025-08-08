#include <pch.h>
#include "Tabs.h"

void TabChangeLog()
{
    // Begin a scrollable area for long logs
    if (ImGui::BeginChild("ChangeLogScroll", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar))
    {
        ImGui::Text("Change Log");
        ImGui::Separator();
        ImGui::Spacing();

        // Predeclare notes arrays for each version
        static const char* notes_v140[] = {
            "Added Repair Current Weapon feature (bindable).",
            "Added Weapon Damage Against Resources (trees, stones, etc.).",
            "Separated Online and Single Player tabs.",
            "Added feature to Collect Relics/Lifemunks.",
            "Fixed crash caused by Palesp.",
            "Added Levels to the Boss Teleporter List.",
            nullptr
        };
        static const char* notes_v130[] = {
            "Game Update v0.6.4",
            "Added Save/Load functionality.",
            "Increased ESP distance to 40000.",
            nullptr
        };
        static const char* notes_v120[] = {
            "Infinite Ammo now works in online mode.",
            "Fixed Infinite Ammo crashing sometimes.",
            "Weight updates instantly and sprint restored (jump will be fixed next).",
            "Added Camera Brightness setting.",
            "Fixed crashes caused by Relic ESP.",
            "Collected Relics no longer show in ESP.",
            "Added extra checks to prevent crashes.",
            nullptr
        };
        static const char* notes_v110[] = {
            "Improved ESP visuals for better clarity.",
            "Added Health display for Pals.",
            "Added Relic / Lifemunk ESP.",
            "Added Aimbot features with Distance & FOV options.",
            "Added Infinite HP.",
            "Configurable hotkeys for toggles (stamina, ESP types, etc.).",
            "World Speed / One-Hit Attack toggle implemented.",
            "Fixed crash issues (stable ESP and caching).",
            nullptr
        };
        static const char* notes_v100[] = {
            "Added basic functionality for aimbot and ESP.",
            "Implemented key bindings for aimbot activation.",
            "Introduced basic UI layout with tabs.",
            nullptr
        };

        struct ChangeLogEntry { const char* version; const char* date; const char* const* notes; };
        static const ChangeLogEntry logs[] = {
            { "v1.4.0", "", notes_v140 },
            { "v1.3.0", "", notes_v130 },
            { "v1.2.0", "", notes_v120 },
            { "v1.1.0", "", notes_v110 },
            { "v1.0.0", "", notes_v100 }
        };

        // Render each log entry
        for (const auto& entry : logs)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 215, 0, 255)); // Gold for versions
            ImGui::Text("%s %s", entry.version, entry.date);
            ImGui::PopStyleColor();
            ImGui::Indent(20.0f);
            for (int i = 0; entry.notes[i] != nullptr; ++i)
            {
                ImGui::BulletText("%s", entry.notes[i]);
            }
            ImGui::Unindent(20.0f);
            ImGui::Spacing();
			ImGui::Separator();
        }

        ImGui::EndChild();
    }
    else
    {
        ImGui::Text("Unable to create ChangeLog view.");
    }
}
