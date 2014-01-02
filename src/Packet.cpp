#include <stdlib.h>
#include <utility>

#include "Packet.hpp"

namespace ytst {
	Packet::Packet() {
		av_init_packet(&packet);
		packet.size = 0;
		packet.data = nullptr;
	}

	Packet::Packet(Packet&& other) : packet(std::move(other.packet)) {
		other.packet.data = nullptr;
	}

	Packet::~Packet() {
		reset();
	}

	void Packet::reset() {
		av_free_packet(&packet);
		packet.data = nullptr;
	}
}
