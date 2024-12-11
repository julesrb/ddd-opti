#include <string>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

// #include "DataBroker.h"
#include "DroneState.hpp"
#include "Log.hpp"
#include "PacketDefinition.hpp"
#include "SerialHandler.hpp"
#include "WsServer.hpp"

#define TAG	"Serial"

std::string getSerialPort()
{
	return SERIAL_PORT;
}

SerialHandler &SerialHandler::getInstance()
{
	static SerialHandler instance(getSerialPort());
	return instance;
}

SerialHandler::SerialHandler(std::string const &port)
	: port(port), port_fd(-1)
{}


SerialHandler::~SerialHandler()
{
	if (this->port_fd >= 0)
		(void)::close(this->port_fd);
}


bool SerialHandler::setup()
{
	this->port_fd = setup_serial();
	if (this->port_fd < 0)
	{
		ERROR(TAG, "Make sure the ESP32 is connected");
		return false;
	}
	INFO(TAG, "Serial port connected.");
	return true;
}


int SerialHandler::setup_serial()
{
	{
		std::string tmp("Connecting to serial port: ");
		tmp.append(SERIAL_PORT);
		INFO(TAG, tmp.c_str());
	}
	int const serial_port = ::open(this->port.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
	if (serial_port < 0)
	{
		ERROR(TAG, "Failed to open requested port.");
		return (-1);
	}
	struct termios tty;
	if (::tcgetattr(serial_port, &tty) != 0)
	{
		ERROR(TAG, "Failed to get term attributes.");
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

	(void)::cfsetispeed(&tty, BAUD_RATE);
	(void)::cfsetospeed(&tty, BAUD_RATE);

	if (::tcsetattr(serial_port, TCSANOW, &tty) != 0)
	{
		ERROR(TAG, "Failed to set term attributes.");
		return (-1);
	}
	return (serial_port);
}

void SerialHandler::parseTeleMsg(char* msg, ssize_t rb)
{
	int const id = static_cast<int>(msg[0]);
	int const index = static_cast<int>(msg[1]);

	if (id == S_TEL_BAT) {
		t_tel_bat bat = *reinterpret_cast<t_tel_bat*>(&msg[0]);

		std::stringstream ss;

		ss << index << "{\"bat\":[" << bat.volt << "," 
					<< bat.cur << "]}";

		sendFront(ss.str());
	}
	else if (id == S_TEL_ATITU) {
		t_tel_atitu atitu = *reinterpret_cast<t_tel_atitu*>(&msg[0]);
		std::stringstream ss;
		ss << index << "{\"ati\":[" << atitu.pitch << "," 
					<< atitu.roll << "," 
					<< atitu.yaw << "]}";

		sendFront(ss.str());
	}

	else if (id == S_TEL_RC) {
		t_tel_rc rc = *reinterpret_cast<t_tel_rc*>(&msg[0]);

		std::stringstream ss;

		ss << index << "{\"rc\":[" << rc.ch_1 << "," << rc.ch_2 << "," 
					<< rc.ch_3 << "," << rc.ch_4 << "," << rc.ch_5 << "]}";

		sendFront(ss.str());
	}
	else if (id == S_GRAPH) {
		t_graph graph = *reinterpret_cast<t_graph*>(&msg[0]);
		// Convert the struct to a raw string of bytes
		std::stringstream ss;
        ss << 9 << "{"
           << "\"xPWM\":" << graph.xPWM << ","
           << "\"yPWM\":" << graph.yPWM << ","
           << "\"zPWM\":" << graph.zPWM << ","
           << "\"yawPWM\":" << graph.yawPWM << ","
           << "\"xPID1\":" << graph.xPID1 << ","
           << "\"yPID1\":" << graph.yPID1 << ","
           << "\"zPID1\":" << graph.zPID1 << ","
           << "\"xPID2\":" << graph.xPID2 << ","
           << "\"yPID2\":" << graph.yPID2 << ","
           << "\"zPID2\":" << graph.zPID2 << ","
           << "\"yawPID2\":" << graph.yawPID2 << ","
           << "\"xPos\":" << graph.xPos << ","
           << "\"yPos\":" << graph.yPos << ","
           << "\"zPos\":" << graph.zPos << ","
		   << "\"yawPos\":" << graph.yawPos << ","
           << "\"xPosSetpoint\":" << graph.xPosSetpoint << ","
           << "\"yPosSetpoint\":" << graph.yPosSetpoint << ","
           << "\"zPosSetpoint\":" << graph.zPosSetpoint
           << "}";
        sendFront(ss.str());
		// std::cout << ss.str() <<std::endl;
		std::cout << "rb = " << rb << std::endl;

	}
	else
	{
		WARN(TAG, "Unrecognized message.");
		WARN(TAG, msg);
	}
}

// TODO: rewrite to fit into polling
void SerialHandler::monitorIncoming()
{
    ssize_t rb(0);
    char buf[BUFFER_SIZE];
    std::string msg;

    while (true)
    {
        usleep(500);
        if (port_fd < 0) continue;

        std::memset(buf, 0, sizeof(buf));  // Clear the buffer before each read
        rb = read(port_fd, buf, BUFFER_SIZE - 1);

        if (rb < 0) {
            std::cerr << "Error reading from serial." << std::endl;
            ERROR(TAG, "Error reading from serial.");
            continue;
        }

        // If the first read returns exactly 32 bytes, attempt two additional reads
        if (rb == 32) {
            ssize_t additionalBytes = read(port_fd, buf + rb, BUFFER_SIZE - rb - 1);
            if (additionalBytes < 0) {
                std::cerr << "Error reading additional bytes from serial." << std::endl;
                ERROR(TAG, "Error reading additional bytes from serial.");
                continue;
            }
            rb += additionalBytes;

            if (rb == 64) {  // If the total bytes read so far is 64, read again
                additionalBytes = read(port_fd, buf + rb, BUFFER_SIZE - rb - 1);
                if (additionalBytes < 0) {
                    std::cerr << "Error reading additional bytes from serial." << std::endl;
                    ERROR(TAG, "Error reading additional bytes from serial.");
                    continue;
                }
                rb += additionalBytes;
            }
        }

        buf[rb] = 0;  // Null-terminate the buffer to safely convert to string
        msg = buf;  // Store the message

        // Parse the message if it's not empty
        if (!msg.empty()) {
            parseTeleMsg(buf, rb);  // Call the parsing function with the buffer
        }
    }
}



ssize_t SerialHandler::send(std::string const &msg)
{
	ssize_t serial_rt;
	{
		std::lock_guard<std::mutex> guard(this->serialMutex);
		serial_rt = ::write(this->port_fd, msg.c_str(), msg.length());
		this->sendFront(msg);
		std::this_thread::sleep_for(std::chrono::milliseconds(10)); // to space out wifi packet.
	}
	return (serial_rt);
}

int SerialHandler::sendFront(std::string const &msg)
{
	WsServer::getInstance().send(msg);
	// INFO(TAG, msg.c_str());
	return (1);
}
