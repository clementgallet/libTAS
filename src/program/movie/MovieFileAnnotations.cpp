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

#include "MovieFileAnnotations.h"

#include "Context.h"

#include <fstream>
#include <filesystem>

MovieFileAnnotations::MovieFileAnnotations(Context* c) : context(c) {}

void MovieFileAnnotations::clear()
{
    text.clear();
}

void MovieFileAnnotations::load()
{
    /* Load annotations if available */
    std::filesystem::path annotations_file = context->config.tempmoviedir / "annotations.txt";
    std::ifstream annotations_stream(annotations_file);
    if (annotations_stream) {
        text = std::string((std::istreambuf_iterator<char>(annotations_stream)),
                     std::istreambuf_iterator<char>());
    }
    else {
        text = "";
    }
}

void MovieFileAnnotations::save()
{
    /* Save annotations */
    std::filesystem::path annotations_file = context->config.tempmoviedir / "annotations.txt";
    std::ofstream annotations_stream(annotations_file);
    annotations_stream << text;
    annotations_stream.close();
}
