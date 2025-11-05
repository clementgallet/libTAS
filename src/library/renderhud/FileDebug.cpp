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

#include "FileDebug.h"

#include "../external/imgui/imgui.h"
#include "../external/imgui/implot.h"
#include "fileio/FileHandle.h"
#include "fileio/FileHandleList.h"
#include "global.h"
#include "logging.h"

#include <string>
#include <vector>
#include <cmath>

// We are setting our own limit of registered file descriptor
#define FD_LIMIT 1024
#define BAR_HEIGHT 0.8f

namespace libtas {

struct fd_history_t {
    std::string file;
    int type;
    uint64_t first_frame;
    uint64_t last_frame;
};

static std::vector<fd_history_t> fd_history[FD_LIMIT];


void FileDebug::update(uint64_t framecount)
{
    FileHandleList::updateAllFiles();
    const auto& filehandles = FileHandleList::getFileList();

    for (const FileHandle &fh : filehandles) {
        for (int i = 0; i < ((fh.type == FileHandle::FILE_PIPE)?2:1); i++) {
            int fd = fh.fds[i];

            if (fd >= FD_LIMIT) {
                LOG(LL_WARN, LCF_FILEIO, "An opened file descriptor (%d) is high than our limit (%d) %s", fd, FD_LIMIT, fh.fileName);
                continue;
            }
            
            if (fd_history[fd].empty()) {
                fd_history_t new_fdh;
                new_fdh.file = fh.fileName;
                new_fdh.type = fh.type;
                new_fdh.first_frame = framecount;
                new_fdh.last_frame = framecount;
                fd_history[fd].push_back(new_fdh);
            }
            else {
                fd_history_t &fdh = fd_history[fd].back();
                std::string fh_name = fh.fileName;
                
                /* Check if it is the same file. We do not assume that this
                 * update is called every frame. */
                if (fh_name == fdh.file) {
                    fdh.last_frame = framecount;
                }
                else {
                    fd_history_t new_fdh;
                    new_fdh.file = fh.fileName;
                    new_fdh.type = fh.type;
                    new_fdh.first_frame = framecount;
                    new_fdh.last_frame = framecount;
                    fd_history[fd].push_back(new_fdh);
                }
            }
        }
    }
}

void FileDebug::draw(uint64_t framecount, bool* p_open = nullptr)
{
    static uint64_t old_framecount = 0;
    
    if (ImGui::Begin("File Debug", p_open))
    {
        if (ImPlot::BeginPlot("File Descriptors", ImVec2(-1,-1), ImPlotFlags_NoTitle)) {
            /* Auto-resize when running, but allow zooming when paused */
            int flags = 0;
            int flagsY = (framecount != old_framecount) ? (ImPlotAxisFlags_AutoFit) : 0;
            ImPlot::SetupAxes("Frame","File descriptor", flags, flags | flagsY);
            // ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.25f);
            // ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Log10);

            if (framecount != old_framecount)
                ImPlot::SetupAxisLimits(ImAxis_X1, std::max(0.0f, static_cast<float>(framecount) - 300.0f), framecount, ImPlotCond_Always);

            /* Build the whole files as PlotShaded data */
            float xs[2], ys1[2], ys2[2];
            for (int fd = 0; fd < FD_LIMIT; fd++) {
                for (fd_history_t fdh : fd_history[fd]) {
                    xs[0] = fdh.first_frame;
                    xs[1] = fdh.last_frame;
                    ys1[0] = fd - BAR_HEIGHT / 2;
                    ys1[1] = fd - BAR_HEIGHT / 2;
                    ys2[0] = fd + BAR_HEIGHT / 2;
                    ys2[1] = fd + BAR_HEIGHT / 2;

                    ImPlot::PlotShaded(FileHandle::typeStr(fdh.type), xs, ys1, ys2, 2);
                }
            }

            if (ImPlot::IsPlotHovered()) {
                ImPlotPoint mouse = ImPlot::GetPlotMousePos();
                int hovered_fd = (int) std::roundf(mouse.y);
                if (hovered_fd >= 0 &&
                    hovered_fd < FD_LIMIT &&
                    (mouse.y - hovered_fd) < (BAR_HEIGHT / 2) && 
                    (mouse.y - hovered_fd) > (-BAR_HEIGHT / 2)) {

                    int frame = (int) std::roundf(mouse.x);

                    for (fd_history_t fdh : fd_history[hovered_fd]) {
                        if ((frame >= fdh.first_frame) && (frame <= fdh.last_frame)) {
                            ImGui::SetTooltip(fdh.file.c_str());
                        }
                    }
                }
            }

            // ImPlot::PopStyleVar();
            ImPlot::EndPlot();
        }
    }
    ImGui::End();
    
    old_framecount = framecount;
}

}
