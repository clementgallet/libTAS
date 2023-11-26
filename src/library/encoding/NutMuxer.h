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

/* NutMixer taken from BizHawk <http://tasvideos.org/BizHawk.html> */

#ifndef LIBTAS_NUTMUXER_H_INCL
#define LIBTAS_NUTMUXER_H_INCL

#include <vector>
#include <cstdint>
#include <cstdio> // FILE
#include <cstring>

namespace libtas {

class NutMuxer {
public:

	static void writeVarU(uint64_t v, std::vector<uint8_t> &stream);
    static void writeVarU(unsigned int v, std::vector<uint8_t> &stream);
	static void writeVarU(int v, std::vector<uint8_t> &stream);
	static void writeVarU(int64_t v, std::vector<uint8_t> &stream);
	static void writeVarS(int64_t v, std::vector<uint8_t> &stream);
	static void writeBytes(const char* s, int len, std::vector<uint8_t> &stream);
	static void writeBE64(uint64_t v, std::vector<uint8_t> &stream);
	static void writeBE32(unsigned int v, std::vector<uint8_t> &stream);
	static void writeBE32(int v, std::vector<uint8_t> &stream);

	static unsigned int CRCtable[];

	static unsigned int nutCRC32(const std::vector<uint8_t> &buf);

	class NutPacket {
    public:

		enum StartCode
		{
			Main = 0x4e4d7a561f5f04ad,
			Stream = 0x4e5311405bf2f9db,
			Syncpoint = 0x4e4be4adeeca4569,
			Index = 0x4e58dd672f23e64e,
			Info = 0x4e49ab68b596ba78
		};

		std::vector<uint8_t> data;
		StartCode startcode;
		FILE *underlying;

		NutPacket(StartCode startcode, FILE *underlying);

		void flush();
		void write(const char* buffer, int count);
	};

	class AVParams {
    public:
		int width, height, samplerate, samplesize, fpsnum, fpsden, channels;
		const char* pixfmt;
		void reduce();
	};

	/// <summary>
	/// stores basic AV parameters
	/// </summary>
	AVParams avparams;

	/// <summary>
	/// target output for nut stream
	/// </summary>
	FILE *output;

	/// <summary>
	/// PTS of video stream.  timebase is 1/framerate, so this is equal to number of frames
	/// </summary>
	uint64_t videopts;

	/// <summary>
	/// PTS of audio stream.  timebase is 1/samplerate, so this is equal to number of samples
	/// </summary>
	uint64_t audiopts;

	/// <summary>
	/// has EOR been writen on this stream?
	/// </summary>
	bool videodone;
	/// <summary>
	/// has EOR been written on this stream?
	/// </summary>
	bool audiodone;

	/// <summary>
	/// write out the main header
	/// </summary>
	void writeMainHeader();

	/// <summary>
	/// write out the 0th stream header (video)
	/// </summary>
	void writeVideoHeader();

	/// <summary>
	/// write out the 1st stream header (audio)
	/// </summary>
	void writeAudioHeader();

    void writeFrame(const uint8_t* payload, unsigned int payloadlen, uint64_t pts, uint64_t ptsnum, uint64_t ptsden, int ptsindex, FILE *underlying);

    void writeVideoFrame(const uint8_t* video, unsigned int len);

    void writeAudioFrame(const uint8_t* samples, unsigned int len);

	NutMuxer(int width, int height, int fpsnum, int fpsden, const char* pixfmt, int samplerate, int samplesize, int channels, FILE *underlying);

	void finish();

};
}


#endif
