/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "LogWindow.h"

#include "../external/imgui/imgui.h"

#include <mutex>

namespace libtas {

static ImGuiTextBuffer buffer;
static ImGuiTextFilter filter;
static ImVector<int> lineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
static bool autoScroll;  // Keep scrolling if already at the bottom.
static std::mutex mutex;

void LogWindow::clear()
{
    buffer.clear();
    lineOffsets.clear();
    lineOffsets.push_back(0);
}

void LogWindow::addLog(const char* beg, const char* end, bool newline)
{
    /* Logs can be added from multiple threads */
    std::lock_guard<std::mutex> lock(mutex);
    if (buffer.size() > 1000000) {
        buffer.clear();
        lineOffsets.clear();
    }
    buffer.append(beg, end);
    /* We always push a string with at most one `\n` character at the end */
    if (newline)
        lineOffsets.push_back(buffer.size());
}

void LogWindow::draw(bool* p_open = nullptr)
{
    if (!ImGui::Begin("Log Window", p_open))
    {
        ImGui::End();
        return;
    }

    // Options menu
    if (ImGui::BeginPopup("Options"))
    {
        ImGui::Checkbox("Auto-scroll", &autoScroll);
        ImGui::EndPopup();
    }

    // Main window
    if (ImGui::Button("Options"))
        ImGui::OpenPopup("Options");
    ImGui::SameLine();
    bool clearButton = ImGui::Button("Clear");
    ImGui::SameLine();
    bool copyButton = ImGui::Button("Copy");
    ImGui::SameLine();
    filter.Draw("Filter", -100.0f);

    ImGui::Separator();

    if (ImGui::BeginChild("scrolling", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar))
    {
        std::lock_guard<std::mutex> lock(mutex);

        if (clearButton)
            clear();
        if (copyButton)
            ImGui::LogToClipboard();

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        const char* buf = buffer.begin();
        const char* buf_end = buffer.end();
        if (filter.IsActive())
        {
            // In this example we don't use the clipper when filter is enabled.
            // This is because we don't have random access to the result of our filter.
            // A real application processing logs with ten of thousands of entries may want to store the result of
            // search/filter.. especially if the filtering function is not trivial (e.g. reg-exp).
            for (int line_no = 0; line_no < lineOffsets.Size; line_no++)
            {
                const char* line_start = buf + lineOffsets[line_no];
                const char* line_end = (line_no + 1 < lineOffsets.Size) ? (buf + lineOffsets[line_no + 1] - 1) : buf_end;
                if (filter.PassFilter(line_start, line_end))
                    ImGui::TextUnformatted(line_start, line_end);
            }
        }
        else
        {
            // The simplest and easy way to display the entire buffer:
            //   ImGui::TextUnformatted(buf_begin, buf_end);
            // And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward
            // to skip non-visible lines. Here we instead demonstrate using the clipper to only process lines that are
            // within the visible area.
            // If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them
            // on your side is recommended. Using ImGuiListClipper requires
            // - A) random access into your data
            // - B) items all being the  same height,
            // both of which we can handle since we have an array pointing to the beginning of each line of text.
            // When using the filter (in the block of code above) we don't have random access into the data to display
            // anymore, which is why we don't use the clipper. Storing or skimming through the search result would make
            // it possible (and would be recommended if you want to search through tens of thousands of entries).
            ImGuiListClipper clipper;
            clipper.Begin(lineOffsets.Size);
            while (clipper.Step())
            {
                for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
                {
                    const char* line_start = buf + lineOffsets[line_no];
                    const char* line_end = (line_no + 1 < lineOffsets.Size) ? (buf + lineOffsets[line_no + 1] - 1) : buf_end;
                    ImGui::TextUnformatted(line_start, line_end);
                }
            }
            clipper.End();
        }
        ImGui::PopStyleVar();

        // Keep up at the bottom of the scroll region if we were already at the bottom at the beginning of the frame.
        // Using a scrollbar or mouse-wheel will take away from the bottom edge.
        if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);
    }
    ImGui::EndChild();
    ImGui::End();
}

}
