/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LINTAS_TYPEINDEX_H_INCLUDED
#define LINTAS_TYPEINDEX_H_INCLUDED

/* This looks aaawwwful */
template <typename T> static inline int type_index() {}
template <> inline int type_index<unsigned char>() {return 0;}
template <> inline int type_index<char>() {return 1;}
template <> inline int type_index<unsigned short>() {return 2;}
template <> inline int type_index<short>() {return 3;}
template <> inline int type_index<unsigned int>() {return 4;}
template <> inline int type_index<int>() {return 5;}
template <> inline int type_index<uint64_t>() {return 6;}
template <> inline int type_index<int64_t>() {return 7;}
template <> inline int type_index<float>() {return 8;}
template <> inline int type_index<double>() {return 9;}

#endif
