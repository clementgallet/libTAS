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

#include "HexDelegate.h"
#include "../external/qhexview/include/QHexView/qhexview.h"

HexDelegate::HexDelegate(QObject *parent) : QHexDelegate(parent) {}

bool HexDelegate::render(quint64 offset, quint8 b, QTextCharFormat& outcf, const QHexView* hexview) {
    const auto now = std::chrono::steady_clock::now();
    auto last_value = last_values_map.find(offset);
    if (last_value != last_values_map.end()) {
        if (last_value->second.last_value == b) {

            const std::chrono::duration<double, std::milli> delta_ms  = now - last_value->second.last_modification_time;
            if (delta_ms > std::chrono::milliseconds(1000)) {
                return false;
            }
            
            double ratio = 0.8f * (delta_ms / std::chrono::milliseconds(1000));
            outcf.setBackground(QColor::fromRgbF(1.0f, 0.0f, 0.0f, 0.8f - ratio));
            return true;
        }
        else {
            last_value->second.last_value = b;
            last_value->second.last_modification_time = now;
            outcf.setBackground(QColor::fromRgbF(1.0f, 0.0f, 0.0f, 1.0f));
            return true;
        }
    }

    ValueHistory v;
    v.last_value = b;
    v.last_modification_time = now - std::chrono::milliseconds(2000);
    last_values_map.emplace(offset, v);
    
    /* Keep the map under a certain size, by removing addresses that are too far */
    if (last_values_map.size() > 1000) {
        auto beg_it = last_values_map.begin();
        auto end_it = last_values_map.end();
        end_it--;
        if ((offset - beg_it->first) > (end_it->first - offset))
            last_values_map.erase(beg_it);
        else
            last_values_map.erase(end_it);
    }
    return false;
}
