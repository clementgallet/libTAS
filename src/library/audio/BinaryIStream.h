/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_BINARYISTREAM_H_INCL
#define LIBTAS_BINARYISTREAM_H_INCL

/* Ultra crappy class for reading integers of different sizes.
 * Feel free to improve it
 */

#include <stdint.h>
#include <vector>

namespace libtas {
class BinaryIStream
{
    private:
        uint8_t* values;
        int size;
        int pos;
        bool end;

    public:
        BinaryIStream(uint8_t* data, int datasize)
        {
            values = data;
            size = datasize;
            pos = 0;
            end = false;
        }

        BinaryIStream(std::vector<uint8_t> &data)
        {
            values = data.data();
            size = data.size();
            pos = 0;
            end = false;
        }

        explicit operator bool() const {return !end;}

        friend BinaryIStream &operator>>( BinaryIStream &stream, uint8_t &v )
        {
            if (stream.end)
                return stream;
            if (stream.pos >= stream.size) {
                stream.end = true;
                return stream;
            }

            v = stream.values[stream.pos];
            stream.pos += 1;
            return stream;
        }

        friend BinaryIStream &operator>>( BinaryIStream &stream, int16_t &v )
        {
            if (stream.end)
                return stream;
            if ((stream.pos+1) >= stream.size) {
                stream.end = true;
                return stream;
            }
            v = *reinterpret_cast<int16_t*>(stream.values + stream.pos);
            stream.pos += 2;
            return stream;
        }
};
}

#endif
