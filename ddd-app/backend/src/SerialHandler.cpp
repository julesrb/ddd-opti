#include "SerialHandler.hpp"

SerialHandler::SerialHandler(WsServer & ref)
	: wsServer(ref),
	g_stopped(false),
	serial_port(-1),
	fifo(-1),
	fifoKey(-1)
	{}


SerialHandler::~SerialHandler() {
	if (serial_port >= 0) close(serial_port);
}


bool SerialHandler::setup() {
	std::cout << "Setting up serial connection..." << std::endl;
	serial_port = setup_serial();
	if (serial_port < 0) {
		std::cerr << "Failed to initialize serial connection." << std::endl;
		std::cerr << "\033[1;31mMake sure the ESP32 is connected\033[0m" << std::endl;
		return false;
	}
	std::cerr << "Serial connection Sucessfull." << std::endl;
	return true;
}


int SerialHandler::setup_serial() {
	std::cout << "Attempting to open serial port: " << SERIAL_PORT << std::endl;
	int serial_port = open(SERIAL_PORT, O_RDWR | O_NOCTTY | O_SYNC);
	if (serial_port < 0) {
		printf("Error %i from open: %s\n", errno, strerror(errno));
		return (-1);
	}
	std::cout << "Serial port opened successfully." << std::endl;

	struct termios tty;
	if (tcgetattr(serial_port, &tty) != 0) {
		printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
		return (-1);
	}

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8-bit characters
	tty.c_iflag &= ~IGNBRK;                     // disable break processing
	tty.c_lflag = 0;     // no signaling chars, no echo, no canonical processing
	tty.c_oflag = 0;     // no remapping, no delays
	tty.c_cc[VMIN] = 0;  // read doesn't block
	tty.c_cc[VTIME] = 5; // 0.5 seconds read timeout
	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl
	tty.c_cflag |= (CLOCAL | CREAD);   // ignore modem controls, enable reading
	tty.c_cflag &= ~(PARENB | PARODD); // shut off parity
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;

	cfsetispeed(&tty, BAUD_RATE);
	cfsetospeed(&tty, BAUD_RATE);

	if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
		printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
		return (-1);
	}
	return (serial_port);
}

void SerialHandler::monitorIncoming() {
	ssize_t rb(0);
	char buf[BUFFER_SIZE];
	std::string msg;

	while (!g_stopped) {
		usleep(500);
		if (serial_port < 0) continue;
		rb = read(serial_port, buf, BUFFER_SIZE - 1);
		if (rb > 0) {
			buf[rb] = 0;
			msg = buf;
			std::memset(buf, 0, sizeof(buf));
			if (!msg.empty())
				std::cout << msg << std::endl;
		}
		else if (rb < 0) {
			std::cerr << "Error reading from serial." << std::endl;
		}
	}
}

int SerialHandler::send(std::string const &msg) {

	std::lock_guard<std::mutex> guardSerial(serialMutex);
	int serial_rt = write(serial_port, msg.c_str(), msg.length());

	// FOR NOW JUST ECHO ANY SERIAL SEND TO THE FRONT
	// std::lock_guard<std::mutex> guardWs(wsServer.wsMutex); // TODO look into the mutex.
	if (wsServer.wsConn) {
		wsServer.wsConn->send_text(msg.c_str());
	}
	return (serial_rt);
}