3D Spatial Pathfinding Engine
A high-performance, interactive 3D voxel engine built from scratch in C++ and OpenGL. This project visualizes real-time pathfinding algorithms (A*, Dijkstra, Greedy Best-First) navigating through procedurally generated Perlin noise cave networks.
It features a custom-built 6-DOF (Degrees of Freedom) Free-Flight camera, cache-optimized memory structures, and a custom $O(1)$ Min-Heap priority queue for lightning-fast algorithm execution.
✨ Core Features

Multi-Algorithm Racing: Switch between A*, Dijkstra's, and Greedy Best-First Search on the fly to visually compare their exploration behaviors and efficiency.
Procedural Voxel Terrain: Uses a custom 3D Perlin Noise mathematical implementation to generate organic, fully navigable cave systems and floating islands rather than basic random obstacles.
Cache-Optimized Memory: The 30x30x30 spatial grid is flattened into a 1D std::vector, keeping the data localized in the CPU cache for maximum algorithmic throughput.
True Free-Flight Camera: Navigate the 3D space using custom Matrix/Vector mathematics (gluLookAt and Cross-Products) for incredibly smooth First-Person spatial flight.
X-Ray Path Rendering: Toggle the search volume visibility in real-time to see exactly how the winning path weaves through the solid Perlin geometry.
🎮 Controls
Movement:

W / S : Move Forward / Backward (Relative to viewing angle)
A / D : Strafe Left / Right
E / Q : Elevate Up / Down (Relative to global axis)
Mouse : Pitch and Yaw (Look around)
Scroll : Adjust Field of View (Zoom)
Engine Controls:

1 : Run A* Search
2 : Run Dijkstra's Algorithm
3 : Run Greedy Best-First Search
Spacebar : Run default search (A*)
V : Toggle X-Ray mode (Hide/Show explored search volume)
H : Regenerate the 3D Perlin Noise terrain
F : Flush/Reset the algorithms (Clears the path, keeps the maze)
R : Reset the camera to the default starting position
🏗️ Architecture & Technology Stack

Language: C++17
Graphics API: OpenGL (Fixed-Function Pipeline for raw geometry rendering)
Windowing & Input: GLFW3
Build System: CMake (utilizes FetchContent for seamless dependency management)
Memory Management: Strictly type-safe architecture using std::size_t and std::vector.
Pathfinding Backend: Custom Pathfinder3D class powered by a custom sift-up/sift-down MinHeap.
🚀 How to Build and Run
This project uses CMake, making it incredibly easy to compile on any system without manually downloading libraries.
Prerequisites:

A C++ compiler (MSVC, GCC, or Clang)
CMake (v3.25 or higher)
Git (must be installed for CMake to fetch GLFW)
VS Code Instructions:

Clone this repository and open the folder in Visual Studio Code.
Install the CMake Tools and C/C++ extensions by Microsoft.
Allow CMake to configure the project (it will automatically download GLFW from GitHub).
Click the Build button on the bottom blue status bar.
Click the Play / Launch button to start the engine. 🧠 The Math Behind the Engine The Heuristic: The engine uses standard 3D Euclidean distance for spatial mapping: $h = \sqrt{(x_2 - x_1)^2 + (y_2 - y_1)^2 + (z_2 - z_1)^2}$ The Camera: To achieve 6-DOF flight, the engine dynamically calculates the Forward, Right, and Up vectors every frame. The Right and Up vectors are derived using cross products to ensure the camera never suffers from gimbal lock or axis tearing during extreme pitch angles.