Dancing Drones Dialogue with OptiTrack

./img/preview.png

A React web application designed for choreographing and visualizing drones accurately in space using the OptiTrack system. This project leverages real-time motion capture to control and preview drones, both in simulation and live flight. The backend is written in C++ and communicates with drones using ESP32 modules, ensuring seamless information flow between the system and the drones.

Features

Choreograph and visualize drone movements in a 3D space using a web interface.
Blender animation export integration to control and execute pre-programmed drone movements.
Live control and preview of drone positions and movements in real time.
Communication between C++ backend and drones via ESP32 devices.
Utilizes the OptiTrack motion capture system for accurate drone tracking.
Tech Stack

Backend
C++: Core logic and communication handling.
ESP32: Wireless communication between backend and drones (sending and receiving commands/data).
Frontend
React.js: Web app for visualizing and controlling the drones.
Three.js: 3D visualization for live drone choreography preview.
Additional Components
Blender: Integration for exporting drone choreography as animations.
OptiTrack: Motion capture system to track drone positions in real-time.
Installation

Backend
Clone the repository:
bash
Copy code
git clone https://github.com/yourusername/dancing-drones-dialogue.git
Navigate to the backend directory:
bash
Copy code
cd backend
Build the C++ backend:
bash
Copy code
make
Frontend
Navigate to the frontend directory:
bash
Copy code
cd frontend
Install dependencies:
bash
Copy code
npm install
Start the React development server:
bash
Copy code
npm run dev
Usage

Start the backend:
Run the backend server that controls communication with drones and the OptiTrack system.
Launch the frontend:
Access the web interface via the browser for choreographing and previewing drone movements.
ESP32 Communication:
Ensure the ESP32 devices are connected and ready to send/receive data to/from the drones.
Blender Integration:
Export animation files from Blender and import them into the system to choreograph drone movements.
