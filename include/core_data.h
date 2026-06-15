#pragma once
#include <cstddef>
#include <cmath>

// This struct holds everything our pathfinder needs to know about a single coordinate in space.
struct Node {
    int x, y, z;
    bool is_obstacle = false;

    // Pathfinding metrics (A* variables)
    float g = INFINITY;
    float h = 0.0f;
    float f = INFINITY;
    Node* parent = nullptr;

    // Visualization states (so we can draw them in different colors later)
    bool in_open = false;
    bool in_closed = false;

    // Heap optimization (O(1) look-ups)
    static constexpr std::size_t invalid_heap_index = static_cast<std::size_t>(-1);
    std::size_t heap_index = invalid_heap_index;

    // Constructor to easily create a node
    Node(int _x, int _y, int _z) : x(_x), y(_y), z(_z) {}
    
    // Default constructor needed for vectors
    Node() : x(0), y(0), z(0) {} 
};