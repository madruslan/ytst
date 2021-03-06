#include <thread>
#include <iostream>

extern "C" {
#include "uuid.h"
}

#include "log.h"

#include "youtube_stream.h"
#include "youtube_downloader.h"

#include "stream/decoder.h"
#include "stream/mpeg_encoder.h"
#include "stream/packet.h"


const size_t UUID_LENGTH = 36;

namespace ytst {
	YoutubeStream::YoutubeStream(const std::string& fifo_directory,
		       std::shared_ptr<PythonSupervisor> python_supervisor,
		       std::atomic<bool>& stream_running,
		       HttpResponseWriter& writer) : 
		fifo_directory(fifo_directory),
		python_supervisor(python_supervisor),
		stream_running(stream_running),
		writer(writer) { }

	YoutubeStream::~YoutubeStream() {
	}

	void YoutubeStream::stream(const std::string& id) {
		std::string infile = fifo_location();
		std::string url = youtube_url(id);

		ytst::YoutubeStream::Fifo fifo(infile);

		LOG(logINFO) << "Starting video download";

		ytst::YoutubeDownloader downloader(url, infile, python_supervisor);
		downloader.download();

		LOG(logINFO) << "Starting decoder";
		ytst::Decoder decoder(infile);
		auto decoder_ctxt = decoder.read_file();

		LOG(logINFO) << "Starting encoder";
		ytst::MPEGEncoder encoder(decoder_ctxt, MPEGEncoder::LAYER3, 128000);
		encoder.open_encoder();
		
		AVFrame* frame;
		ytst::Packet packet;
		LOG(logINFO) << "Begin decoding";

		while ((frame = decoder.decode_frame()) != nullptr && stream_running) {
			try {
				encoder.encode_frame(frame, packet);
				if (packet.packet.size > 0) {
					std::shared_ptr<Buffer> buf(new Buffer(reinterpret_cast<const char*>(packet.packet.data), packet.packet.size));
					writer.write_buffer(buf);
				}
				packet.reset();
			} catch(std::runtime_error e) {
				LOG(logERROR) << "Encoding exception: " << e.what();
			}
		}
		writer.write_last_chunk();

		LOG(logINFO) << "Done decoding";
	}

	std::string YoutubeStream::youtube_url(const std::string& id) {
		return std::string("https://www.youtube.com/watch?v=") + id;
	}

	std::string YoutubeStream::fifo_location() {
		// XXX: Not OS independent!
		return std::string(fifo_directory + "/" + stream_id() + ".mp4");
	}
	
	std::string YoutubeStream::stream_id() {
		char buf[UUID_LENGTH];
		afsUUID uuid;

		uuid_create(&uuid);
		uuid_to_string(&uuid, buf, UUID_LENGTH);

		return std::string(buf, UUID_LENGTH);
	}
}
