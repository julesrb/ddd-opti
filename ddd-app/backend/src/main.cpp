#include "DroneState.hpp"
#include "SerialHandler.hpp"
#include "Path.hpp"
#include "crow_all.h"

#include <thread>
#include <iostream>
#include <memory>
#include <mutex>

// Shared pointer for WebSocket connection
crow::websocket::connection* wsConn = nullptr;
std::mutex wsMutex; // Mutex to protect the WebSocket connection

int settingWsPort(int argc, char ** argv) {
	int wsport = 8080;
	if (argc == 2)
		wsport = std::atoi(argv[1]);
	std::cout << "port = " << wsport << std::endl;
	return wsport;
}

crow::SimpleApp & settingWsConnection() {
	static crow::SimpleApp app;

	CROW_ROUTE(app, "/ws")
	.websocket(&app)
	.onopen([](crow::websocket::connection& conn) {
		(void)conn;
		std::cout << "WebSocket connection opened" << std::endl;
		// Store the connection in the shared pointer
		std::lock_guard<std::mutex> guard(wsMutex); // Lock to protect access
		std::cout << "wsconn ptr = " << wsConn << std::endl;
		wsConn = &conn; // Save the connection
	})
	.onmessage([](crow::websocket::connection& conn, const std::string& message, bool is_binary) {
		(void)is_binary; // Ignore binary data for now
		std::cout << "Received message: " << message << std::endl;

		// Echo the message back to the client
		conn.send_text("Message received: " + message);
	})
	.onclose([](crow::websocket::connection& conn, const std::string& reason) {
		(void)conn;
		std::cout << "WebSocket connection closed: " << reason << std::endl;
		std::lock_guard<std::mutex> guard(wsMutex);
		wsConn= nullptr;
	});
	return app;
}

int main(int argc, char ** argv) {

	int wsport = settingWsPort(argc, argv);


	std::cout << "Setting up serial connection..." << std::endl;
	SerialHandler handler(wsMutex, wsConn);
	if (!handler.setup()) {
		std::cerr << "Failed to initialize serial connection." << std::endl;
		std::cerr << "\033[1;31mMake sure the ESP32 is connected\033[0m" << std::endl;
	}
	int fifoCmd = handler.createNamedPipe(PIPE_NAME_CMD_LINE);
	int fifoKey = handler.createNamedPipe(PIPE_NAME_KEY_HOOK);
	if (fifoCmd < 0 || fifoKey < 0) return 1;
	// Launch the transmitData function in a separate thread
	std::thread transmitThread([&handler, fifoCmd, fifoKey]() {
		handler.transmit(fifoCmd, fifoKey);
	});


	// WebSocket server with Crow
	crow::SimpleApp & app = settingWsConnection();
	// Run the Crow app in another thread so it doesn't block the main thread
	std::thread crowThread([&app, wsport]() {
		app.port(wsport).multithreaded().run();
	});


	// Do other stuff in the main thread if needed
	std::cout << "Main thread doing other work..." << std::endl;
	Path path1("animation.json");

	sleep(10); // std::this_thread::sleep_for(std::chrono::seconds(3));

	path1.send(wsConn);


	int i = 0;
	while (true) { // test to see if the connectio is working
		(void)i;
		//  std::this_thread::sleep_for(std::chrono::seconds(1));
		// std::lock_guard<std::mutex> guard(wsMutex);
		// if (wsConn) { // If the connection exists
		//     std::cout << "Sending message from main thread..." << std::endl;
		// 	std::stringstream ss;
		// 	ss << "Data " << i << " from main thread";
		//     wsConn->send_text(ss.str());
		// 	i++;
		// } else {
		//     std::cout << "No active WebSocket connection" << std::endl;
		// }
	}



	transmitThread.join();
	crowThread.join();
	close(fifoCmd);
	close(fifoKey);
	unlink(PIPE_NAME_CMD_LINE);
	unlink(PIPE_NAME_KEY_HOOK);

	return 0;
}
