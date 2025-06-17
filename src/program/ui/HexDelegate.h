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

#ifndef LIBTAS_HEXDELEGATE_H_INCLUDED
#define LIBTAS_HEXDELEGATE_H_INCLUDED

#include "../external/qhexview/include/QHexView/model/qhexdelegate.h"
#include "../external/qhexview/include/QHexView/model/qhexdocument.h"
#include "../external/qhexview/include/QHexView/model/qhexutils.h"

#include <map>

class QHexView;

struct ValueHistory {
    quint8 last_value;
    std::chrono::time_point<std::chrono::steady_clock> last_modification_time;
};

class HexDelegate: public QHexDelegate {
    Q_OBJECT

public:
    HexDelegate(QObject* parent = nullptr);
    bool render(quint64 offset, quint8 b, QTextCharFormat& outcf, const QHexView* hexview) override;
                        
private:
    /* Map of addresses to last values */
    std::map<quint64, ValueHistory> last_values_map;

};

#endif
