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

#include "Signature.h"
#include <sstream>
#include <cstring>
#include <immintrin.h>

bool Signature::hasMask() const
{
	return memchr(mask.data(), 0, bytes.size()) != nullptr;
}

void Signature::fromIdaString(const std::string sigstr)
{
    bytes.clear();
    mask.clear();
    
    std::istringstream iss(sigstr);
    int next = iss.peek();
    while (next != EOF) {
        if (next == '?') {
            iss.get();
            iss.get();
            iss.get();
            bytes.push_back(0xCC);
            mask.push_back(0);
        }
        else {
            int b;
            iss >> std::hex >> b >> std::ws;
            bytes.push_back((uint8_t) b);
            mask.push_back(0xFF);
        }
        next = iss.peek();
    }
}

// Output the sig as a "F8 66 4B ?? ?? ?? 88" format string
std::string Signature::toIdaString() const
{
    std::string string;
    size_t count = bytes.size();
    if (count > 0) {
        string.reserve(count * 3);
        char b[4];
        for (size_t i = 0; i < count; i++)
        {
            if (mask[i]) {
                std::snprintf(b, 4, "%02X ", bytes[i]);
                string += b;
            }
            else
                string += "?? ";
        }

        // Remove the final ' ' space
        string.pop_back();
    }
    return string;
}

static inline uint32_t get_first_bit_set(uint32_t x)
{
    return __builtin_ctz(x);
}

static inline uint32_t clear_leftmost_set(uint32_t value)
{
    return value & (value - 1);
}

static int memcmp_mask(const uint8_t *buffer1, const uint8_t *buffer2, const uint8_t *mask2, size_t count)
{
    while (count--) {
        if (*mask2) {
            if (*buffer1 != *buffer2)
                return -1;
        }

        buffer1++, buffer2++, mask2++;
    }
    return 0;
}

// Find signature pattern in memory
__attribute__((target("avx2"))) uint8_t* SigSearch::FindAVX2(uint8_t* data, size_t size, const Signature &sig, bool hasWildcards)
{
    const uint8_t *pat = sig.bytes.data();
    size_t patLen = sig.bytes.size();
    size_t patLen1 = (patLen - 1);
    size_t patLen2 = (patLen - 2);

    // Fill 'first' and 'last' with the first and last pattern byte respectively
    const __m256i first = _mm256_set1_epi8(pat[0]);
    const __m256i last = _mm256_set1_epi8(pat[patLen1]);

    // We must be able to load a full m256i value, so we skip the last bytes
    size_t i;

    if (!hasWildcards) {
        // A little faster without wildcards

        // Scan 32 bytes at the time..
        for (i = 0; i < (size-32); i += 32) {
            // Load in the next 32 bytes of input first and last
            // Can use align 32 bit read for first since the input is page aligned
            const __m256i block_first = _mm256_load_si256((const __m256i*) (data + i));
            const __m256i block_last = _mm256_loadu_si256((const __m256i*) (data + i + patLen1));

            // Compare first and last data to get 32byte masks
            const __m256i eq_first = _mm256_cmpeq_epi8(first, block_first);
            const __m256i eq_last = _mm256_cmpeq_epi8(last, block_last);

            // AND the equality masks and into a 32 bit mask
            uint32_t mask = _mm256_movemask_epi8(_mm256_and_si256(eq_first, eq_last));

            // Do pattern compare between first and last position if we got our first and last at this data position
            while (mask != 0) {
                uint32_t bitpos = get_first_bit_set(mask);
                if (memcmp(data + i + bitpos + 1, pat + 1, patLen2) == 0) {
                    return data + i + bitpos;
                }
                mask = clear_leftmost_set(mask);
            }
        }
    }
    else {
        // Pattern scan with wildcards mask
        const uint8_t *msk = sig.mask.data();

        // We must be able to load a full m256i value, so skip the last bytes
        for (i = 0; i < (size-32); i += 32) {
            const __m256i block_first = _mm256_load_si256((const __m256i*) (data + i));
            const __m256i block_last = _mm256_loadu_si256((const __m256i*) (data + i + patLen1));

            const __m256i eq_first = _mm256_cmpeq_epi8(first, block_first);
            const __m256i eq_last = _mm256_cmpeq_epi8(last, block_last);

            uint32_t mask = _mm256_movemask_epi8(_mm256_and_si256(eq_first, eq_last));

            // Do a byte pattern w/mask compare between first and last position if we got our first and last
            while (mask != 0) {
                uint32_t bitpos = get_first_bit_set(mask);
                if (memcmp_mask(data + i + bitpos + 1, pat + 1, msk + 1, patLen2) == 0) {
                    return data + i + bitpos;
                }
                mask = clear_leftmost_set(mask);
            }
        }
    }

    // Search the last bytes without AVX2
    return SigSearch::FindCommon(data + i, size - i, sig, hasWildcards);
}


