#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <thread>
#include <chrono>
#include <GLFW/glfw3.h>
#include "core_data.h"
#include "pathfinder.h" 
#include "perlin.h"

// --- CAMERA ---
float camX = 15.0f, camY = 15.0f, camZ = 60.0f;
float yaw = -90.0f;
float pitch = 0.0f; 
float zoomFov = 45.0f;

double lastMouseX = 400, lastMouseY = 300;
bool firstMouse = true;
bool showSearchVolume = true; 

float deltaTime = 0.0f;
float lastFrame = 0.0f;

int startEndArr[6] = {2, 2, 2, 27, 2, 2};

const int GRID_SIZE = 30;
std::vector<Node> grid;
Pathfinder3D* pathfinder = nullptr;

// --- INPUT ---
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastMouseX = xpos;
        lastMouseY = ypos;
        firstMouse = false;
    }

    float sensitivity = 0.15f;
    yaw   += -(xpos - lastMouseX) * sensitivity;
    pitch += (lastMouseY - ypos) * sensitivity; // reversed: up = positive

    if (pitch >  89.0f) pitch =  89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    lastMouseX = xpos;
    lastMouseY = ypos;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    zoomFov -= static_cast<float>(yoffset) * 2.0f;
    if (zoomFov <   1.0f) zoomFov =   1.0f;
    if (zoomFov > 120.0f) zoomFov = 120.0f;
}



void processContinuousInput(GLFWwindow* window) {
    float speed = 25.0f * deltaTime;

    float radYaw   = yaw   * (3.14159265f / 180.0f);
    float radPitch = pitch * (3.14159265f / 180.0f);

    // 1. FRONT: exact direction the camera is pointing (follows pitch + yaw)
    float fX = cos(radYaw) * cos(radPitch);
    float fY = sin(radPitch);
    float fZ = sin(radYaw) * cos(radPitch);

    // 2. RIGHT: perpendicular horizontal strafe (cross of front x world-up)
    //    rY is always 0 so strafing stays flat regardless of pitch
    float rX = -fZ;
    float rY =  0.0f;
    float rZ =  fX;
    float rLen = sqrt(rX*rX + rZ*rZ);
    if (rLen > 0.0001f) { rX /= rLen; rZ /= rLen; }

    // 3. LOCAL UP: perpendicular to both right and front (cross of right x front)
    //    This tilts with the camera, so Q/E feel natural at any pitch angle
    float uX = (rY * fZ) - (rZ * fY);   // rY = 0, simplifies but kept explicit
    float uY = (rZ * fX) - (rX * fZ);
    float uZ = (rX * fY) - (rY * fX);
    float uLen = sqrt(uX*uX + uY*uY + uZ*uZ);
    if (uLen > 0.0001f) { uX /= uLen; uY /= uLen; uZ /= uLen; }

    // W/S: fly forward/backward along exact look direction (dives and climbs)
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { camX -= fX*speed; camY += fY*speed; camZ += fZ*speed; }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { camX += fX*speed; camY -= fY*speed; camZ -= fZ*speed; }

    // A/D: strafe left/right (purely horizontal, rY=0 so camY unaffected)
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { camX -= rX*speed; camY += rY*speed; camZ += rZ*speed; }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { camX += rX*speed; camY -= rY*speed; camZ -= rZ*speed; }

    // Q/E: move along local camera up/down (perpendicular to look direction)
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) { camX -= 0; camY -= speed; camZ -= 0; }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) { camX += 0; camY += speed; camZ += 0; }
}

