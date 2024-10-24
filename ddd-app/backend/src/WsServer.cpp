#include "WsServer.hpp"


WsServer::WsServer(std::vector<DroneState> &ref) : port(8080), drones(ref) {}

WsServer::~WsServer() {}

void WsServer::settingWsPort(char* arg) {
	port = std::atoi(arg);
	std::cout << "WebSocket port = " << port << std::endl;
}


void WsServer::settingWsConnection() {

	CROW_ROUTE(app, "/ws")
	.websocket(&app)
	.onopen([this](crow::websocket::connection& conn) {
		(void)conn;
		if (wsConn != nullptr) {
			std::cerr << "A WebSocket connection is already active. Not opening another connection." << std::endl;
			return;
		}
		std::cout << "WebSocket connection opened" << std::endl;
		std::lock_guard<std::mutex> guard(wsMutex); // Lock to protect access
		wsConn = &conn; 	// Store the connection in the shared pointer
		for (size_t i = 0; i < drones.size(); i++) {
			drones[i].sendAll();
		}
		
	})
	.onmessage([this](crow::websocket::connection& conn, const std::string& message, bool is_binary) {
		(void)is_binary; // Ignore binary data for now
		(void)this;
		(void)conn;
		std::cout << "Received message: " << message << std::endl;

		if (message.empty()) {
			std::cerr << "Error: JSON input is empty!" << std::endl;
			return;
		}

		// Parse message
		std::string number_str;
		number_str += message[0];
		int index = std::stoi(number_str);
		std::string wihtoutIndex = message.substr(1);
		std::cout << wihtoutIndex << std::endl;

		try {
			json data = json::parse(wihtoutIndex);
			if (data.contains("trim")) {
				std::cout << "Parameter 'trim' exists: " << data["trim"] << std::endl;
				// Add your logic for handling 'trim'
			}

			if (data.contains("light")) {
				std::cout << "Parameter 'light' exists: " << data["light"] << std::endl;
				// Add your logic for handling 'light'
			}

			if (data.contains("setpoint")) {
				std::cout << "Parameter 'setpoint' exists: " << data["setpoint"] << std::endl;
				// Add your logic for handling 'setpoint'
			}

			if (data.contains("armed")) {
				if (data["armed"].get<bool>()) 
					drones[index].startup();
				else
					drones[index].disarm();
			}

			if (data.contains("path")) {
				std::cout << "Parameter 'path' exists: " << data["path"] << std::endl;
				if (data["path"] == "send")
					drones[index].path->sendFrameByFrame();
			}
		} catch (nlohmann::json::parse_error& e) {
			std::cerr << "Parse error: " << e.what() << std::endl;
		}

		// conn.send_text(message);
	})
	.onclose([this](crow::websocket::connection& conn, const std::string& reason) {
		(void)conn;
		std::cout << "WebSocket connection closed: " << reason << std::endl;
		std::lock_guard<std::mutex> guard(wsMutex);
		wsConn= nullptr;
	});
}