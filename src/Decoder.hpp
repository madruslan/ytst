#ifndef YTST_DECODER_HPP
#define YTST_DECODER_HPP

#include <memory>

#include "Packet.hpp"

extern "C" {
#include <libavformat/avformat.h>
}

namespace ytst {
	class Decoder {
		const char* infile;
		std::shared_ptr<AVFormatContext> avFormat;
		std::shared_ptr<AVCodecContext> avAudioCodec;
		std::shared_ptr<AVFrame> avFrame;

		Packet packet;
		AVStream* audioStream;

	public:
		Decoder(const char* infile);
		std::shared_ptr<AVCodecContext> read_file();
		AVFrame* decode_frame();
	};
}

#endif
