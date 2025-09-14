#pragma once
#include <string>
#include <unordered_map>

struct EightbitState {
	double damp1 = 0.1;
	bool broadcastPackets = false;
	uint16_t port = 4000;
	std::string ip = "127.0.0.1";
	std::unordered_map<int, std::tuple<IVoiceCodec*, int>> afflictedPlayers;
};
