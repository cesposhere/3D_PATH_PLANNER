#pragma once
#include <vector>
#include <cmath>
#include "core_data.h"

// 1. Our Custom, High-Performance Priority Queue
class MinHeap {
    std::vector<Node*> heap;

    void swap(std::size_t i, std::size_t j) {
        Node* temp = heap[i];
        heap[i] = heap[j];
        heap[j] = temp;
        // The nodes memorize their own position in the heap!
        heap[i]->heap_index = i;
        heap[j]->heap_index = j;
    }

    void sift_up(std::size_t idx) {
        while (idx > 0) {
            std::size_t parent = (idx - 1) / 2;
            if (heap[idx]->f < heap[parent]->f) {
                swap(idx, parent);
                idx = parent;
            } else break;
        }
    }

    void sift_down(std::size_t idx) {
        std::size_t size = heap.size();
        while (true) {
            std::size_t left = 2 * idx + 1;
            std::size_t right = 2 * idx + 2;
            std::size_t smallest = idx;

            if (left < size && heap[left]->f < heap[smallest]->f) smallest = left;
            if (right < size && heap[right]->f < heap[smallest]->f) smallest = right;

            if (smallest != idx) {
                swap(idx, smallest);
                idx = smallest;
            } else break;
        }
    }

public:
    bool is_empty() const { return heap.empty(); }

    void push(Node* node) {
        heap.push_back(node);
        node->heap_index = heap.size() - 1;
        node->in_open = true;
        sift_up(node->heap_index);
    }

    Node* pop() {
        if (is_empty()) return nullptr;
        Node* min_node = heap[0];
        min_node->in_open = false;
        
        swap(0, heap.size() - 1);
        heap.pop_back();
        min_node->heap_index = Node::invalid_heap_index;
        
        if (!is_empty()) sift_down(0);
        return min_node;
    }

    void update(Node* node) {
        if (node->heap_index != Node::invalid_heap_index && node->heap_index < heap.size()) {
            sift_up(node->heap_index);
        }
    }
    
    void clear() { heap.clear(); }
};


// Create our three modes
enum class AlgoType { ASTAR, DIJKSTRA, GREEDY };

// 2. The Core 3D A* Engine
class Pathfinder3D {
    std::vector<Node>& grid;
    int size;
    MinHeap open_set;
    Node* target_node = nullptr;

public:
    bool is_searching = false;
    bool path_found = false;
    std::vector<Node*> final_path;
    AlgoType current_algo = AlgoType::ASTAR; // Track which algorithm is running

    Pathfinder3D(std::vector<Node>& g, int s) : grid(g), size(s) {}

    Node* get_node(int x, int y, int z) {
        if (x < 0 || x >= size || y < 0 || y >= size || z < 0 || z >= size) return nullptr;
        return &grid[x + (y * size) + (z * size * size)];
    }

    float heuristic(Node* a, Node* b) {
        int dx = a->x - b->x;
        int dy = a->y - b->y;
        int dz = a->z - b->z;
        return std::sqrt(static_cast<float>(dx*dx + dy*dy + dz*dz));
    }

    void reset_search() {
        is_searching = false;
        path_found = false;
        final_path.clear();
        open_set.clear();
        for (auto& node : grid) {
            node.g = INFINITY; node.h = 0.0f; node.f = INFINITY;
            node.parent = nullptr; node.in_open = false; node.in_closed = false;
            node.heap_index = Node::invalid_heap_index;
        }
    }

    // Now accepts the algorithm type!
    void start_search(int arr[6], AlgoType algo) {
        int sx = arr[0], sy = arr[1], sz = arr[2];
        int tx = arr[3], ty = arr[4], tz = arr[5];
        // Reset the grid memory completely before a new search
        for (auto& node : grid) {
            node.g = INFINITY; node.h = 0.0f; node.f = INFINITY;
            node.parent = nullptr; node.in_open = false; node.in_closed = false;
            node.heap_index = Node::invalid_heap_index;
        }
        
        open_set.clear();
        final_path.clear();
        is_searching = true;
        path_found = false;
        current_algo = algo;

        Node* start = get_node(sx, sy, sz);
        target_node = get_node(tx, ty, tz);

        start->g = 0;
        start->h = heuristic(start, target_node);
        
        // Initial node setup based on the algorithm
        if (current_algo == AlgoType::DIJKSTRA) start->f = start->g;
        else if (current_algo == AlgoType::GREEDY) start->f = start->h;
        else start->f = start->g + start->h; 
        
        open_set.push(start);
    }

    void step() {
        if (!is_searching || path_found) return;
        // Process 40 nodes per frame so we can watch it fly
        for(int i = 0; i < 40; i++) {

            // If the open set is empty but we haven't found the target, it's trapped.
            if (open_set.is_empty()) {
                is_searching = false;
                std::cout << "\n======================================" << std::endl;
                std::cout << "   PATH NOT FOUND! TARGET UNREACHABLE " << std::endl;
                std::cout << "======================================\n" << std::endl;
                break;
            }
            if (open_set.is_empty()) break;

            Node* current = open_set.pop();
            current->in_closed = true;

            if (current == target_node) {
                while (current != nullptr) {
                    final_path.push_back(current);
                    current = current->parent;
                }
                path_found = true;
                is_searching = false;
                return;
            }

            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dz = -1; dz <= 1; dz++) {
                        if (dx == 0 && dy == 0 && dz == 0) continue;

                        Node* neighbor = get_node(current->x + dx, current->y + dy, current->z + dz);
                        if (!neighbor || neighbor->is_obstacle || neighbor->in_closed) continue;

                        float move_cost = std::sqrt(static_cast<float>(dx*dx + dy*dy + dz*dz));
                        float tentative_g = current->g + move_cost;

                        if (tentative_g < neighbor->g) {
                            neighbor->parent = current;
                            neighbor->g = tentative_g;
                            neighbor->h = heuristic(neighbor, target_node);
                            
                            // THE CORE LOGIC SPLIT:
                            if (current_algo == AlgoType::DIJKSTRA) {
                                neighbor->f = neighbor->g; // Only care about distance from start
                            } 
                            else if (current_algo == AlgoType::GREEDY) {
                                neighbor->f = neighbor->h; // Only care about distance to target
                            } 
                            else {
                                neighbor->f = neighbor->g + neighbor->h; // A* cares about both
                            }

                            if (!neighbor->in_open) {
                                open_set.push(neighbor);
                            } else {
                                open_set.update(neighbor); 
                            }
                        }
                    }
                }
            }
        }
    }
};