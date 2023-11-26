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

#include "NutMuxer.h"

#include "logging.h"

namespace libtas {

void NutMuxer::writeVarU(uint64_t v, std::vector<uint8_t> &stream)
{
	uint8_t b[10];
	int i = 0;
	do
	{
		if (i > 0)
			b[i++] = static_cast<uint8_t>((v & 127) | 128);
		else
			b[i++] = static_cast<uint8_t>(v & 127);
		v /= 128;
	} while (v > 0);
	for (; i > 0; i--)
	   stream.push_back(b[i - 1]);
}

void NutMuxer::writeVarU(unsigned int v, std::vector<uint8_t> &stream)
{
	writeVarU(static_cast<uint64_t>(v), stream);
}

void NutMuxer::writeVarU(int v, std::vector<uint8_t> &stream)
{
	writeVarU(static_cast<uint64_t>(v), stream);
}

void NutMuxer::writeVarU(int64_t v, std::vector<uint8_t> &stream)
{
	writeVarU(static_cast<uint64_t>(v), stream);
}

void NutMuxer::writeVarS(int64_t v, std::vector<uint8_t> &stream)
{
	uint64_t temp;
	if (v < 0)
		temp = 1 + 2 * static_cast<uint64_t>(-v);
	else
		temp = 2 * static_cast<uint64_t>(v);
	writeVarU(temp - 1, stream);
}

void NutMuxer::writeBytes(const char* s, int len, std::vector<uint8_t> &stream)
{
	writeVarU(len, stream);
	stream.insert(stream.end(), s, s + len);
}

void NutMuxer::writeBE64(uint64_t v, std::vector<uint8_t> &stream)
{
	uint8_t b[8];
	for (int i = 7; i >= 0; i--)
	{
		b[i] = static_cast<uint8_t>(v & 255);
		v >>= 8;
	}
	stream.insert(stream.end(), b, b + 8);
}

void NutMuxer::writeBE32(unsigned int v, std::vector<uint8_t> &stream)
{
	uint8_t b[4];
	for (int i = 3; i >= 0; i--)
	{
		b[i] = static_cast<uint8_t>(v & 255);
		v >>= 8;
	}
	stream.insert(stream.end(), b, b + 4);
}

void NutMuxer::writeBE32(int v, std::vector<uint8_t> &stream)
{
	uint8_t b[4];
	for (int i = 3; i >= 0; i--)
	{
		b[i] = static_cast<uint8_t>(v & 255);
		v >>= 8;
	}
    stream.insert(stream.end(), b, b + 4);
}

unsigned int NutMuxer::CRCtable[] =
{
	0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9,
	0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005,
	0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61,
	0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD,
};

unsigned int NutMuxer::nutCRC32(const std::vector<uint8_t> &buf)
{
	unsigned int crc = 0;
	for (unsigned int i = 0; i < buf.size(); i++)
	{
		crc ^= static_cast<unsigned int>(buf[i]) << 24;
		crc = (crc << 4) ^ CRCtable[crc >> 28];
		crc = (crc << 4) ^ CRCtable[crc >> 28];
	}
	return crc;
}

NutMuxer::NutPacket::NutPacket(StartCode sc, FILE *u)
{
	startcode = sc;
	underlying = u;
}

void NutMuxer::NutPacket::flush()
{
	// first, prep header
	std::vector<uint8_t> header;
	writeBE64(static_cast<uint64_t>(startcode), header);
	writeVarU(static_cast<int>(data.size() + 4), header); // +4 for checksum
	if (data.size() > 4092)
	{
		writeBE32(nutCRC32(header), header);
	}

	fwrite(header.data(), 1, header.size(), underlying);

    writeBE32(nutCRC32(data), data);
    fwrite(data.data(), 1, data.size(), underlying);
}

void NutMuxer::NutPacket::write(const char* buffer, int count)
{
	data.insert(data.end(), buffer, buffer + count);
}


void NutMuxer::AVParams::reduce()
{
    int u = fpsnum;
    int v = fpsden;
    while ( v != 0) {
        unsigned r = u % v;
        u = v;
        v = r;
    }
    fpsnum /= u;
	fpsden /= u;
}

void NutMuxer::writeMainHeader()
{
	debuglogstdio(LCF_DUMP, "Write nut main header");

	// note: this file start tag not actually part of main headers
    const char tag[] = "nut/multimedia container\0";
	fwrite(tag, 1, strlen(tag)+1, output);

	NutPacket header_packet(NutPacket::Main, output);

	writeVarU(3, header_packet.data); // version
	writeVarU(2, header_packet.data); // stream_count
	writeVarU(65536, header_packet.data); // max_distance

	writeVarU(2, header_packet.data); // time_base_count
	// timebase is length of single frame, so reversed num+den is intentional
	writeVarU(avparams.fpsden, header_packet.data); // time_base_num[0]
	writeVarU(avparams.fpsnum, header_packet.data); // time_base_den[0]
	writeVarU(1, header_packet.data); // time_base_num[1]
	writeVarU(avparams.samplerate, header_packet.data); // time_base_den[1]

	// frame flag compression is ignored for simplicity
	for (int i = 0; i < 255; i++) // not 256 because entry 0x4e is skipped (as it would indicate a startcode)
	{
		writeVarU((1 << 12), header_packet.data); // tmp_flag = FLAG_CODED
		writeVarU(0, header_packet.data); // tmp_fields
	}

	// header compression ignored because it's not useful to us
	writeVarU(0, header_packet.data); // header_count_minus1

	// BROADCAST_MODE only useful for realtime transmission clock recovery
	writeVarU(0, header_packet.data); // main_flags

	header_packet.flush();
}

void NutMuxer::writeVideoHeader()
{
	debuglogstdio(LCF_DUMP, "Write nut video header");

    NutPacket header_packet(NutPacket::Stream, output);

	writeVarU(0, header_packet.data); // stream_id
	writeVarU(0, header_packet.data); // stream_class = video
	writeBytes(avparams.pixfmt, 4, header_packet.data); // fourcc
	writeVarU(0, header_packet.data); // time_base_id = 0
	writeVarU(8, header_packet.data); // msb_pts_shift
	writeVarU(1, header_packet.data); // max_pts_distance
	writeVarU(0, header_packet.data); // decode_delay
	writeVarU(1, header_packet.data); // stream_flags = FLAG_FIXED_FPS
	writeBytes("", 0, header_packet.data); // codec_specific_data

	// stream_class = video
	writeVarU(avparams.width, header_packet.data); // width
	writeVarU(avparams.height, header_packet.data); // height
	writeVarU(1, header_packet.data); // sample_width
	writeVarU(1, header_packet.data); // sample_height
	writeVarU(18, header_packet.data); // colorspace_type = full range rec709 (avisynth's "PC.709")

	header_packet.flush();
}

void NutMuxer::writeAudioHeader()
{
	debuglogstdio(LCF_DUMP, "Write nut audio header");

    NutPacket header_packet(NutPacket::Stream, output);

	writeVarU(1, header_packet.data); // stream_id
	writeVarU(1, header_packet.data); // stream_class = audio
	if ((avparams.samplesize / avparams.channels) == 2)
		writeBytes("PSD\x10", 4, header_packet.data); // fourcc = little-endian signed interleaved 16-bit
	else if ((avparams.samplesize / avparams.channels) == 1)
		writeBytes("PUD\x08", 4, header_packet.data); // fourcc = little-endian unsigned interleaved 8-bit
	else {
		debuglogstdio(LCF_DUMP | LCF_ERROR, "Unrecognized audio format");
		writeBytes("\x00\x00\x00\x00", 4, header_packet.data);
	}
	writeVarU(1, header_packet.data); // time_base_id = 1
	writeVarU(8, header_packet.data); // msb_pts_shift
	writeVarU(avparams.samplerate, header_packet.data); // max_pts_distance
	writeVarU(0, header_packet.data); // decode_delay
	writeVarU(0, header_packet.data); // stream_flags = none; no FIXED_FPS because we aren't guaranteeing same-size audio chunks
	writeBytes("", 0, header_packet.data); // codec_specific_data

	// stream_class = audio
	writeVarU(avparams.samplerate, header_packet.data); // samplerate_num
	writeVarU(1, header_packet.data); // samplerate_den
	writeVarU(avparams.channels, header_packet.data); // channel_count

	header_packet.flush();
}

void NutMuxer::writeFrame(const uint8_t* payload, unsigned int payloadlen, uint64_t pts, uint64_t ptsnum, uint64_t ptsden, int ptsindex, FILE *underlying)
{
	// create syncpoint
	NutPacket sync(NutPacket::Syncpoint, underlying);
	writeVarU(pts * 2 + static_cast<uint64_t>(ptsindex), sync.data); // global_key_pts
	writeVarU(1, sync.data); // back_ptr_div_16, this is wrong
	sync.flush();

	std::vector<uint8_t> frameheader;
	frameheader.push_back(0); // frame_code
	// frame_flags = FLAG_CODED, so:
	int flags = 0;
	flags |= 1 << 0; // FLAG_KEY
	if (payloadlen == 0)
		flags |= 1 << 1; // FLAG_EOR
	flags |= 1 << 3; // FLAG_CODED_PTS
	flags |= 1 << 4; // FLAG_STREAM_ID
	flags |= 1 << 5; // FLAG_SIZE_MSB
	flags |= 1 << 6; // FLAG_CHECKSUM
	writeVarU(flags, frameheader);
	writeVarU(ptsindex, frameheader); // stream_id
	writeVarU(pts + 256, frameheader); // coded_pts = pts + 1 << msb_pts_shift
	writeVarU(payloadlen, frameheader); // data_size_msb

    writeBE32(nutCRC32(frameheader), frameheader); // checksum
    size_t written = fwrite(frameheader.data(), 1, frameheader.size(), underlying);

	if (written != frameheader.size())
		debuglogstdio(LCF_DUMP | LCF_WARNING, "Incomplete header transfer to ffmpeg");

	if (payload) {
		written = fwrite(payload, 1, payloadlen, underlying);
		if (written != payloadlen)
			debuglogstdio(LCF_DUMP | LCF_WARNING, "Incomplete buffer transfer to ffmpeg");
	}
}

void NutMuxer::writeVideoFrame(const uint8_t* video, unsigned int len)
{
	debuglogstdio(LCF_DUMP, "Write nut video frame");
	debuglogstdio(LCF_DUMP, "Video pts is %f", (double)videopts * avparams.fpsden / avparams.fpsnum);

	writeFrame(video, len, videopts, static_cast<uint64_t>(avparams.fpsden), static_cast<uint64_t>(avparams.fpsnum), 0, output);
	videopts++;

}

void NutMuxer::writeAudioFrame(const uint8_t* samples, unsigned int len)
{
	debuglogstdio(LCF_DUMP, "Write nut audio frame");
	debuglogstdio(LCF_DUMP, "Audio pts is %f", (double)audiopts / avparams.samplerate);

	writeFrame(samples, len, audiopts, 1, static_cast<uint64_t>(avparams.samplerate), 1, output);

	audiopts += static_cast<uint64_t>(len) / static_cast<uint64_t>(avparams.samplesize);
}

NutMuxer::NutMuxer(int width, int height, int fpsnum, int fpsden, const char* pixfmt, int samplerate, int samplesize, int channels, FILE *underlying)
{
	avparams.width = width;
	avparams.height = height;
	avparams.fpsnum = fpsnum;
	avparams.fpsden = fpsden;
	avparams.reduce(); // timebases in nut MUST be relatively prime
	avparams.samplerate = samplerate;
	avparams.samplesize = samplesize;
	avparams.channels = channels;
	avparams.pixfmt = pixfmt;
	output = underlying;

	audiopts = 0;
	videopts = 0;

	writeMainHeader();
	writeVideoHeader();
	writeAudioHeader();

	videodone = false;
	audiodone = false;
}

void NutMuxer::finish()
{
	debuglogstdio(LCF_DUMP, "Write nut EOF frames");
	// writeVideoFrame(nullptr, 0);
	// writeAudioFrame(nullptr, 0);
}

}
