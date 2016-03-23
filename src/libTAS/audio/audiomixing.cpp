/*
    Copyright 2011 nitsuja and contributors
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of the Hourglass project and was modified to 
    be included in libTAS.

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

#include "audiomixing.h"

/* This functions are taken directly from the hourglass source code
 * See https://github.com/Hourglass-Resurrection/Hourglass-Resurrection/blob/master/wintasee/soundmixing.cpp
 */

/* This is used mainly for clamping from -32678 to 32767 */
#define clamptofullsignedrange(x,lo,hi) (((unsigned int)((x)-(lo))<=(unsigned int)((hi)-(lo)))?(x):(((x)<0)?(lo):(hi)))

/* This is the mixing uber-function that does all the hard work.
 * There is a heavy focus on performance here,
 * so this tries to do everything and do it "well enough", with nothing really fancy.
 * 
 * FREQUENCY ADJUSTMENT:
 *   This handles conversion from any frequency to any other frequency.
 *   The adjustment amount is decided implicitly based on the ratio between size and outSize.
 * BIT RATE CONVERSION:
 *   This handles all possible combinations of bitrate conversions between the following types:
 *   (8-bit signed, 8-bit unsigned, 16-bit signed, and 16-bit unsigned)
 *   as determined by the types you specify for fromtype and totype.
 *   in practice, only the following 4 combinations are useful:
 *   (8-bit unsigned to 8-bit unsigned, 16-bit signed to 16-bit signed,
 *    8-bit unsigned to 16-bit signed, and 16-bit signed to 8-bit unsigned)
 *   however, it is recommended the mixing destination buffer always be 16-bit, for quality reasons.
 * CHANNEL CONVERSION:
 *   This handles all 4 possible combinations of channel conversions
 *   (mono to mono, stereo to stereo, mono to stereo, and stereo to mono)
 *   as determined by the values you specify for fromchannels and tochannels.
 *   However, it is recommended the mixing destination buffer always be stereo, for quality reasons.
 * PANNING AND VOLUME ADJUSTMENT:
 *   The volume levels given ( range [0,65536) ) are applied to the source audio when mixing.
 *   This automatically includes panning, which applies for every case except (mono to mono) conversion.
 * CLIPPING:
 *   Values exceeding the bounds of the destination bitrate will be clamped to within the valid range.
 * INTERPOLATION:
 *   This performs linear interpolation of samples.
 *   Some games would sound very noticeably wrong otherwise.
 */

