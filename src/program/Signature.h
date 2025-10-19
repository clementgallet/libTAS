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

#ifndef LIBTAS_SIGNATURE_H_INCLUDED
#define LIBTAS_SIGNATURE_H_INCLUDED

#include <string>
#include <vector>
#include <stdint.h>
#include <stddef.h>

/* Code taken mostly from <https://github.com/kweatherman/sigmakerex> */

struct Signature {
    bool hasMask() const;
    void fromIdaString(const std::string sigstr);
    std::string toIdaString() const;

    std::vector<uint8_t> bytes;
    std::vector<uint8_t> mask;	// 0xFF = keep, 0 = wildcard/skip
};

namespace SigSearch {
    uint8_t* FindAVX2(uint8_t* data, size_t size, const Signature &sig, bool hasWildcards);
    uint8_t* FindCommon(uint8_t* input, size_t inputLen, const Signature &sig, bool hasWildcards);
    int SearchCommon(uint8_t* input, size_t inputLen, const Signature &sig, ptrdiff_t* output_offset);
    int SearchAVX2(uint8_t* input, size_t inputLen, const Signature &sig, ptrdiff_t* output_offset);
    int Search(uint8_t* input, size_t inputLen, const Signature &sig, ptrdiff_t* output_offset);
};

#endif