// ------------------------------------------------------------------------------------------------

// Find signature pattern in memory
// Base memory search reference, about 10x slower than the AVX2 version
uint8_t* SigSearch::FindCommon(uint8_t* input, size_t inputLen, const Signature &sig, bool hasWildcards)
{
    if (!hasWildcards) {
        // If no wildcards, faster to use a memcmp() type
        const uint8_t *pat = sig.bytes.data();
        const uint8_t *end = (input + inputLen);
        const uint8_t first = *pat;
        size_t sigLen = sig.bytes.size();

        // Setup last in the pattern length byte quick for rejection test
        size_t lastIdx = (sigLen - 1);
        uint8_t last = pat[lastIdx];

        for (uint8_t* ptr = input; ptr < end; ++ptr) {
            if ((ptr[0] == first) && (ptr[lastIdx] == last)) {
                if (memcmp(ptr+1, pat+1, sigLen-2) == 0)
                    return ptr;
            }
        }
    }
    else {
        const uint8_t *pat = sig.bytes.data();
        const uint8_t *msk = sig.mask.data();
        const uint8_t *end = (input + inputLen);
        const uint8_t first = *pat;
        size_t sigLen = sig.bytes.size();
        size_t lastIdx = (sigLen - 1);
        uint8_t last = pat[lastIdx];

        for (uint8_t* ptr = input; ptr < end; ++ptr) {
            if ((ptr[0] == first) && (ptr[lastIdx] == last)) {
                const uint8_t *patPtr = pat+1;
                const uint8_t *mskPtr = msk+1;
                const uint8_t *memPtr = ptr+1;
                bool found = true;

                for (size_t i = 0; (i < sigLen-2) && (memPtr < end); ++mskPtr, ++patPtr, ++memPtr, i++) {
                    if (!*mskPtr)
                        continue;

                    if (*memPtr != *patPtr) {
                        found = false;
                        break;
                    }
                }

                if (found)
                    return ptr;
            }
        }
    }

    return nullptr;
}

// ------------------------------------------------------------------------------------------------

// Reference version search
int SigSearch::SearchCommon(uint8_t* input, size_t inputLen, const Signature &sig, ptrdiff_t* output_offset)
{
    size_t sigSize = sig.bytes.size();
    size_t len = inputLen - sigSize;
    size_t count = 0;
    bool hasWildcards = sig.hasMask();

    inputLen -= sigSize;

    // Search for signature match..
    uint8_t* match = FindCommon(input, len, sig, hasWildcards);
    while (match)
    {
        *output_offset = (ptrdiff_t) (match - input);
        ++count;
        // Stop now if we've hit two matches
        // if (++count >= 2)
        //     break;

        ++match;
        len = (inputLen - (int) (match - input));
        if (len < sigSize)
            break;

        // Next search
        match = FindCommon(match, len, sig, hasWildcards);
    };

    return count;
}

// Fast AVX2 based search
int SigSearch::SearchAVX2(uint8_t* input, size_t inputLen, const Signature &sig, ptrdiff_t* output_offset)
{
    size_t sigSize = sig.bytes.size();
    size_t len = inputLen - sigSize;
    size_t count = 0;
    bool hasWildcards = sig.hasMask();

    inputLen -= sigSize;

    uint8_t* match = FindAVX2(input, len, sig, hasWildcards);
    while (match)
    {
        *output_offset = (ptrdiff_t) (match - input);
        ++count;
        // if (++count >= 2)
        //     break;

        ++match;
        len = (inputLen - (int) (match - input));
        if (len < sigSize)
            break;

        match = FindAVX2(match, len, sig, hasWildcards);
    };

    return count;
}

// Search for signature pattern, returning a status result
int SigSearch::Search(uint8_t* input, size_t inputLen, const Signature &sig, ptrdiff_t* output_offset)
{
    static bool isAVX2Supported = __builtin_cpu_supports("avx2");
    
    if (isAVX2Supported)
        return SearchAVX2(input, inputLen, sig, output_offset);
    else
        return SearchCommon(input, inputLen, sig, output_offset);
}