// --- VISUALS ---
void drawGridBounds() {
    glColor3f(0.3f, 0.3f, 0.3f); glLineWidth(2.0f); glBegin(GL_LINES);
    glVertex3f(0, 0, 0); glVertex3f(GRID_SIZE, 0, 0); glVertex3f(0, GRID_SIZE, 0); glVertex3f(GRID_SIZE, GRID_SIZE, 0);
    glVertex3f(0, 0, GRID_SIZE); glVertex3f(GRID_SIZE, 0, GRID_SIZE); glVertex3f(0, GRID_SIZE, GRID_SIZE); glVertex3f(GRID_SIZE, GRID_SIZE, GRID_SIZE);
    glVertex3f(0, 0, 0); glVertex3f(0, GRID_SIZE, 0); glVertex3f(GRID_SIZE, 0, 0); glVertex3f(GRID_SIZE, GRID_SIZE, 0);
    glVertex3f(0, 0, GRID_SIZE); glVertex3f(0, GRID_SIZE, GRID_SIZE); glVertex3f(GRID_SIZE, 0, GRID_SIZE); glVertex3f(GRID_SIZE, GRID_SIZE, GRID_SIZE);
    glVertex3f(0, 0, 0); glVertex3f(0, 0, GRID_SIZE); glVertex3f(GRID_SIZE, 0, 0); glVertex3f(GRID_SIZE, 0, GRID_SIZE);
    glVertex3f(0, GRID_SIZE, 0); glVertex3f(0, GRID_SIZE, GRID_SIZE); glVertex3f(GRID_SIZE, GRID_SIZE, 0); glVertex3f(GRID_SIZE, GRID_SIZE, GRID_SIZE);
    glEnd();
}

void drawVoxel(float x, float y, float z, float size, float r, float g, float b) {
    float h = size / 2.0f; 
    glBegin(GL_QUADS);
    glColor3f(r * 1.2f, g * 1.2f, b * 1.2f); glVertex3f(x-h, y+h, z-h); glVertex3f(x-h, y+h, z+h); glVertex3f(x+h, y+h, z+h); glVertex3f(x+h, y+h, z-h);
    glColor3f(r, g, b); glVertex3f(x-h, y-h, z+h); glVertex3f(x+h, y-h, z+h); glVertex3f(x+h, y+h, z+h); glVertex3f(x-h, y+h, z+h);
    glVertex3f(x-h, y-h, z-h); glVertex3f(x-h, y+h, z-h); glVertex3f(x+h, y+h, z-h); glVertex3f(x+h, y-h, z-h);
    glColor3f(r * 0.8f, g * 0.8f, b * 0.8f); glVertex3f(x+h, y-h, z-h); glVertex3f(x+h, y+h, z-h); glVertex3f(x+h, y+h, z+h); glVertex3f(x+h, y-h, z+h);
    glVertex3f(x-h, y-h, z-h); glVertex3f(x-h, y-h, z+h); glVertex3f(x-h, y+h, z+h); glVertex3f(x-h, y+h, z-h);
    glColor3f(r * 0.6f, g * 0.6f, b * 0.6f); glVertex3f(x-h, y-h, z-h); glVertex3f(x+h, y-h, z-h); glVertex3f(x+h, y-h, z+h); glVertex3f(x-h, y-h, z+h);
    glEnd();
}

void initializeGrid() {
    grid.clear();
    grid.reserve(GRID_SIZE * GRID_SIZE * GRID_SIZE);
    PerlinNoise perlin;
    float scale = 0.15f; 

    for (int z = 0; z < GRID_SIZE; z++) {
        for (int y = 0; y < GRID_SIZE; y++) {
            for (int x = 0; x < GRID_SIZE; x++) {
                Node n(x, y, z);
                double noiseValue = perlin.noise(x * scale, y * scale, z * scale);
                if (noiseValue > 0.1) n.is_obstacle = true;
                if (x >= 1 && x <= 3 && y >= 1 && y <= 3 && z >= 1 && z <= 3) n.is_obstacle = false; 
                if (x >= 26 && x <= 28 && y >= 26 && y <= 28 && z >= 26 && z <= 28) n.is_obstacle = false; 
                grid.push_back(n);
            }
        }
    }
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    
    // [V] Toggle X-Ray mode
    if (key == GLFW_KEY_V && action == GLFW_PRESS) {
        showSearchVolume = !showSearchVolume;
    }

    // [R] Reset Camera Position
    if (key == GLFW_KEY_R) {
        std::cout << "Camera Reset." << std::endl;
        camX = 15.0f; camY = 15.0f; camZ = 60.0f;
        yaw = -90.0f; pitch = 0.0f;
    }

    // [F] Reset Algorithms (Clear the green/red blocks but keep the maze)
    if (key == GLFW_KEY_F) {
        std::cout << "Algorithms Cleared." << std::endl;
        if (pathfinder) pathfinder->reset_search();
    }

    // [H] Generate a completely new Perlin Noise Lattice
    if (key == GLFW_KEY_H) {
        std::cout << "Generating new Perlin map..." << std::endl;
        initializeGrid(); 
        if (pathfinder) {
            delete pathfinder;
            pathfinder = new Pathfinder3D(grid, GRID_SIZE);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Check if a search is already running so we don't crash by starting a new one
    if (pathfinder && pathfinder->is_searching) return;

    // Press 1 for A*
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        std::cout << "Starting A* Search..." << std::endl;
        pathfinder->start_search(startEndArr, AlgoType::ASTAR);
    }
    // Press 2 for Dijkstra
    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        std::cout << "Starting Dijkstra's Algorithm..." << std::endl;
        pathfinder->start_search(startEndArr, AlgoType::DIJKSTRA);
    }
    // Press 3 for Greedy Best-First
    if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
        std::cout << "Starting Greedy Best-First Search..." << std::endl;
        pathfinder->start_search(startEndArr, AlgoType::GREEDY);
    }
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}




