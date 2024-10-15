#include <csignal>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <chrono>
#include <iostream>
#include <string>
#include <cstring>
#include "DroneState.hpp"

#define VERSION 0.1

#define PIPE_NAME_CMD_LINE "./tmp/ddd-data-interchange0"
#define PIPE_NAME_KEY_HOOK "./tmp/ddd-data-interchange1"
#define BUFFER_SIZE 512
#define MIN_INTER_SEND 1.5
#ifdef __linux__
	#define SERIAL_PORT "/dev/ttyUSB0"
	#define BAUD_RATE 1000000
#elif __APPLE__
	#define SERIAL_PORT "/dev/cu.usbserial-0001"
	#define BAUD_RATE B115200
#else
	#error "Unsupported platform"
#endif

class SerialHandler {
public:
	SerialHandler();
	~SerialHandler();

	bool	setup();
	void	cleanup();
	void	transmit(int pipe, int pipeKey);
	int		createNamedPipe(std::string namePipe);

private:
	volatile	std::sig_atomic_t g_stopped;
	int			serial_port;
	int			fifo;
	int			fifoKey;

	int setup_serial();

};