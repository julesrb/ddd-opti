#include "Path.hpp"

// Path::~Path() {}

Path::Path(std::string file_path, DroneState & ref) : drone(ref), sending(false)
{
	std::ifstream inputFile(file_path);
	std::string str((std::istreambuf_iterator<char>(inputFile)),
						std::istreambuf_iterator<char>());
	jsonStr = str;

	if (jsonStr.empty()) {
		std::cerr << "Error: JSON input is empty!" << std::endl;
		return;
	}

	// Parse frames
	try {
		json data = json::parse(jsonStr);
		this->name = data["path_name"];
		this->length = data["length"];
		this->fps = data["fps"];
		for (const auto& frame : data["frames"]) {
			FrameData f;
			f.frame = frame["frame"];
			f.location = {
				frame["location"]["x"],
				frame["location"]["y"],
				frame["location"]["z"]
			};
			f.rotation = {
				0.0, // Default to 0.0 for x
				0.0, // Default to 0.0 for y
				frame["rotation"]["z"] // Only use z from the JSON
			};
			float power = frame["light"]["power"];
			f.light = {
				power / 10,
				frame["light"]["angle"]
			};
			frames.push_back(f);
		}
	} catch (nlohmann::json::parse_error& e) {
		std::cerr << "Parse error: " << e.what() << std::endl;
	}


}

std::vector<FrameData> & Path::getFrames() {
	return frames;
}

int Path::sendFrameByFrame() {
	if (sending) {
		std::cout << "sending already" << std::endl;
		return 0;
	}

    // Create a new thread for sending frames
     std::thread sendAllFrames([this]() {
            std::vector<FrameData>& frames = getFrames();
            for (size_t i = 0; i < length; i++) {
                std::cout << "about the send frame " << i << std::endl;
                std::stringstream ssSerial;
                ssSerial << "\"setpoint\":[" << frames[i].location.x << "," << frames[i].location.y << "," << frames[i].location.z << "],"
                          << "\"light\":[" << frames[i].light.angle << "," << frames[i].light.power << "]";

                if (drone.send(ssSerial.str().c_str()) < 0)
                    std::cout << "serial send failed " << i << std::endl;

                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
		sending = false;
        });
    sendAllFrames.detach(); // Detach the thread
    return 0;
}


Path::Path(Path const &cpy) : drone(cpy.drone){
	(void)cpy;
	// TODO ?
}

Path & Path::operator=(Path const &rhs) {
	(void)rhs;
	return *this;
	//TODO ?
}