int main() {
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(1000, 800, "3D Engine - Free Flight Camera", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }

    glfwMakeContextCurrent(window);

    // Capture and hide the mouse cursor for FPS-style look
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback); 
    
    glEnable(GL_DEPTH_TEST);
    initializeGrid(); 
    pathfinder = new Pathfinder3D(grid, GRID_SIZE);

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processContinuousInput(window); 
        pathfinder->step();

        glClearColor(0.05f, 0.06f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int width, height; 
        glfwGetFramebufferSize(window, &width, &height); 
        glViewport(0, 0, width, height);
        
        // Projection
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        float aspect = static_cast<float>(width) / static_cast<float>(height > 0 ? height : 1);
        float fH = tan(zoomFov / 360.0f * 3.14159f) * 0.1f;
        float fW = fH * aspect;
        glFrustum(-fW, fW, -fH, fH, 0.1f, 1000.0f);

        // Camera view
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glRotatef(-pitch, 1.0f, 0.0f, 0.0f);
        glRotatef(-yaw - 90.0f, 0.0f, 1.0f, 0.0f);
        glTranslatef(-camX, -camY, -camZ);

        // Draw scene
        drawGridBounds();
        float voxelSize = 0.7f; 
        for (const auto& node : grid) {
            if (node.is_obstacle) {
                drawVoxel(node.x + 0.5f, node.y + 0.5f, node.z + 0.5f, voxelSize, 0.4f, 0.4f, 0.4f);
            } else if (showSearchVolume) {
                if      (node.in_closed) { drawVoxel(node.x + 0.5f, node.y + 0.5f, node.z + 0.5f, voxelSize, 0.6f, 0.2f, 0.2f); }
                else if (node.in_open)   { drawVoxel(node.x + 0.5f, node.y + 0.5f, node.z + 0.5f, voxelSize, 0.2f, 0.6f, 0.2f); }
            }
        }
        
        drawVoxel(
            startEndArr[0] + 0.5f,
            startEndArr[1] + 0.5f,
            startEndArr[2] + 0.5f,
            0.9f, 0.0f, 1.0f, 0.0f
        );
        drawVoxel(
            startEndArr[3] + 0.5f,
            startEndArr[4] + 0.5f,
            startEndArr[5] + 0.5f,
            0.9f, 1.0f, 0.0f, 0.0f
        ); 

        if (pathfinder->path_found && !pathfinder->final_path.empty()) {
            glDisable(GL_DEPTH_TEST); 
            glColor3f(1.0f, 0.8f, 0.0f); 
            glLineWidth(8.0f); 
            glBegin(GL_LINE_STRIP);
            for (Node* p : pathfinder->final_path) { glVertex3f(p->x + 0.5f, p->y + 0.5f, p->z + 0.5f); }
            glEnd();
            glEnable(GL_DEPTH_TEST); 
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete pathfinder;
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}