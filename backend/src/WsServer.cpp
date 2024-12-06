#include <algorithm>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "json.hpp"
#include "ws.h"

#include "DroneControl.hpp"
#include "Log.hpp"
#include "WsServer.hpp"

#define WS_PORT	4242
#define WS_HOST	"0.0.0.0"
#define TAG		"WSServ"

WsServer &WsServer::getInstance()
{
	static WsServer instance(WS_PORT, WS_HOST);
	return instance;
}

WsServer::WsServer(int port, char const *host) : server(), clients()
{
	this->server.host = host;
	this->server.port = static_cast<uint16_t>(port);
	this->server.thread_loop = 1;
	this->server.timeout_ms = 1000;
	this->server.evs.onopen = &(this->onopen);
	this->server.evs.onclose = &(this->onclose);
	this->server.evs.onmessage = &(this->onmessage);
}

WsServer::~WsServer() {}


void WsServer::start()
{
	ws_socket(&this->server);
	// std::cout << "WebSocket Server listening on port: " << this->server.port << std::endl;
	std::string tmp("WebSocket Server listening on port: ");
	tmp.append(std::to_string(this->server.port));
	INFO(TAG, tmp.c_str());
}

void WsServer::send(std::string const &msg, size_t client)
{
	if (client < this->clients.size())
		ws_sendframe_txt(this->clients[client], msg.c_str());
}

void WsServer::add_client(ws_cli_conn_t client)
{
	this->clients.push_back(client);
}

void WsServer::remove_client(ws_cli_conn_t client)
{
	auto it = std::find(this->clients.begin(), this->clients.end(), client);
	if (it != this->clients.end())
		this->clients.erase(it);
}

void WsServer::onopen(ws_cli_conn_t client)
{
	WsServer::getInstance().add_client(client);
	INFO(TAG, "Client connected");
}

void WsServer::onclose(ws_cli_conn_t client)
{
	WsServer::getInstance().remove_client(client);
	INFO(TAG, "Client disconnected");
}

using json = nlohmann::json;

void WsServer::onmessage(ws_cli_conn_t client, const unsigned char *msg, \
	uint64_t size, int type)
{
	(void)client;
	(void)type;
	if (size < 2) {
		return;
	}

	// NOTE:
	// Messages starting with '*' are position updates from NatNet Client
	// Messages starting with drone index number are instructions received for that drone
	int const idx = msg[0] - '0';
	if (idx < 0)
	{
		std::string tmp(reinterpret_cast<const char*>(msg + 1), size - 1);
		// INFO(TAG, tmp.c_str());
		float p[3];
		float q[4];
		int drone_idx;
		char c;
		std::stringstream ss(tmp);
		ss >> drone_idx
			>> c >> p[0]
			>> c >> p[2]
			>> c >> p[1]
			>> c >> c >> q[0]
			>> c >> q[1]
			>> c >> q[2]
			>> c >> q[3];
		// drone00 is received as asset #1
		DroneControl::getInstance().update(drone_idx - 1, p, q);
	}
	else
	{
		try
		{
			std::string tmp(reinterpret_cast<char const *>(msg + 1), size - 1);
			json data = json::parse(tmp);
			DroneControl::getInstance().update(idx, data);
		}
		catch (nlohmann::json::parse_error& e)
		{
			std::string	tmp("Parsing error: ");
			tmp.append(e.what());
			ERROR(TAG, tmp.c_str());
		}
	}
}