template<typename fromtype, typename totype, int fromchannels, int tochannels>
static void Mix(const uint8_t* buf, uint8_t* outbuf, int size, int outSize, bool sizeReachesBufferEnd, int lvas, int rvas)
{
    enum { fromshift = (2+sizeof(fromtype)-sizeof(totype))<<3 }; // 16 when both buffers have the same bit/sample... this is for combining differing bitrates
    enum { maxto = (1<<(8*sizeof(totype)-1))-1 }; // clamping magnitude (127 or 32767)
    enum { tosignoffset = (totype(-1)<0)?0:-(1<<(8*sizeof(totype)-1)) }; // add this to make numbers in the "to" buffer signed
    enum { fromsignoffset = (fromtype(-1)<0)?0:-(1<<(8*sizeof(fromtype)-1)) }; // add this to make numbers in the "from" buffer signed
    enum { toincrement = sizeof(totype) * tochannels };
    enum { fromincrement = sizeof(fromtype) * fromchannels };

    int frac = 0; // amount of next sample to use, out of 1024 (for linear interpolation)
    int fracnumer = (size*(toincrement<<10));
    int fracincrement = fracnumer / outSize;
    /* Unfortunately needs to be very exact (can't drift even by 1 or clicking becomes audible), so we also need: */
    int fracErrorIncrement = fracnumer % outSize;
    int fracError = 0;

    int offsetRemainder = 0;
    int inOffset = 0;

    for(int i = 0; i < outSize; outbuf += toincrement, i += toincrement) {

        int offset = inOffset;
        offset -= offset % fromincrement; // prevent starting from the wrong speaker (compiler should be smart enough not to do a modulo op here)

        const uint8_t* inbuf = buf + offset;
        offset += fromincrement;
        if(sizeReachesBufferEnd && (offset > (size - fromincrement)))
            /* note: don't subtract bufferSize instead, that doesn't work right (slight clicking) even in the looping case */
            offset = size - fromincrement;

        /* Next sample position (for interpolation) */
        const uint8_t* inbuf2 = buf + offset;

        int myL = (int)((fromtype*)inbuf)[0] + fromsignoffset;
        int myR = (int)((fromtype*)inbuf)[fromchannels-1] + fromsignoffset;
        int myL2 = (int)((fromtype*)inbuf2)[0] + fromsignoffset;
        int myR2 = (int)((fromtype*)inbuf2)[fromchannels-1] + fromsignoffset;
        int otherL = (int)((totype*)outbuf)[0] + tosignoffset;
        int otherR = (int)((totype*)outbuf)[tochannels-1] + tosignoffset;
        int mixedL = (otherL + ((int)(((myL * (int)lvas) >> fromshift) * (1024-frac) + ((myL2 * (int)lvas) >> fromshift) * frac) >> 10));
        int mixedR = (otherR + ((int)(((myR * (int)rvas) >> fromshift) * (1024-frac) + ((myR2 * (int)rvas) >> fromshift) * frac) >> 10));
        if(tochannels != 1) {
            /* Stereo output */
            ((totype*)outbuf)[0] = (clamptofullsignedrange(mixedL,-maxto-1,maxto))-tosignoffset;
            ((totype*)outbuf)[1] = (clamptofullsignedrange(mixedR,-maxto-1,maxto))-tosignoffset;
        }
        else {
            /* Monaural output */
            int mixed = (mixedL + mixedR) >> 1;
            ((totype*)outbuf)[0] = clamptofullsignedrange(mixed,-maxto-1,maxto)-tosignoffset;
        }

        /* I'm going to lots of trouble to avoid using integer division or modulus
         * in this inner loop, since they used to be slowing it down drastically.
         * in my tests, this function was using 60% of the frame time (in cave story),
         * and now it runs fully 4 times faster than it did before.
         */

        offsetRemainder += size*toincrement;
        while(offsetRemainder >= outSize) // definitely needs to be while, not if
        {
            offsetRemainder -= outSize;
            inOffset++;
        }

        fracError += fracErrorIncrement;
        if(fracError >= outSize)
        {
            fracError -= outSize;
            frac++;
        }
        frac = (frac + fracincrement) & 0x3FF;
    }
}

template<int fromchannels, int tochannels>
static void Mix(const uint8_t* buf, uint8_t* outbuf, int myBitsPerSample, int outBitsPerSample, int size, int outSize, bool sizeReachesBufferEnd, int lvas, int rvas)
{
	/* Note: WAV uses unsigned for 8-bit and signed for 16-bit (it makes a big difference!) */
	if(myBitsPerSample <= 8 && outBitsPerSample <= 8)
		Mix<uint8_t,uint8_t,fromchannels,tochannels>(buf, outbuf, size, outSize, sizeReachesBufferEnd, lvas, rvas);
	else if(myBitsPerSample > 8 && outBitsPerSample > 8)
		Mix<int16_t,int16_t,fromchannels,tochannels>(buf, outbuf, size, outSize, sizeReachesBufferEnd, lvas, rvas);
	else if(outBitsPerSample > 8)
		Mix<uint8_t,int16_t,fromchannels,tochannels>(buf, outbuf, size, outSize, sizeReachesBufferEnd, lvas, rvas);
	else
		Mix<int16_t,uint8_t,fromchannels,tochannels>(buf, outbuf, size, outSize, sizeReachesBufferEnd, lvas, rvas);
}

void MixFromToInternal(const uint8_t* buf, int size,
        uint8_t* outbuf, int outSize,
        int bitDepth, int nbChannels,
        int outBitDepth, int outNbChannels,
        bool lastSample, int lvas, int rvas)
{

	if(nbChannels == 1 && outNbChannels == 1)
		Mix<1,1>(buf, outbuf, bitDepth, outBitDepth, size, outSize, lastSample, lvas, rvas);
	else if(nbChannels == 1 && outNbChannels == 2)
		Mix<1,2>(buf, outbuf, bitDepth, outBitDepth, size, outSize, lastSample, lvas, rvas);
	else if(nbChannels == 2 && outNbChannels == 1)
		Mix<2,1>(buf, outbuf, bitDepth, outBitDepth, size, outSize, lastSample, lvas, rvas);
	else if(nbChannels == 2 && outNbChannels == 2)
		Mix<2,2>(buf, outbuf, bitDepth, outBitDepth, size, outSize, lastSample, lvas, rvas);
}


