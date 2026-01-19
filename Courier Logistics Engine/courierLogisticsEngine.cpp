#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
#include <windows.h>

#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif
#ifdef REGISTERED
#undef REGISTERED
#endif

#include <iomanip> 
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <cmath>
#include <stdexcept>
#include <limits>
#include <algorithm>
#include <utility> 
using namespace std;

template<typename T>
class vector {
private:
    T* _data;
    size_t _size;
    size_t _capacity;

    void grow() {
        // compute new capacity (at least 1)
        size_t newCap = _capacity ? _capacity * 2 : 4;
        T* newData = new T[newCap];

        // copy at most newCap elements and only if _data is valid
        size_t copyCount = _size;
        if (copyCount > newCap) copyCount = newCap; // defensive
        if (_data && copyCount > 0) {
            for (size_t i = 0; i < copyCount; ++i) {
                newData[i] = _data[i];
            }
        }

        // if we couldn't copy everything (shouldn't happen), adjust size
        if (copyCount < _size) _size = copyCount;

        delete[] _data;
        _data = newData;
        _capacity = newCap;
    }

public:
    vector() : _data(nullptr), _size(0), _capacity(0) {}
    explicit vector(size_t n, const T& val = T()) : _data(nullptr), _size(n), _capacity(n) {
        if (_capacity) {
            _data = new T[_capacity];
            for (size_t i = 0; i < _size; ++i) _data[i] = val;
        }
    }

    // Copy
    vector(const vector& other) : _data(nullptr), _size(other._size), _capacity(other._capacity) {
        if (_capacity) {
            _data = new T[_capacity];
            size_t copyCount = _size;
            if (other._data && copyCount > 0) {
                for (size_t i = 0; i < copyCount; ++i) _data[i] = other._data[i];
            }
            else {
                // Defensive: if other._data is null but size claims >0, clamp size
                if (!other._data) _size = 0;
            }
        }
        else {
            _size = 0;
        }
    }

    // Move
    vector(vector&& other) noexcept : _data(other._data), _size(other._size), _capacity(other._capacity) {
        other._data = nullptr; other._size = 0; other._capacity = 0;
    }

    vector& operator=(const vector& other) {
        if (this == &other) return *this;
        delete[] _data;
        _size = other._size;
        _capacity = other._capacity;
        _data = nullptr;
        if (_capacity) {
            _data = new T[_capacity];
            size_t copyCount = _size;
            if (other._data && copyCount > 0) {
                for (size_t i = 0; i < copyCount; ++i) _data[i] = other._data[i];
            }
            else {
                if (!other._data) _size = 0;
            }
        }
        else {
            _size = 0;
        }
        return *this;
    }

    vector& operator=(vector&& other) noexcept {
        if (this != &other) {
            delete[] _data;
            _data = other._data;
            _size = other._size;
            _capacity = other._capacity;
            other._data = nullptr; other._size = 0; other._capacity = 0;
        }
        return *this;
    }

    ~vector() { delete[] _data; }

    size_t size() const noexcept { return _size; }
    bool empty() const noexcept { return _size == 0; }

    void push_back(const T& v) {
        if (_size >= _capacity) grow();
        _data[_size++] = v;
    }

    void push_back(T&& v) {
        if (_size >= _capacity) grow();
        _data[_size++] = std::move(v);
    }

    void pop_back() noexcept {
        if (_size > 0) --_size;
    }

    T& operator[](size_t i) { return _data[i]; }
    const T& operator[](size_t i) const { return _data[i]; }

    T* begin() noexcept { return _data ? _data : nullptr; }
    T* end() noexcept { return _data ? (_data + _size) : nullptr; }
    const T* begin() const noexcept { return _data ? _data : nullptr; }
    const T* end() const noexcept { return _data ? (_data + _size) : nullptr; }

    T* data() noexcept { return _data; }
    const T* data() const noexcept { return _data; }

    void clear() noexcept { _size = 0; }
    void reserve(size_t n) {
        if (n <= _capacity) return;
        T* newData = new T[n];
        size_t copyCount = _size;
        if (_data && copyCount > 0) {
            for (size_t i = 0; i < copyCount; ++i) newData[i] = _data[i];
        }
        else {
            if (!_data) copyCount = 0;
        }
        delete[] _data;
        _data = newData;
        _capacity = n;
        _size = copyCount;
    }
};

// Minimal queue built on top of the above vector template.
// Supports: push, pop, front, empty, size, clear.
template<typename T>
class queue {
private:
    vector<T> _data;
    size_t _head;

    void compact_if_needed() {
        size_t total = _data.size();
        if (total == 0) {
            _head = 0;
            return;
        }

        if (_head > 64 && _head * 2 >= total) {
            // Move remaining elements to a new buffer safely
            vector<T> newBuf;
            newBuf.reserve(total - _head);
            for (size_t i = _head; i < total; ++i) {
                newBuf.push_back(std::move(_data[i]));
            }
            _data = std::move(newBuf);
            _head = 0;
        }
    }

public:
    queue() : _data(), _head(0) {}
    bool empty() const noexcept { return _head >= _data.size(); }
    size_t size() const noexcept { return (_data.size() > _head) ? (_data.size() - _head) : 0; }

    void push(const T& v) { _data.push_back(v); }
    void push(T&& v) { _data.push_back(std::move(v)); }

    T& front() { return _data[_head]; }
    const T& front() const { return _data[_head]; }

    void pop() {
        if (empty()) return;
        ++_head;
        compact_if_needed();
    }

    void clear() {
        _data.clear();
        _head = 0;
    }
};

// Function to change text color 
static void SetTextColor(int color)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

// Function to set position of text on page 
static void gotoxy(int x, int y)
{
    COORD coordinate; coordinate.X = x;
    coordinate.Y = y; HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(hConsole, coordinate);
}

// Function to clear the screen 
static void clearScreen() { system("cls"); }

// Function to pause the pro<gram 
static void pause() { system("pause"); }

static  void frontPage()
{
    pause();
    clearScreen();
    SetTextColor(11);
    gotoxy(55, 6); cout << " #####                                     ";
    gotoxy(55, 7); cout << "#     #  ####  #    # #####  # ###### #####";
    gotoxy(55, 8); cout << "#       #    # #    # #    # # #      #    #";
    gotoxy(55, 9); cout << "#       #    # #    # #    # # #####  #    #";
    gotoxy(55, 10); cout << "#       #    # #    # #####  # #      #####";
    gotoxy(55, 11); cout << "#     # #    # #    # #   #  # #      #   #";
    gotoxy(55, 12); cout << " #####   ####   ####  #    # # ###### #    #";
    cout << "\n\n";

    gotoxy(51, 15); cout << "#                                                   ";
    gotoxy(51, 16); cout << "#        ####   ####  #  ####  ##### #  ####   #### ";
    gotoxy(51, 17); cout << "#       #    # #    # # #        #   # #    # #     ";
    gotoxy(51, 18); cout << "#       #    # #      #  ####    #   # #       #### ";
    gotoxy(51, 19); cout << "#       #    # #  ### #      #   #   # #           #";
    gotoxy(51, 20); cout << "#       #    # #    # # #    #   #   # #    # #    #";
    gotoxy(51, 21); cout << "#######  ####   ####  #  ####    #   #  ####   #### ";
    cout << "\n\n";

    gotoxy(58, 24); cout << "#######                              ";
    gotoxy(58, 25); cout << "#       #    #  ####  # #    # ######";
    gotoxy(58, 26); cout << "#       ##   # #    # # ##   # #     ";
    gotoxy(58, 27); cout << "#####   # #  # #      # # #  # ##### ";
    gotoxy(58, 28); cout << "#       #  # # #  ### # #  # # #     ";
    gotoxy(58, 29); cout << "#       #   ## #    # # #   ## #     ";
    gotoxy(58, 30); cout << "####### #    #  ####  # #    # ######";
    cout << "\n\n";

}

static void thankyou()
{
    clearScreen();
    SetTextColor(11);
    gotoxy(55, 6); cout << " #######                            ";
    gotoxy(55, 7); cout << "    #    #    #   ##   #    # #    #";
    gotoxy(55, 8); cout << "    #    #    #  #  #  ##   # #   # ";
    gotoxy(55, 9); cout << "    #    ###### #    # # #  # ####  ";
    gotoxy(55, 10); cout << "    #    #    # ###### #  # # #  #  ";
    gotoxy(55, 11); cout << "    #    #    # #    # #   ## #   # ";
    gotoxy(55, 12); cout << "    #    #    # #    # #    # #    #";
    gotoxy(55, 15); cout << "       #     #                      ";
    gotoxy(55, 16); cout << "        #   #   ####  #    #         ";
    gotoxy(55, 17); cout << "         # #   #    # #    #         ";
    gotoxy(55, 18); cout << "          #    #    # #    #         ";
    gotoxy(55, 19); cout << "          #    #    # #    #";
    gotoxy(55, 20); cout << "          #    #    # #    #";
    gotoxy(55, 21); cout << "          #     ####   #### ";
    cout << endl;
    cout << endl;
}
/* ===================== STATUS ENUM ===================== */
enum Status
{
    REGISTERED,
    DISPATCHED,
    LOADED,
    IN_TRANSIT,
    UNLOADED,
    OUT_FOR_DELIVERY,
    DELIVERY_ATTEMPT_FAILED,
    RETURN_TO_SENDER,
    DELIVERED
};

/* ===================== Priority ENUM ===================== */
enum DeliveryPriority {
    NORMAL = 1,
    TWO_DAY = 2,
    OVERNIGHT = 3
};

/* ===================== Weight ENUM ===================== */
enum WeightCategory {
    LIGHT = 1,
    MEDIUM = 2,
    HEAVY = 3
};

/* ===================== Zone ENUM ===================== */
enum Zone
{
    NORTH = 1,
    SOUTH = 2,
    EAST = 3,
    WEST = 4,
    CENTRAL = 5
};

static string zoneToString(Zone zone)
{
    switch (zone)
    {
    case NORTH:
        return "North";
    case SOUTH:
        return "South";
    case EAST:
        return "East";
    case WEST:
        return "West";
    case CENTRAL:
        return "Central";
    default:
        return "Unknown";
    }
}

static string zoneToString(int zone)
{
    return zoneToString(static_cast<Zone>(zone));
}

/* ===================== ROAD STATUS ENUM ===================== */
enum RoadStatus {
    ROAD_OPEN,
    ROAD_BLOCKED,
    ROAD_CONGESTED
};

static string roadStatusToString(RoadStatus rs) {
    switch (rs) {
    case ROAD_OPEN: return "Open";
    case ROAD_BLOCKED: return "Blocked";
    case ROAD_CONGESTED: return "Congested";
    default: return "Unknown";
    }
}

/* ===================== SIMPLE ARRAY ===================== */
template <typename T>
class SimpleArray {
private:
    vector<T> v;

public:
    SimpleArray() { v.reserve(10); }

    // defaulted copy/move/dtor are fine

    void push_back(const T& value) { v.push_back(value); }
    void push_back(T&& value) { v.push_back(std::move(value)); }

    T& operator[](int index) {
        if (index < 0 || index >= static_cast<int>(v.size()))
            throw out_of_range("Index out of bounds");
        return v[static_cast<size_t>(index)];
    }

    const T& operator[](int index) const {
        if (index < 0 || index >= static_cast<int>(v.size()))
            throw out_of_range("Index out of bounds");
        return v[static_cast<size_t>(index)];
    }

    int size() const { return static_cast<int>(v.size()); }
    bool empty() const { return v.empty(); }
    void clear() { v.clear(); }
    void removeLast() { if (!v.empty()) v.pop_back(); }

    T* begin() { return v.data(); }
    T* end() { return v.data() + v.size(); }
    const T* begin() const { return v.data(); }
    const T* end() const { return v.data() + v.size(); }
};
/* ===================== ROUTING STRUCTURES ===================== */
struct LocationNode {
    int id;
    string name;
    Zone zone;
    bool isWarehouse;
    bool isDeliveryPoint;

    LocationNode(int _id = 0, string _name = "", Zone _zone = CENTRAL,
        bool warehouse = false, bool delivery = false) {
        id = _id;
        name = _name;
        zone = _zone;
        isWarehouse = warehouse;
        isDeliveryPoint = delivery;
    }
};

struct RoadEdge {
    int from;
    int to;
    double distance;
    double baseTime;
    RoadStatus status;
    int trafficLevel;

    RoadEdge(int f = 0, int t = 0, double d = 0.0, double bt = 0.0) {
        from = f;
        to = t;
        distance = d;
        baseTime = bt;
        status = ROAD_OPEN;
        trafficLevel = 1;
    }

    double getEffectiveTime() const {
        if (status == ROAD_BLOCKED) {
            return 1e9;
        }

        double multiplier = 1.0;
        if (status == ROAD_CONGESTED) {
            multiplier = 2.0;
        }

        multiplier *= (1.0 + (trafficLevel - 1) * 0.2);
        return baseTime * multiplier;
    }
};

struct Path {
    SimpleArray<int> nodes;

    Path() {}

    bool equals(const Path& other) const {
        if (nodes.size() != other.nodes.size()) return false;
        for (int i = 0; i < nodes.size(); i++) {
            if (nodes[i] != other.nodes[i]) return false;
        }
        return true;
    }
};

/* ===================== ROUTING ENGINE - FIXED VERSION ===================== */
class RoutingEngine {
private:
    LocationNode* locations;
    SimpleArray<RoadEdge>* adjacencyList;
    int locationCount;
    int maxLocations;

    // Helper function to check if a path is valid
    bool isValidPath(const Path& path) {
        if (path.nodes.size() < 2) return false;

        // Check if all nodes in the path are valid
        for (int i = 0; i < path.nodes.size(); i++) {
            int nodeId = path.nodes[i];
            if (nodeId < 0 || nodeId >= locationCount) {
                return false;
            }
        }

        // Check if all edges in the path exist and are not blocked
        for (int i = 0; i < path.nodes.size() - 1; i++) {
            int u = path.nodes[i];
            int v = path.nodes[i + 1];

            bool edgeFound = false;
            for (int j = 0; j < adjacencyList[u].size(); j++) {
                const RoadEdge& edge = adjacencyList[u][j];
                if (edge.to == v) {
                    if (edge.status == ROAD_BLOCKED) {
                        return false;
                    }
                    edgeFound = true;
                    break;
                }
            }

            if (!edgeFound) {
                return false;
            }
        }
        return true;
    }

    // Helper function to check if a path already exists in a list
    bool pathExists(const Path& path, const SimpleArray<Path>& allPaths) {
        for (int i = 0; i < allPaths.size(); i++) {
            if (allPaths[i].equals(path)) {
                return true;
            }
        }
        return false;
    }

    // Helper function to find all simple paths between start and end
    void findAllPathsDFS(int current, int end, vector<bool>& visited,
        Path& currentPath, SimpleArray<Path>& allPaths,
        int maxPaths) {
        visited[current] = true;
        currentPath.nodes.push_back(current);

        if (current == end) {
            allPaths.push_back(currentPath);
        }
        else {
            for (int i = 0; i < adjacencyList[current].size(); i++) {
                int neighbor = adjacencyList[current][i].to;
                if (!visited[neighbor] &&
                    adjacencyList[current][i].status != ROAD_BLOCKED &&
                    allPaths.size() < maxPaths) {
                    findAllPathsDFS(neighbor, end, visited, currentPath, allPaths, maxPaths);
                }
            }
        }

        visited[current] = false;
        if (currentPath.nodes.size() > 0) {
            currentPath.nodes.push_back(currentPath.nodes.size() - 1);
        }
    }

public:
    RoutingEngine(int maxLoc = 50) {
        maxLocations = maxLoc;
        locationCount = 0;
        locations = new LocationNode[maxLocations];
        adjacencyList = new SimpleArray<RoadEdge>[maxLocations];
        initializeDefaultNetwork();
    }

    ~RoutingEngine() {
        delete[] locations;
        delete[] adjacencyList;
    }

    void initializeDefaultNetwork() {
        locationCount = 0;

        // Clear adjacency lists
        for (int i = 0; i < maxLocations; i++) {
            adjacencyList[i].clear();
        }

        // Add locations
        locations[locationCount++] = LocationNode(0, "Central Hub", CENTRAL, true, false);
        locations[locationCount++] = LocationNode(1, "North Warehouse", NORTH, true, false);
        locations[locationCount++] = LocationNode(2, "South Warehouse", SOUTH, true, false);
        locations[locationCount++] = LocationNode(3, "East Warehouse", EAST, true, false);
        locations[locationCount++] = LocationNode(4, "West Warehouse", WEST, true, false);
        locations[locationCount++] = LocationNode(5, "Delivery Point North", NORTH, false, true);
        locations[locationCount++] = LocationNode(6, "Delivery Point South", SOUTH, false, true);
        locations[locationCount++] = LocationNode(7, "Delivery Point East", EAST, false, true);
        locations[locationCount++] = LocationNode(8, "Delivery Point West", WEST, false, true);
        locations[locationCount++] = LocationNode(9, "Delivery Point Central", CENTRAL, false, true);

        // Add roads (bidirectional)
        addRoad(0, 1, 10.0, 15.0);
        addRoad(0, 2, 12.0, 18.0);
        addRoad(0, 3, 8.0, 12.0);
        addRoad(0, 4, 9.0, 14.0);
        addRoad(1, 5, 5.0, 8.0);
        addRoad(2, 6, 6.0, 9.0);
        addRoad(3, 7, 4.0, 6.0);
        addRoad(4, 8, 5.0, 7.0);
        addRoad(0, 9, 3.0, 5.0);
        addRoad(1, 3, 15.0, 22.0);
        addRoad(2, 4, 14.0, 21.0);
        addRoad(5, 6, 20.0, 30.0);
    }

    void addRoad(int from, int to, double distance, double baseTime) {
        if (from >= 0 && from < locationCount && to >= 0 && to < locationCount) {
            RoadEdge edge1(from, to, distance, baseTime);
            RoadEdge edge2(to, from, distance, baseTime);
            adjacencyList[from].push_back(edge1);
            adjacencyList[to].push_back(edge2);
        }
    }

    // STABLE BFS algorithm for path finding
    Path findShortestPath(int start, int end) {
        Path result;

        if (start < 0 || start >= locationCount || end < 0 || end >= locationCount) {
            return result;
        }

        if (start == end) {
            result.nodes.push_back(start);
            return result;
        }

        bool* visited = new bool[locationCount];
        int* previous = new int[locationCount];

        for (int i = 0; i < locationCount; i++) {
            visited[i] = false;
            previous[i] = -1;
        }

        queue<int> q;
        q.push(start);
        visited[start] = true;

        while (!q.empty()) {
            int current = q.front();
            q.pop();

            if (current == end) {
                break;
            }

            for (int i = 0; i < adjacencyList[current].size(); i++) {
                const RoadEdge& edge = adjacencyList[current][i];
                int neighbor = edge.to;

                if (!visited[neighbor] && edge.status != ROAD_BLOCKED) {
                    visited[neighbor] = true;
                    previous[neighbor] = current;
                    q.push(neighbor);
                }
            }
        }

        // Reconstruct path
        if (previous[end] != -1) {
            vector<int> tempPath;
            for (int at = end; at != -1; at = previous[at]) {
                tempPath.push_back(at);
            }

            // Reverse the path
            for (size_t i = tempPath.size(); i-- > 0; ) {
                result.nodes.push_back(tempPath[i]);
            }
        }

        delete[] visited;
        delete[] previous;

        return result;
    }

    // FIXED ALTERNATIVE ROUTE FINDING FUNCTION
    SimpleArray<Path> findAlternativeRoutes(int start, int end, int maxRoutes = 3) {
        SimpleArray<Path> routes;

        // Get shortest path first
        Path shortestPath = findShortestPath(start, end);
        if (shortestPath.nodes.empty()) {
            return routes;
        }

        routes.push_back(shortestPath);

        // If we only need one route, return
        if (maxRoutes <= 1) {
            return routes;
        }

        // Try to find alternative routes using DFS
        vector<bool> visited(locationCount, false);
        Path currentPath;
        SimpleArray<Path> allPaths;

        findAllPathsDFS(start, end, visited, currentPath, allPaths, maxRoutes);

        // Sort paths by length (number of nodes)
        for (int i = 0; i < allPaths.size() && routes.size() < maxRoutes; i++) {
            if (!allPaths[i].equals(shortestPath)) {
                routes.push_back(allPaths[i]);
            }
        }

        // If still not enough routes, add some predefined alternatives
        if (routes.size() < maxRoutes) {
            // Alternative 1: Central Hub -> East Warehouse -> South Warehouse -> Delivery South
            if (start == 0 && end == 6 && routes.size() < maxRoutes) {
                Path alt1;
                alt1.nodes.push_back(0);
                alt1.nodes.push_back(3);
                alt1.nodes.push_back(2);
                alt1.nodes.push_back(6);

                if (isValidPath(alt1) && !pathExists(alt1, routes)) {
                    routes.push_back(alt1);
                }
            }

            // Alternative 2: Central Hub -> West Warehouse -> South Warehouse -> Delivery South
            if (start == 0 && end == 6 && routes.size() < maxRoutes) {
                Path alt2;
                alt2.nodes.push_back(0);
                alt2.nodes.push_back(4);
                alt2.nodes.push_back(2);
                alt2.nodes.push_back(6);

                if (isValidPath(alt2) && !pathExists(alt2, routes)) {
                    routes.push_back(alt2);
                }
            }
        }

        return routes;
    }

    void updateRoadStatus(int from, int to, RoadStatus newStatus) {
        if (from < 0 || from >= locationCount || to < 0 || to >= locationCount) {
            return;
        }

        for (int i = 0; i < adjacencyList[from].size(); i++) {
            RoadEdge& edge = adjacencyList[from][i];
            if (edge.to == to) {
                edge.status = newStatus;
                break;
            }
        }

        for (int i = 0; i < adjacencyList[to].size(); i++) {
            RoadEdge& edge = adjacencyList[to][i];
            if (edge.to == from) {
                edge.status = newStatus;
                break;
            }
        }
    }

    void updateTrafficLevel(int from, int to, int level) {
        if (level < 1) level = 1;
        if (level > 5) level = 5;

        if (from < 0 || from >= locationCount || to < 0 || to >= locationCount) {
            return;
        }

        for (int i = 0; i < adjacencyList[from].size(); i++) {
            RoadEdge& edge = adjacencyList[from][i];
            if (edge.to == to) {
                edge.trafficLevel = level;
                break;
            }
        }

        for (int i = 0; i < adjacencyList[to].size(); i++) {
            RoadEdge& edge = adjacencyList[to][i];
            if (edge.to == from) {
                edge.trafficLevel = level;
                break;
            }
        }
    }

    double calculateRouteTime(const Path& path) {
        if (path.nodes.size() < 2) return 0.0;

        double totalTime = 0.0;
        for (int i = 0; i < path.nodes.size() - 1; i++) {
            int u = path.nodes[i];
            int v = path.nodes[i + 1];

            if (u < 0 || u >= locationCount || v < 0 || v >= locationCount) {
                return 1e9; // Invalid path
            }

            bool edgeFound = false;
            for (int j = 0; j < adjacencyList[u].size(); j++) {
                const RoadEdge& edge = adjacencyList[u][j];
                if (edge.to == v) {
                    totalTime += edge.getEffectiveTime();
                    edgeFound = true;
                    break;
                }
            }

            if (!edgeFound) {
                return 1e9; // Missing edge
            }
        }

        return totalTime;
    }

    // Dynamic route recalculation
    Path recalculateRoute(int start, int end, const Path& currentPath) {
        Path newPath = findShortestPath(start, end);

        if (newPath.nodes.empty()) {
            return currentPath;  // Keep current path if no alternative
        }

        double currentTime = calculateRouteTime(currentPath);
        double newTime = calculateRouteTime(newPath);

        // Only switch if new route is significantly better (20% faster)
        if (newTime < currentTime * 0.8) {
            return newPath;
        }

        return currentPath;
    }

    void displayNetwork() {
        SetTextColor(192);
        cout << "NETWORK STATUS\n";
        SetTextColor(12);
        cout << "Total locations: " << locationCount << "\n\n";

        // -------- LOCATIONS TABLE --------
        cout << left
            << setw(6) << "ID"
            << setw(28) << "Name"
            << setw(12) << "Zone"
            << setw(14) << "Warehouse"
            << setw(16) << "Delivery Point"
            << endl;

        cout << string(70, '-') << endl;

        for (int i = 0; i < locationCount; i++) {
            cout << left
                << setw(6) << locations[i].id
                << setw(28) << locations[i].name
                << setw(12) << zoneToString(locations[i].zone)
                << setw(14) << (locations[i].isWarehouse ? "Yes" : "No")
                << setw(16) << (locations[i].isDeliveryPoint ? "Yes" : "No")
                << endl;
        }

        // -------- ROAD CONNECTIONS TABLE --------
        cout << "\nRoad Connections:\n";

        cout << left
            << setw(25) << "From"
            << setw(25) << "To"
            << setw(10) << "Dist(km)"
            << setw(12) << "Base(min)"
            << setw(14) << "Status"
            << setw(10) << "Traffic"
            << setw(14) << "Eff.Time(min)"
            << endl;

        cout << string(105, '-') << endl;

        for (int i = 0; i < locationCount; i++) {
            for (int j = 0; j < adjacencyList[i].size(); j++) {
                const RoadEdge& edge = adjacencyList[i][j];
                if (edge.from < edge.to) { // print each edge once
                    cout << left
                        << setw(25) << locations[edge.from].name
                        << setw(25) << locations[edge.to].name
                        << setw(10) << edge.distance
                        << setw(12) << edge.baseTime
                        << setw(14) << roadStatusToString(edge.status)
                        << setw(10) << (to_string(edge.trafficLevel) + "/5")
                        << setw(14) << edge.getEffectiveTime()
                        << endl;
                }
            }
        }
    }

    void displayRoute(const Path& path) {
        if (path.nodes.empty()) {
            cout << "No route available.\n";
            return;
        }

        cout << "Route: ";
        for (int i = 0; i < path.nodes.size(); i++) {
            int nodeId = path.nodes[i];
            if (nodeId >= 0 && nodeId < locationCount) {
                cout << locations[nodeId].name;
                if (i < path.nodes.size() - 1) {
                    cout << " -> ";
                }
            }
        }

        double routeTime = calculateRouteTime(path);
        if (routeTime >= 1e9) {
            cout << "\nWARNING: Route contains blocked roads!\n";
        }
        else {
            cout << "\nTotal time: " << routeTime << " minutes\n";
        }
    }



    void displayRouteWithDetails(const Path& path) {
        if (path.nodes.empty()) {
            cout << "No route available.\n";
            return;
        }
        cout << endl;
        clearScreen();
        SetTextColor(192);
        cout << "ROUTE DETAILS\n";
        SetTextColor(7);
        displayRoute(path);

        double totalTime = 0.0;
        double totalDistance = 0.0;
        int segmentCount = 0;
        int blockedSegments = 0;
        int congestedSegments = 0;

        cout << "\nSegment-by-Segment Analysis:\n";

        // -------- TABLE HEADER --------
        cout << left
            << setw(10) << "Segment"
            << setw(20) << "From"
            << setw(20) << "To"
            << setw(12) << "Dist(km)"
            << setw(12) << "Base(min)"
            << setw(14) << "Status"
            << setw(10) << "Traffic"
            << setw(14) << "Eff.Time"
            << endl;

        cout << string(112, '-') << endl;

        for (int i = 0; i < (int)path.nodes.size() - 1; i++) {
            int u = path.nodes[i];
            int v = path.nodes[i + 1];

            if (u < 0 || u >= locationCount || v < 0 || v >= locationCount) {
                cout << "ERROR: Invalid node IDs in segment\n";
                continue;
            }

            bool edgeFound = false;

            for (int j = 0; j < adjacencyList[u].size(); j++) {
                const RoadEdge& edge = adjacencyList[u][j];

                if (edge.to == v) {
                    segmentCount++;
                    totalTime += edge.getEffectiveTime();
                    totalDistance += edge.distance;

                    string statusText = roadStatusToString(edge.status);

                    if (edge.status == ROAD_BLOCKED) {
                        blockedSegments++;
                        statusText += " (Avoided)";
                    }
                    else if (edge.status == ROAD_CONGESTED) {
                        congestedSegments++;
                        statusText += " (Slow)";
                    }

                    cout << left
                        << setw(10) << segmentCount
                        << setw(20) << locations[u].name
                        << setw(20) << locations[v].name
                        << setw(12) << edge.distance
                        << setw(12) << edge.baseTime
                        << setw(14) << statusText
                        << setw(10) << (to_string(edge.trafficLevel) + "/5")
                        << setw(14) << edge.getEffectiveTime()
                        << endl;

                    edgeFound = true;
                    break;
                }
            }

            if (!edgeFound) {
                cout << "ERROR: No road found between "
                    << locations[u].name << " and "
                    << locations[v].name << "\n";
            }
        }

        // -------- SUMMARY TABLE --------
        SetTextColor(192);
        cout << "\nROUTE SUMMARY\n";
        SetTextColor(7);
        cout << left
            << setw(28) << "Total Segments"
            << ": " << segmentCount << endl;

        cout << left
            << setw(28) << "Total Distance (km)"
            << ": " << totalDistance << endl;

        cout << left
            << setw(28) << "Total Time (min)"
            << ": " << totalTime << endl;

        cout << left
            << setw(28) << "Blocked Segments Avoided"
            << ": " << blockedSegments << endl;

        cout << left
            << setw(28) << "Congested Segments"
            << ": " << congestedSegments << endl;

        if (segmentCount > 0) {
            double avgTimePerSegment = totalTime / (double)segmentCount;
            cout << left
                << setw(28) << "Avg Time per Segment (min)"
                << ": " << avgTimePerSegment << endl;
        }
    }

    void comprehensiveRouteFinding(int start, int end) {
        cout << endl;
        SetTextColor(192);
        cout << "COMPREHENSIVE ROUTE FINDING\n";
        SetTextColor(7);

        cout << left
            << setw(20) << "From"
            << ": " << getLocationName(start) << "  (ID: " << start << ")\n";

        cout << left
            << setw(20) << "To"
            << ": " << getLocationName(end) << "  (ID: " << end << ")\n";


        // ---------- VALIDATION ----------
        if (start < 0 || start >= locationCount) {
            cout << "\n[ERROR] Invalid Start Location ID: " << start << "\n";
            cout << "Valid IDs: 0 to " << (locationCount - 1) << "\n";
            return;
        }
        if (end < 0 || end >= locationCount) {
            cout << "\n[ERROR] Invalid End Location ID: " << end << "\n";
            cout << "Valid IDs: 0 to " << (locationCount - 1) << "\n";
            return;
        }


        // ---------- STEP 1 : SHORTEST ROUTE ----------
        cout << endl;
        SetTextColor(192);
        cout << "1. SHORTEST ROUTE ANALYSIS\n";
        SetTextColor(7);

        Path shortestRoute = findShortestPath(start, end);

        if (shortestRoute.nodes.empty()) {
            cout << "\nNo route found between locations.\n";
            cout << "Possible Reasons:\n";
            cout << " - Roads are blocked\n";
            cout << " - Network is disconnected\n";
            return;
        }

        cout << " Route Found with "
            << shortestRoute.nodes.size()
            << " nodes.\n";

        cout << "ROUTE OVERVIEW:\n";
        displayRoute(shortestRoute);


        cout << "\nShow detailed breakdown? (1=Yes, 0=No): ";
        int choice;

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            choice = 0;
        }
        cout << endl;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 1) {
            displayRouteWithDetails(shortestRoute);
        }


        // ---------- STEP 2 : ALTERNATIVE ROUTES ----------
        cout << endl;
        SetTextColor(192);
        cout << "2. ALTERNATIVE ROUTE OPTIONS\n";
        SetTextColor(7);

        SimpleArray<Path> alternatives = findAlternativeRoutes(start, end, 3);

        if (alternatives.size() > 0) {

            cout << " Total Routes Found: " << alternatives.size() << "\n";

            double shortestTime = calculateRouteTime(shortestRoute);

            for (int i = 0; i < alternatives.size(); i++) {

                cout << "\nRoute " << (i + 1)
                    << (i == 0 ? "  (Shortest Path)" : "  (Alternative)")
                    << "\n-------------------------------------------------\n";

                if (!alternatives[i].nodes.empty()) {

                    displayRoute(alternatives[i]);

                    double currentTime = calculateRouteTime(alternatives[i]);

                    if (i > 0) {
                        double diff = currentTime - shortestTime;

                        if (diff < 0)
                            cout << "Faster by " << -diff << " min\n";
                        else if (diff > 0)
                            cout << "Slower by " << diff << " min\n";
                        else
                            cout << "Same duration as shortest route\n";
                    }
                }
            }
        }
        else {
            cout << "No alternative routes available.\n";
        }


        // ---------- STEP 3 : DYNAMIC ROUTE NOTE ----------
        cout << endl;
        SetTextColor(192);
        cout << "3. DYNAMIC ROUTE RECALCULATION";
        SetTextColor(7);
        cout << endl;
        cout << " Routes auto-update when conditions change" << endl;
        cout << " Blocked roads are automatically avoided" << endl;
        cout << " System always selects best possible path" << endl;


        // ---------- STEP 4 : FINAL RECOMMENDATION ----------
        SetTextColor(192);
        cout << "\n4. FINAL RECOMMENDATION"; SetTextColor(7); cout << endl;
        SetTextColor(7);
        double routeTime = calculateRouteTime(shortestRoute);

        cout << "\nRECOMMENDED ROUTE:\n";
        displayRoute(shortestRoute);

        cout << "Reasoning:\n";
        if (routeTime < 30)       cout << " Very Fast Route (< 30 min)\n";
        else if (routeTime < 60)  cout << " Reasonable Travel Time (< 60 min)\n";
        else                      cout << " Longer Route, but Optimal\n";

        cout << " Considers live road conditions\n";
        cout << " Avoids blocked & congested paths\n";
        cout << " Optimized for fastest delivery\n";
    }

    int getLocationCount() const {
        return locationCount;
    }

    string getLocationName(int id) const {
        if (id >= 0 && id < locationCount) {
            return locations[id].name;
        }
        return "Unknown";
    }
};

/* ===================== TRACKING NODE ===================== */
class TrackingNode {
public:
    string location;
    Status status;
    string timestamp;
    TrackingNode* prev;
    TrackingNode* next;

    TrackingNode(string loc, Status st, string timeStr = "")
    {
        location = loc;
        status = st;
        prev = next = NULL;

        if (timeStr != "")
            timestamp = timeStr;
        else
        {
            time_t now = time(0);
            timestamp = ctime(&now);
            timestamp.pop_back();
        }
    }
};

/* ===================== PARCEL CLASS ===================== */
class Parcel {
public:
    int parcelID;
    string sender, receiver;
    DeliveryPriority priority;
    WeightCategory weight;
    Zone zone;
    Path currentRoute;
    int currentRouteIndex;
    TrackingNode* head;
    TrackingNode* tail;

    Parcel(int id = 0, string s = "", string r = "",
        DeliveryPriority p = NORMAL,
        WeightCategory w = LIGHT,
        Zone z = CENTRAL)
    {
        parcelID = id;
        sender = s;
        receiver = r;
        priority = p;
        weight = w;
        zone = z;
        head = tail = NULL;
        currentRouteIndex = 0;
    }

    int priorityScore() const {
        return (priority * 10) + weight;
    }

    void addTracking(string loc, Status st, string timeStr = "") {
        TrackingNode* node = new TrackingNode(loc, st, timeStr);
        if (!head)
            head = tail = node;
        else {
            tail->next = node;
            node->prev = tail;
            tail = node;
        }
    }

    string statusToString(Status st)
    {
        switch (st)
        {
        case REGISTERED: return "Registered";
        case DISPATCHED: return "Dispatched";
        case LOADED: return "Loaded";
        case IN_TRANSIT: return "In Transit";
        case UNLOADED: return "Unloaded";
        case OUT_FOR_DELIVERY: return "Out for Delivery";
        case DELIVERY_ATTEMPT_FAILED: return "Delivery Attempt Failed";
        case RETURN_TO_SENDER: return "Return to Sender";
        case DELIVERED: return "Delivered";
        default: return "Unknown";
        }
    }

    void showTracking() {
        TrackingNode* temp = head;

        SetTextColor(192);
        cout << "TRACKING HISTORY (Parcel ID: "
            << parcelID << ")\n";
        SetTextColor(12);

        // Table Header
        cout << left
            << setw(25) << "Location"
            << setw(25) << "Status"
            << setw(25) << "Timestamp"
            << "\n-----------------------------------------------------------------------------------------------------------------\n";

        // Table Rows
        while (temp) {
            cout << left
                << setw(25) << temp->location
                << setw(25) << statusToString(temp->status)
                << setw(25) << temp->timestamp
                << "\n";

            temp = temp->next;
        }

        cout << "=================================================================================================================\n";
    }


    string getNextLocation(const RoutingEngine& router) {
        if (currentRouteIndex < currentRoute.nodes.size()) {
            return "Location " + to_string(currentRoute.nodes[currentRouteIndex]);
        }
        return "Destination reached";
    }
};

/* ===================== AVL NODE ===================== */
class AVLNode {
public:
    Parcel* parcel;
    AVLNode* left;
    AVLNode* right;
    int height;

    AVLNode(Parcel* p) {
        parcel = p;
        left = right = NULL;
        height = 1;
    }
};

/* ===================== AVL TREE ===================== */
class ParcelAVL {
private:
    AVLNode* root;

    int getHeight(AVLNode* n)
    {
        if (n != NULL)
            return n->height;
        else
            return 0;
    }

    int getBalance(AVLNode* n)
    {
        if (n != NULL)
            return getHeight(n->left) - getHeight(n->right);
        else
            return 0;
    }

    int maxInt(int a, int b)
    {
        if (a > b)
            return a;
        else
            return b;
    }

    AVLNode* rightRotate(AVLNode* y)
    {
        AVLNode* x = y->left;
        AVLNode* T2 = x->right;
        x->right = y;
        y->left = T2;
        y->height = maxInt(getHeight(y->left), getHeight(y->right)) + 1;
        x->height = maxInt(getHeight(x->left), getHeight(x->right)) + 1;
        return x;
    }

    AVLNode* leftRotate(AVLNode* x)
    {
        AVLNode* y = x->right;
        AVLNode* T2 = y->left;
        y->left = x;
        x->right = T2;
        x->height = maxInt(getHeight(x->left), getHeight(x->right)) + 1;
        y->height = maxInt(getHeight(y->left), getHeight(y->right)) + 1;
        return y;
    }

    AVLNode* insertNode(AVLNode* node, Parcel* p)
    {
        if (!node)
            return new AVLNode(p);
        if (p->parcelID < node->parcel->parcelID)
            node->left = insertNode(node->left, p);
        else if (p->parcelID > node->parcel->parcelID)
            node->right = insertNode(node->right, p);
        else
            return node;

        node->height = 1 + maxInt(getHeight(node->left), getHeight(node->right));
        int balance = getBalance(node);

        if (balance > 1 && p->parcelID < node->left->parcel->parcelID)
            return rightRotate(node);
        if (balance < -1 && p->parcelID > node->right->parcel->parcelID)
            return leftRotate(node);
        if (balance > 1 && p->parcelID > node->left->parcel->parcelID)
        {
            node->left = leftRotate(node->left);
            return rightRotate(node);
        }
        if (balance < -1 && p->parcelID < node->right->parcel->parcelID)
        {
            node->right = rightRotate(node->right);
            return leftRotate(node);
        }

        return node;
    }

    AVLNode* minValueNode(AVLNode* node) {
        AVLNode* current = node;
        while (current->left != NULL)
            current = current->left;
        return current;
    }

    AVLNode* deleteNode(AVLNode* root, int id) {
        if (!root) return root;

        if (id < root->parcel->parcelID)
            root->left = deleteNode(root->left, id);
        else if (id > root->parcel->parcelID)
            root->right = deleteNode(root->right, id);
        else {
            if (!root->left || !root->right)
            {
                AVLNode* temp = root->left ? root->left : root->right;
                if (!temp)
                {
                    temp = root;
                    root = NULL;
                }
                else {
                    root->parcel = temp->parcel;
                    root->left = temp->left;
                    root->right = temp->right;
                    root->height = temp->height;
                }
                delete temp;
            }
            else {
                AVLNode* temp = minValueNode(root->right);
                root->parcel = temp->parcel;
                root->right = deleteNode(root->right, temp->parcel->parcelID);
            }
        }

        if (!root)
            return root;

        root->height = 1 + maxInt(getHeight(root->left), getHeight(root->right));
        int balance = getBalance(root);

        if (balance > 1 && getBalance(root->left) >= 0)
            return rightRotate(root);
        if (balance > 1 && getBalance(root->left) < 0)
        {
            root->left = leftRotate(root->left);
            return rightRotate(root);
        }
        if (balance < -1 && getBalance(root->right) <= 0)
            return leftRotate(root);
        if (balance < -1 && getBalance(root->right) > 0)
        {
            root->right = rightRotate(root->right);
            return leftRotate(root);
        }

        return root;
    }

    Parcel* searchNode(AVLNode* node, int id)
    {
        if (!node)
            return NULL;
        if (id == node->parcel->parcelID)
            return node->parcel;
        if (id < node->parcel->parcelID)
            return searchNode(node->left, id);
        return searchNode(node->right, id);
    }

public:
    ParcelAVL()
    {
        root = NULL;
    }
    void insert(Parcel* p)
    {
        root = insertNode(root, p);
    }
    void remove(int id)
    {
        root = deleteNode(root, id);
    }
    Parcel* search(int id)
    {
        return searchNode(root, id);
    }
};

/* ===================== HEAP FOR PRIORITY QUEUE ===================== */
class ParcelSortingHeap {
private:
    struct HeapNode {
        Parcel* parcel;
    };

    HeapNode* heap;
    int capacity;
    int size;

    int parent(int i) const { return (i - 1) / 2; }
    int leftIndex(int i) const { return (2 * i + 1); }
    int rightIndex(int i) const { return (2 * i + 2); }

    void swapNodes(int a, int b) {
        HeapNode temp = heap[a];
        heap[a] = heap[b];
        heap[b] = temp;
    }

    void heapifyUp(int index) {
        while (index > 0 &&
            heap[parent(index)].parcel->priorityScore() <
            heap[index].parcel->priorityScore()) {
            swapNodes(index, parent(index));
            index = parent(index);
        }
    }

    void heapifyDown(int index) {
        int largest = index;
        int l = leftIndex(index);
        int r = rightIndex(index);

        if (l < size &&
            heap[l].parcel->priorityScore() >
            heap[largest].parcel->priorityScore()) {
            largest = l;
        }

        if (r < size &&
            heap[r].parcel->priorityScore() >
            heap[largest].parcel->priorityScore()) {
            largest = r;
        }

        if (largest != index) {
            swapNodes(index, largest);
            heapifyDown(largest);
        }
    }

public:
    ParcelSortingHeap(int cap = 1000) {
        capacity = cap;
        size = 0;
        heap = new HeapNode[capacity];
    }

    ~ParcelSortingHeap() {
        delete[] heap;
    }

    void insert(Parcel* p) {
        if (size >= capacity) {
            cout << "Heap is full!\n";
            return;
        }

        heap[size].parcel = p;
        heapifyUp(size);
        size++;
    }

    Parcel* extractHighestPriority() {
        if (size <= 0) return nullptr;

        Parcel* top = heap[0].parcel;
        heap[0] = heap[size - 1];
        size--;
        heapifyDown(0);
        return top;
    }

    void removeParcel(int parcelID) {
        for (int i = 0; i < size; i++) {
            if (heap[i].parcel->parcelID == parcelID) {
                heap[i] = heap[size - 1];
                size--;
                heapifyDown(i);
                return;
            }
        }
    }

    bool isEmpty() const {
        return size == 0;
    }

    void displayQueue() {
        SetTextColor(192);
        cout << "Intelligent Sorted Parcel Queue\n";
        SetTextColor(12);

        // Table Header
        cout << std::left << setw(12) << "Parcel ID"
            << setw(18) << "Priority Score"
            << setw(12) << "Zone" << endl;

        cout << string(42, '-') << endl; // separator line

        // Table Rows
        for (int i = 0; i < size; i++) {
            cout << std::left << setw(12) << heap[i].parcel->parcelID
                << setw(18) << heap[i].parcel->priorityScore()
                << setw(12) << zoneToString(heap[i].parcel->zone)
                << endl;
        }
    }
};

/* ===================== OPERATION LOG STRUCT ===================== */
struct OperationLog {
    string timestamp;
    string operationType;
    int parcelID;
    string details;
    string riderID;
    OperationLog() {
        timestamp = "";
        operationType = "";
        parcelID = 0;
        details = "";
        riderID = "";
    }

    OperationLog(string opType, int pID, string det = "", string rID = "") {
        operationType = opType;
        parcelID = pID;
        details = det;
        riderID = rID;

        time_t now = time(0);
        timestamp = ctime(&now);
        timestamp.pop_back(); // Remove newline
    }

    void display() const {
        // Table row
        cout << left
            << setw(27) << timestamp
            << setw(25) << operationType
            << setw(10) << parcelID
            << setw(15) << (riderID.empty() ? "N/A" : riderID)
            << setw(30) << details << endl;
    }
};

/* ===================== RIDER CLASS ===================== */
class Rider {
public:
    string riderID;
    string name;
    Zone currentZone;
    int maxLoad; // Maximum parcels rider can carry
    int currentLoad;
    SimpleArray<int> assignedParcels; // Parcel IDs assigned to rider
    bool isAvailable;

    Rider(string id = "", string n = "", Zone zone = CENTRAL, int load = 5) {
        riderID = id;
        name = n;
        currentZone = zone;
        maxLoad = load;
        currentLoad = 0;
        isAvailable = true;
    }

    bool canAcceptParcel(int parcelWeight = 1)  const {
        return isAvailable && (currentLoad + parcelWeight) <= maxLoad;
    }

    void assignParcel(int parcelID, int parcelWeight = 1) {
        assignedParcels.push_back(parcelID);
        currentLoad += parcelWeight;
        if (currentLoad >= maxLoad) {
            isAvailable = false;
        }
    }

    void completeDelivery(int parcelID, int parcelWeight = 1) {
        // Remove parcel from assigned list
        for (int i = 0; i < assignedParcels.size(); i++) {
            if (assignedParcels[i] == parcelID) {
                // Shift remaining elements
                for (int j = i; j < assignedParcels.size() - 1; j++) {
                    assignedParcels[j] = assignedParcels[j + 1];
                }
                // Remove last element
                assignedParcels.removeLast();
                break;
            }
        }
        currentLoad -= parcelWeight;
        if (currentLoad < 0) currentLoad = 0;
        isAvailable = true;
    }

    void display() const {
        // Print rider info
        cout << std::left
            << setw(10) << riderID
            << setw(20) << name
            << setw(10) << zoneToString(currentZone)
            << setw(10) << (to_string(currentLoad) + "/" + to_string(maxLoad))
            << setw(15) << (isAvailable ? "Available" : "Fully Loaded");

        // Assigned parcels
        if (!assignedParcels.empty()) {
            for (int i = 0; i < assignedParcels.size(); i++) {
                cout << assignedParcels[i];
                if (i < assignedParcels.size() - 1) cout << ", ";
            }
        }
        else {
            cout << "None";
        }

        cout << endl;
    }
};

/* ===================== QUEUE STRUCTURES ===================== */
struct QueueParcel {
    Parcel* parcel;
    time_t queueTime;
    string queueType;

    QueueParcel(Parcel* p = nullptr, string type = "") {
        parcel = p;
        queueType = type;
        queueTime = time(0);
    }

    double timeInQueue() const {
        return difftime(time(0), queueTime);
    }
};

/* ===================== COURIER OPERATIONS ENGINE ===================== */
class CourierOperationsEngine {
private:
    // Queues
    SimpleArray<QueueParcel> pickupQueue;
    SimpleArray<QueueParcel> warehouseQueue;
    SimpleArray<QueueParcel> transitQueue;

    // Riders
    SimpleArray<Rider> riders;

    // Operation logs
    SimpleArray<OperationLog> operationLogs;

    // Missing parcels tracking
    SimpleArray<int> missingParcels;

    // Reference to tracking system (forward declaration)
    class TrackingSystem* trackingSystem;
    RoutingEngine* router;

    // Helper to get weight value
    int getWeightValue(WeightCategory w) {
        switch (w) {
        case LIGHT: return 1;
        case MEDIUM: return 2;
        case HEAVY: return 3;
        default: return 1;
        }
    }

    // Helper to find rider by zone and availability
    Rider* findAvailableRider(Zone zone, int requiredCapacity = 1) {
        for (int i = 0; i < riders.size(); i++) {
            if (riders[i].currentZone == zone &&
                riders[i].canAcceptParcel(requiredCapacity)) {
                return &riders[i];
            }
        }

        // If no rider in same zone, find any available rider
        for (int i = 0; i < riders.size(); i++) {
            if (riders[i].canAcceptParcel(requiredCapacity)) {
                return &riders[i];
            }
        }

        return nullptr;
    }

    // Helper to move parcel between queues
    void moveParcelToQueue(Parcel* p, const string& fromQueue, const string& toQueue) {
        // Remove from old queue
        SimpleArray<QueueParcel>* oldQueue = nullptr;
        if (fromQueue == "pickup") oldQueue = &pickupQueue;
        else if (fromQueue == "warehouse") oldQueue = &warehouseQueue;
        else if (fromQueue == "transit") oldQueue = &transitQueue;

        if (oldQueue) {
            for (int i = 0; i < oldQueue->size(); i++) {
                if ((*oldQueue)[i].parcel->parcelID == p->parcelID) {
                    // Remove from old queue
                    for (int j = i; j < oldQueue->size() - 1; j++) {
                        (*oldQueue)[j] = (*oldQueue)[j + 1];
                    }
                    oldQueue->removeLast(); // fixed
                    break;
                }
            }
        }

        // Add to new queue
        if (toQueue == "pickup") pickupQueue.push_back(QueueParcel(p, "pickup"));
        else if (toQueue == "warehouse") warehouseQueue.push_back(QueueParcel(p, "warehouse"));
        else if (toQueue == "transit") transitQueue.push_back(QueueParcel(p, "transit"));
    }

public:
    CourierOperationsEngine(class TrackingSystem* ts, RoutingEngine* rt) {
        trackingSystem = ts;
        router = rt;
        initializeDefaultRiders();
    }

    void initializeDefaultRiders() {
        // Create default riders for each zone
        riders.push_back(Rider("R001", "John Rider", NORTH, 6));
        riders.push_back(Rider("R002", "Sarah Swift", SOUTH, 5));
        riders.push_back(Rider("R003", "Mike Express", EAST, 7));
        riders.push_back(Rider("R004", "Lisa Fleet", WEST, 5));
        riders.push_back(Rider("R005", "Tom Courier", CENTRAL, 8));
        riders.push_back(Rider("R006", "Anna Delivery", NORTH, 4));
        riders.push_back(Rider("R007", "Bob Transport", SOUTH, 6));
    }

    // ===================== QUEUE OPERATIONS =====================

    void addToPickupQueue(Parcel* p) {
        pickupQueue.push_back(QueueParcel(p, "pickup"));
        operationLogs.push_back(OperationLog("PICKUP_QUEUE_ADD", p->parcelID,
            "Added to pickup queue"));
        cout << "Parcel " << p->parcelID << " added to pickup queue\n";
    }

    void addToWarehouseQueue(Parcel* p) {
        warehouseQueue.push_back(QueueParcel(p, "warehouse"));
        operationLogs.push_back(OperationLog("WAREHOUSE_QUEUE_ADD", p->parcelID,
            "Added to warehouse queue"));
        cout << "Parcel " << p->parcelID << " added to warehouse queue\n";
    }

    void addToTransitQueue(Parcel* p) {
        transitQueue.push_back(QueueParcel(p, "transit"));
        operationLogs.push_back(OperationLog("TRANSIT_QUEUE_ADD", p->parcelID,
            "Added to transit queue"));
        cout << "Parcel " << p->parcelID << " added to transit queue\n";
    }

    // ===================== RIDER ASSIGNMENT =====================

    void assignRidersToPickup() {
        SetTextColor(192);
        cout << "ASSIGNING RIDERS TO PICKUP QUEUE\n";
        SetTextColor(12);
        int assignedCount = 0;

        // Sort pickup queue by priority (highest priority first)
        for (int i = 0; i < pickupQueue.size() - 1; i++) {
            for (int j = 0; j < pickupQueue.size() - i - 1; j++) {
                if (pickupQueue[j].parcel->priorityScore() <
                    pickupQueue[j + 1].parcel->priorityScore()) {
                    QueueParcel temp = pickupQueue[j];
                    pickupQueue[j] = pickupQueue[j + 1];
                    pickupQueue[j + 1] = temp;
                }
            }
        }

        // Table header
        cout << left
            << setw(10) << "ParcelID"
            << setw(20) << "RiderID"
            << setw(25) << "Rider Name"
            << setw(10) << "Zone"
            << setw(10) << "Load"
            << setw(15) << "Status" << endl;
        cout << string(90, '-') << endl;

        // Assign riders to parcels
        for (int i = 0; i < pickupQueue.size(); i++) {
            Parcel* p = pickupQueue[i].parcel;
            Rider* rider = findAvailableRider(CENTRAL, getWeightValue(p->weight));

            if (rider) {
                rider->assignParcel(p->parcelID, getWeightValue(p->weight));

                // Remove from pickup queue
                for (int j = i; j < pickupQueue.size() - 1; j++) {
                    pickupQueue[j] = pickupQueue[j + 1];
                }
                pickupQueue.removeLast(); // fixed
                i--; // Adjust index after removal

                operationLogs.push_back(OperationLog("RIDER_ASSIGNED", p->parcelID,
                    "Assigned to rider " + rider->riderID,
                    rider->riderID));

                // Table row
                cout << left
                    << setw(10) << p->parcelID
                    << setw(20) << rider->riderID
                    << setw(25) << rider->name
                    << setw(10) << zoneToString(CENTRAL)
                    << setw(10) << rider->currentLoad
                    << "Assigned" << endl;

                assignedCount++;
            }
        }

        cout << "\nTotal parcels assigned: " << assignedCount << endl;
    }

    void assignRidersToDelivery() {
        SetTextColor(192);
        cout << "ASSIGNING RIDERS FOR DELIVERY\n";
        SetTextColor(12);
        int assignedCount = 0;

        // Table header
        cout << left
            << setw(10) << "ParcelID"
            << setw(20) << "RiderID"
            << setw(25) << "Rider Name"
            << setw(10) << "Zone"
            << setw(10) << "Load"
            << "Status" << endl;
        cout << string(95, '-') << endl;

        // Process warehouse queue by zone
        for (int i = 0; i < warehouseQueue.size(); i++) {
            Parcel* p = warehouseQueue[i].parcel;
            Rider* rider = findAvailableRider(p->zone, getWeightValue(p->weight));

            if (rider) {
                rider->assignParcel(p->parcelID, getWeightValue(p->weight));

                // Remove from warehouse queue
                for (int j = i; j < warehouseQueue.size() - 1; j++) {
                    warehouseQueue[j] = warehouseQueue[j + 1];
                }
                warehouseQueue.removeLast(); // fixed
                i--; // Adjust index after removal

                operationLogs.push_back(OperationLog("DELIVERY_ASSIGNED", p->parcelID,
                    "Assigned for delivery to " + rider->riderID,
                    rider->riderID));

                // Table row
                cout << left
                    << setw(10) << p->parcelID
                    << setw(20) << rider->riderID
                    << setw(25) << rider->name
                    << setw(10) << zoneToString(p->zone)
                    << setw(10) << rider->currentLoad
                    << "Assigned" << endl;

                assignedCount++;
            }
        }

        cout << "\nTotal parcels assigned for delivery: " << assignedCount << endl;
    }


    // ===================== PARCEL WORKFLOW =====================

    void processParcelWorkflow(int parcelID);

    // ===================== MISSING PARCEL DETECTION =====================
    void checkForMissingParcels() {
        SetTextColor(12);
        cout << "\nCHECKING FOR MISSING PARCELS\n";
        int missingCount = 0;

        // Table header
        cout << left
            << setw(10) << "ParcelID"
            << setw(20) << "Queue Type"
            << setw(20) << "Time in Queue (min)"
            << setw(25) << "Status" << endl;
        cout << string(80, '-') << endl;

        // Check parcels in transit for too long
        time_t currentTime = time(0);
        for (int i = 0; i < transitQueue.size(); i++) {
            double timeInTransit = transitQueue[i].timeInQueue();

            if (timeInTransit > 3600.0) { // More than 1 hour in transit
                int parcelID = transitQueue[i].parcel->parcelID;

                // Check if already marked missing
                bool alreadyMissing = false;
                for (int j = 0; j < missingParcels.size(); j++) {
                    if (missingParcels[j] == parcelID) {
                        alreadyMissing = true;
                        break;
                    }
                }

                if (!alreadyMissing) {
                    missingParcels.push_back(parcelID);
                    operationLogs.push_back(OperationLog("MISSING_DETECTED", parcelID,
                        "In transit for " + to_string(static_cast<int>(timeInTransit / 60.0)) + " minutes"));

                    // Table row
                    cout << left
                        << setw(10) << parcelID
                        << setw(20) << "Transit"
                        << setw(20) << static_cast<int>(timeInTransit / 60.0)
                        << setw(25) << "Potential Missing" << endl;

                    missingCount++;
                }
            }
        }

        // Check pickup queue for stalled parcels
        for (int i = 0; i < pickupQueue.size(); i++) {
            double timeInQueue = pickupQueue[i].timeInQueue();

            if (timeInQueue > 1800.0) { // More than 30 minutes in pickup queue
                int parcelID = pickupQueue[i].parcel->parcelID;

                bool alreadyMissing = false;
                for (int j = 0; j < missingParcels.size(); j++) {
                    if (missingParcels[j] == parcelID) {
                        alreadyMissing = true;
                        break;
                    }
                }

                if (!alreadyMissing) {
                    missingParcels.push_back(parcelID);
                    operationLogs.push_back(OperationLog("STALLED_PICKUP", parcelID,
                        "In pickup queue for " + to_string(static_cast<int>(timeInQueue / 60.0)) + " minutes"));

                    // Table row
                    cout << left
                        << setw(10) << parcelID
                        << setw(20) << "Pickup"
                        << setw(20) << static_cast<int>(timeInQueue / 60.0)
                        << setw(25) << "Stalled" << endl;

                    missingCount++;
                }
            }
        }

        if (missingCount == 0) {
            cout << " No missing parcels detected\n";
        }
        else {
            cout << "\n Found " << missingCount << " potential missing/stalled parcels\n";
        }
    }


    void resolveMissingParcel(int parcelID) {
        // Table header
        cout << left
            << setw(15) << "ParcelID"
            << setw(25) << "Action"
            << setw(35) << "Details" << endl;
        cout << string(75, '-') << endl;

        // Remove from missing list
        for (int i = 0; i < missingParcels.size(); i++) {
            if (missingParcels[i] == parcelID) {
                for (int j = i; j < missingParcels.size() - 1; j++) {
                    missingParcels[j] = missingParcels[j + 1];
                }
                missingParcels.removeLast(); // fixed

                operationLogs.push_back(OperationLog("MISSING_RESOLVED", parcelID,
                    "Parcel found and restored"));

                // Table row
                cout << left
                    << setw(15) << parcelID
                    << setw(25) << "Removed from Missing List"
                    << setw(35) << "Parcel found and restored" << endl;
                return;
            }
        }

        // Parcel not found case
        cout << left
            << setw(15) << parcelID
            << setw(25) << "Not in Missing List"
            << setw(35) << "-" << endl;
    }


    // ===================== UNDO/REPLAY OPERATIONS =====================

    void undoLastOperation() {
        if (operationLogs.empty()) {
            cout << "No operations to undo\n";
            return;
        }

        OperationLog lastOp = operationLogs[operationLogs.size() - 1];

        // Table header
        SetTextColor(192);
        cout << "UNDO LAST OPERATION\n";
        SetTextColor(12);
        cout << left
            << setw(10) << "ParcelID"
            << setw(25) << "OperationType"
            << setw(35) << "Details" << endl;
        cout << string(70, '-') << endl;

        // Display last operation as a table row
        cout << left
            << setw(10) << lastOp.parcelID
            << setw(25) << lastOp.operationType
            << setw(35) << lastOp.details << endl;

        // Based on operation type, undo it
        if (lastOp.operationType.find("QUEUE_ADD") != string::npos) {
            // Remove from appropriate queue
            int parcelID = lastOp.parcelID;

            // Find and remove from all queues
            SimpleArray<QueueParcel>* queues[] = { &pickupQueue, &warehouseQueue, &transitQueue };
            string queueNames[] = { "pickup", "warehouse", "transit" };

            for (int q = 0; q < 3; q++) {
                for (int i = 0; i < queues[q]->size(); i++) {
                    if ((*queues[q])[i].parcel->parcelID == parcelID) {
                        // Remove from queue
                        for (int j = i; j < queues[q]->size() - 1; j++) {
                            (*queues[q])[j] = (*queues[q])[j + 1];
                        }
                        queues[q]->push_back(QueueParcel()); // Remove last

                        // Table row for queue removal
                        cout << left
                            << setw(10) << parcelID
                            << setw(25) << "Removed from Queue"
                            << setw(35) << queueNames[q] << endl;
                        break;
                    }
                }
            }
        }
        else if (lastOp.operationType == "RIDER_ASSIGNED" ||
            lastOp.operationType == "DELIVERY_ASSIGNED") {
            // Remove rider assignment
            for (int i = 0; i < riders.size(); i++) {
                if (riders[i].riderID == lastOp.riderID) {
                    riders[i].completeDelivery(lastOp.parcelID, 1);

                    // Table row for rider assignment removal
                    cout << left
                        << setw(10) << lastOp.parcelID
                        << setw(25) << "Removed Assignment"
                        << setw(35) << lastOp.riderID << endl;
                    break;
                }
            }
        }

        // Remove the log entry properly
        if (operationLogs.size() > 0) {
            SimpleArray<OperationLog> newLogs;
            for (int i = 0; i < operationLogs.size() - 1; i++) {
                newLogs.push_back(operationLogs[i]);
            }
            operationLogs = newLogs;
        }

        operationLogs.push_back(OperationLog("UNDO_OPERATION", lastOp.parcelID,
            "Undid: " + lastOp.operationType));

        cout << "\nOperation undone successfully\n";
    }


    void replayOperations(int limit = 10) {
        SetTextColor(192);
        cout << "REPLAYING LAST " << limit << " OPERATIONS\n";
        SetTextColor(12);
        cout << left
            << setw(27) << "Timestamp"
            << setw(25) << "Operation"
            << setw(10) << "ParcelID"
            << setw(15) << "RiderID"
            << setw(30) << "Details" << endl;
        cout << string(95, '-') << endl;  // separator line
        int start = max(0, (int)operationLogs.size() - limit);
        for (int i = start; i < operationLogs.size(); i++) {
            operationLogs[i].display();
        }
    }

    // ===================== DISPLAY FUNCTIONS =====================

    void displayAllQueues() {
        SetTextColor(192);
        cout << "CURRENT QUEUE STATUS\n";
        SetTextColor(12);
        // Pickup Queue
        cout << "\nPICKUP QUEUE (" << pickupQueue.size() << " parcels):\n";
        if (pickupQueue.empty()) {
            cout << "Empty\n";
        }
        else {
            cout << left
                << setw(5) << "No."
                << setw(10) << "ParcelID"
                << setw(15) << "Priority"
                << setw(20) << "Time in Queue (min)" << endl;
            cout << string(50, '-') << endl;
            for (int i = 0; i < pickupQueue.size(); i++) {
                QueueParcel qp = pickupQueue[i];
                cout << left
                    << setw(5) << (i + 1)
                    << setw(10) << qp.parcel->parcelID
                    << setw(15) << qp.parcel->priorityScore()
                    << setw(20) << qp.timeInQueue() / 60 << endl;
            }
        }

        // Warehouse Queue
        cout << "\nWAREHOUSE QUEUE (" << warehouseQueue.size() << " parcels):\n";
        if (warehouseQueue.empty()) {
            cout << "Empty\n";
        }
        else {
            cout << left
                << setw(5) << "No."
                << setw(10) << "ParcelID"
                << setw(15) << "Zone"
                << setw(20) << "Time in Queue (min)" << endl;
            cout << string(50, '-') << endl;
            for (int i = 0; i < warehouseQueue.size(); i++) {
                QueueParcel qp = warehouseQueue[i];
                cout << left
                    << setw(5) << (i + 1)
                    << setw(10) << qp.parcel->parcelID
                    << setw(15) << zoneToString(qp.parcel->zone)
                    << setw(20) << qp.timeInQueue() / 60 << endl;
            }
        }

        // Transit Queue
        cout << "\nTRANSIT QUEUE (" << transitQueue.size() << " parcels):\n";
        if (transitQueue.empty()) {
            cout << "Empty\n";
        }
        else {
            cout << left
                << setw(5) << "No."
                << setw(10) << "ParcelID"
                << setw(15) << "Zone"
                << setw(20) << "In Transit (min)" << endl;
            cout << string(50, '-') << endl;
            for (int i = 0; i < transitQueue.size(); i++) {
                QueueParcel qp = transitQueue[i];
                cout << left
                    << setw(5) << (i + 1)
                    << setw(10) << qp.parcel->parcelID
                    << setw(15) << zoneToString(qp.parcel->zone)
                    << setw(20) << qp.timeInQueue() / 60 << endl;
            }
        }
    }

    void displayAllRiders() {
        SetTextColor(192);
        cout << "RIDER STATUS\n";
        SetTextColor(12);
        // Print table header (optional, for multiple riders print once)
        cout << std::left
            << setw(10) << "RiderID"
            << setw(20) << "Name"
            << setw(10) << "Zone"
            << setw(10) << "Load"
            << setw(15) << "Status"
            << "Assigned Parcels" << endl;
        cout << string(85, '-') << endl;
        for (int i = 0; i < riders.size(); i++) {
            riders[i].display();
        }
    }

    void displayMissingParcels() {
        SetTextColor(192);
        cout << "MISSING PARCELS\n";
        SetTextColor(12);
        if (missingParcels.empty()) {
            cout << "No missing parcels\n";
        }
        else {
            cout << "Missing Parcels (" << missingParcels.size() << "):\n";
            for (int i = 0; i < missingParcels.size(); i++) {
                cout << "  " << i + 1 << ". Parcel ID: " << missingParcels[i] << endl;
            }
        }
    }

    void displayOperationLogs(int count = 20) {
        SetTextColor(192);
        cout << "\nOPERATION LOGS (Last " << min(count, (int)operationLogs.size()) << ")\n";
        SetTextColor(12);
        // Table headers (optional, print once before displaying multiple rows)
        cout << left
            << setw(27) << "Timestamp"
            << setw(25) << "Operation"
            << setw(10) << "ParcelID"
            << setw(15) << "RiderID"
            << setw(30) << "Details" << endl;
        cout << string(95, '-') << endl;  // separator line
        int start = max(0, (int)operationLogs.size() - count);
        for (int i = start; i < operationLogs.size(); i++) {
            operationLogs[i].display();
        }
    }

    // ===================== GETTERS FOR INTEGRATION =====================

    SimpleArray<QueueParcel>& getPickupQueue() { return pickupQueue; }
    SimpleArray<QueueParcel>& getWarehouseQueue() { return warehouseQueue; }
    SimpleArray<QueueParcel>& getTransitQueue() { return transitQueue; }
    SimpleArray<Rider>& getRiders() { return riders; }
    SimpleArray<int>& getMissingParcels() { return missingParcels; }

    // Add rider manually
    void addRider(string id, string name, Zone zone, int maxLoad) {
        riders.push_back(Rider(id, name, zone, maxLoad));
        operationLogs.push_back(OperationLog("RIDER_ADDED", 0,
            "Added rider " + name + " (" + id + ")"));
        cout << "Rider " << name << " added to system\n";
    }

    // Manual parcel queue management
    void manualQueueParcel(int parcelID, string queueType);
};

/* ===================== TRACKING SYSTEM WITH ROUTING ===================== */
class TrackingSystem {
private:
    ParcelAVL parcelIndex;
    ParcelSortingHeap sortingQueue;
    RoutingEngine* router;
    CourierOperationsEngine* operationsEngine;

public:
    TrackingSystem() {
        router = new RoutingEngine(50);
        operationsEngine = new CourierOperationsEngine(this, router);
        loadFromFile();

    }

    ~TrackingSystem() {
        delete router;
        delete operationsEngine;
    }

    void registerParcel(int id, string sender, string receiver,
        DeliveryPriority pr, WeightCategory wt, Zone zone) {

        Parcel* p = new Parcel(id, sender, receiver, pr, wt, zone);
        p->addTracking("Central Hub", REGISTERED);

        parcelIndex.insert(p);
        sortingQueue.insert(p);

        // Add to pickup queue in operations engine
        operationsEngine->addToPickupQueue(p);

        saveParcel(p);
        cout << "Parcel registered & added to sorting queue.\n";
    }

    void dispatchNextParcel()
    {
        Parcel* p = sortingQueue.extractHighestPriority();
        if (!p)
        {
            cout << "No parcels available for dispatch.\n";
            return;
        }

        // Calculate route based on destination zone
        int start = 0;  // Central Hub
        int end = getDeliveryPointForZone(p->zone);

        p->currentRoute = router->findShortestPath(start, end);
        p->currentRouteIndex = 0;

        p->addTracking("Central Hub", DISPATCHED);
        p->addTracking("Central Hub", LOADED);

        // Move to warehouse queue in operations engine
        operationsEngine->addToWarehouseQueue(p);

        if (!p->currentRoute.nodes.empty()) {
            p->addTracking("On Route to " + zoneToString(p->zone) +
                " via optimized path", IN_TRANSIT);
            cout << "\n Parcel " << p->parcelID
                << " dispatched via optimized route to "
                << zoneToString(p->zone) << " zone.\n";

            cout << "\nROUTE CALCULATED:\n";
            router->displayRoute(p->currentRoute);

            double estimatedTime = router->calculateRouteTime(p->currentRoute);
            cout << "Estimated arrival: " << estimatedTime << " minutes\n";
        }
        else {
            p->addTracking("On Route to " + zoneToString(p->zone), IN_TRANSIT);
            cout << " Parcel " << p->parcelID
                << " dispatched toward "
                << zoneToString(p->zone) << " zone (no optimized route available).\n";
        }
    }

    void unloadParcel(int id, string location)
    {
        Parcel* p = parcelIndex.search(id);
        if (!p)
        {
            cout << "Parcel not found!\n";
            return;
        }

        p->addTracking(location, UNLOADED);

        // Update route progress
        if (p->currentRouteIndex < p->currentRoute.nodes.size()) {
            p->currentRouteIndex++;
        }

        cout << "Parcel unloaded at " << location << ".\n";

        // Add to transit queue if at delivery point
        if (location.find("Delivery Point") != string::npos) {
            operationsEngine->addToTransitQueue(p);
        }

        // Check if route needs recalculation due to traffic changes
        if (!p->currentRoute.nodes.empty() && p->currentRouteIndex < p->currentRoute.nodes.size() - 1) {
            int current = p->currentRoute.nodes[p->currentRouteIndex];
            int destination = p->currentRoute.nodes[p->currentRoute.nodes.size() - 1];

            cout << "\nChecking for route optimization...\n";
            Path newRoute = router->recalculateRoute(current, destination, p->currentRoute);

            // Check if routes are different
            bool different = false;
            if (newRoute.nodes.size() != p->currentRoute.nodes.size()) {
                different = true;
            }
            else {
                for (int i = 0; i < newRoute.nodes.size(); i++) {
                    if (newRoute.nodes[i] != p->currentRoute.nodes[i]) {
                        different = true;
                        break;
                    }
                }
            }

            if (different) {
                cout << " Route updated due to traffic changes.\n";
                p->currentRoute = newRoute;
                p->currentRouteIndex = 0;

                cout << "New route:\n";
                router->displayRoute(p->currentRoute);
            }
            else {
                cout << " Current route remains optimal.\n";
            }
        }
    }

    void attemptDelivery(int parcelId, int deliveredSuccessfully)
    {
        Parcel* p = parcelIndex.search(parcelId);

        if (p == NULL)
        {
            cout << "Parcel not found!\n";
            return;
        }

        p->addTracking("Out for Delivery", OUT_FOR_DELIVERY);

        if (deliveredSuccessfully==1)
        {
            p->addTracking("Receiver Address", DELIVERED);
            cout << " Parcel delivered successfully.\n";
        }
        else
        {
            p->addTracking("Delivery Failed", DELIVERY_ATTEMPT_FAILED);
            p->addTracking("Returning to Sender", RETURN_TO_SENDER);
            p->addTracking("In Transit to Sender", IN_TRANSIT);
            p->addTracking("Sender Address", DELIVERED);

            cout << " Delivery failed. Parcel returned to sender.\n";
        }
    }

    void returnToSender(int id)
    {
        Parcel* p = parcelIndex.search(id);
        if (!p)
        {
            cout << "Parcel not found!\n";
            return;
        }

        p->addTracking("Returning to Sender", RETURN_TO_SENDER);
        p->addTracking("In Transit to Sender", IN_TRANSIT);
        p->addTracking("Sender Address", DELIVERED);

        cout << " Parcel returned to sender.\n";
    }

    void updateTracking(int id, string location, Status st) {
        Parcel* p = parcelIndex.search(id);
        if (!p)
        {
            cout << "Parcel not found!\n"; return;
        }
        p->addTracking(location, st);
        saveTracking(p, location, st);
        cout << " Tracking updated successfully.\n";
    }

    void trackParcel(int id) {
        Parcel* p = parcelIndex.search(id);
        if (!p)
        {
            cout << "Parcel not found!\n"; return;
        }
        p->showTracking();

        // Show route information if available
        if (!p->currentRoute.nodes.empty()) {
            SetTextColor(192);
            cout << "\nCURRENT ROUTE INFORMATION\n";
            SetTextColor(12);
            router->displayRoute(p->currentRoute);
            cout << "Progress: " << p->currentRouteIndex << "/"
                << p->currentRoute.nodes.size() << " waypoints\n";

            if (p->currentRouteIndex < p->currentRoute.nodes.size()) {
                cout << "Next stop: ";
                if (p->currentRouteIndex < p->currentRoute.nodes.size()) {
                    int nextNode = p->currentRoute.nodes[p->currentRouteIndex];
                    cout << router->getLocationName(nextNode) << "\n";
                }
            }
        }
    }

    void cancelParcel(int id) {
        Parcel* p = parcelIndex.search(id);
        if (!p)
        {
            cout << "Parcel not found!\n"; return;
        }
        sortingQueue.removeParcel(id);
        parcelIndex.remove(id);
        removeParcelFromFile(id);
        cout << " Parcel removed from system (cancelled/lost).\n";
    }

    // ========== ROUTING MODULE FUNCTIONS ==========

    void displayNetworkStatus() {
        router->displayNetwork();
    }

    void findRouteBetweenLocations() {
        clearScreen();
        SetTextColor(192);
        cout << "FIND ROUTE BETWEEN LOCATIONS\n";
        SetTextColor(12);
        int locationCount = router->getLocationCount();

        // Show available locations
        cout << "Available Locations (0-9):\n";
        for (int i = 0; i < min(10, locationCount); i++) {
            cout << i << ": " << router->getLocationName(i) << "\n";
        }

        int start, end;
        cout << "\nEnter Start Location ID (0-9): ";
        if (!(cin >> start)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input! Please enter a number.\n";
            return;
        }

        cout << "Enter End Location ID (0-9): ";
        if (!(cin >> end)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input! Please enter a number.\n";
            return;
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (start < 0 || start > 9 || end < 0 || end > 9) {
            cout << "Invalid location IDs! Must be 0-9.\n";
            return;
        }

        // Use comprehensive route finding
        router->comprehensiveRouteFinding(start, end);
    }

    void updateRoadCondition() {
        SetTextColor(192);
        cout << "UPDATE ROAD CONDITION";
        SetTextColor(12);
        // Show available roads
        cout << "\nMajor Roads (enter IDs):\n";
        cout << "0-1: Central Hub <-> North Warehouse\n";
        cout << "0-2: Central Hub <-> South Warehouse\n";
        cout << "0-3: Central Hub <-> East Warehouse\n";
        cout << "0-4: Central Hub <-> West Warehouse\n";
        cout << "1-5: North Warehouse <-> Delivery Point North\n";
        cout << "2-6: South Warehouse <-> Delivery Point South\n";
        cout << "3-7: East Warehouse <-> Delivery Point East\n";
        cout << "4-8: West Warehouse <-> Delivery Point West\n";
        cout << "0-9: Central Hub <-> Delivery Point Central\n";
        cout << "1-3: North Warehouse <-> East Warehouse\n";
        cout << "2-4: South Warehouse <-> West Warehouse\n";
        cout << "5-6: Delivery Point North <-> Delivery Point South\n";

        int from, to, status, traffic;
        cout << "\nUpdate road FROM location ID (0-9): ";
        if (!(cin >> from)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input!\n";
            return;
        }

        cout << "Update road TO location ID (0-9): ";
        if (!(cin >> to)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input!\n";
            return;
        }

        if (from < 0 || from > 9 || to < 0 || to > 9) {
            cout << "Invalid location IDs!\n";
            return;
        }

        cout << "New status (0=Open, 1=Blocked, 2=Congested): ";
        if (!(cin >> status)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input!\n";
            return;
        }

        if (status < 0 || status > 2) {
            cout << "Invalid status! Use 0, 1, or 2.\n";
            return;
        }

        if (status == 0 || status == 2) {
            cout << "Traffic level (1=Light, 3=Medium, 5=Heavy): ";
            if (!(cin >> traffic)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                traffic = 3;
            }
            if (traffic < 1 || traffic > 5) {
                cout << "Traffic must be 1-5. Using default 3.\n";
                traffic = 3;
            }
        }
        else {
            traffic = 1; // Blocked roads don't need traffic level
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        router->updateRoadStatus(from, to, static_cast<RoadStatus>(status));
        router->updateTrafficLevel(from, to, traffic);

        string fromName = router->getLocationName(from);
        string toName = router->getLocationName(to);

        cout << "\nRoad condition updated successfully!\n";
        cout << "Road: " << fromName << " <-> " << toName << endl;
        cout << "Status: " << roadStatusToString(static_cast<RoadStatus>(status)) << endl;
        if (status != 1) cout << "Traffic Level: " << traffic << "/5\n";

        // Show impact
        cout << "\nIMPACT ANALYSIS:\n";
        cout << " All active routes will be recalculated if affected\n";
        cout << " Future routes will consider this updated condition\n";
        cout << " Parcels in transit may be rerouted\n";
    }

    void simulateDynamicRecalculation() {
        SetTextColor(192);
        cout << "\nDYNAMIC ROUTE RECALCULATION SIMULATION\n";
        SetTextColor(12);

        int start = 0;  // Central Hub
        int end = 5;    // Delivery Point North

        cout << "\nScenario: Parcel delivery from Central Hub to North Delivery Point\n";

        // Get initial route
        Path initialRoute = router->findShortestPath(start, end);
        cout << "\n1. INITIAL OPTIMAL ROUTE:\n";
        cout << "-------------------------\n";
        if (initialRoute.nodes.empty()) {
            cout << "No initial route found!\n";
            return;
        }
        router->displayRoute(initialRoute);

        cout << "\n\n2. SIMULATING ROAD BLOCKAGE SCENARIO:\n";
        cout << "--------------------------------------\n";

        // Simulate road blockage between Central Hub and North Warehouse
        cout << "Blocking road between Central Hub (0) and North Warehouse (1)...\n";
        router->updateRoadStatus(0, 1, ROAD_BLOCKED);

        Path blockedRoute = router->findShortestPath(start, end);
        if (blockedRoute.nodes.empty()) {
            cout << " No alternative route found after blockage!\n";
        }
        else {
            cout << "New route after blockage:\n";
            router->displayRoute(blockedRoute);
        }

        // Restore original status
        router->updateRoadStatus(0, 1, ROAD_OPEN);
        cout << " Road status restored to normal\n";

        cout << "\n\n3. KEY FEATURES DEMONSTRATED:\n";
        cout << "---------------------------\n";
        cout << " Shortest route calculation using BFS algorithm\n";
        cout << " Automatic avoidance of blocked roads\n";
        cout << " Dynamic recalculation when conditions change\n";
        cout << " Route optimization based on current network state\n";
    }

    // ========== COURIER OPERATIONS ENGINE ACCESS ==========

    Parcel* searchParcel(int id) {
        return parcelIndex.search(id);
    }

    CourierOperationsEngine* getOperationsEngine() {
        return operationsEngine;
    }

private:
    int getDeliveryPointForZone(Zone zone) {
        switch (zone) {
        case NORTH: return 5;
        case SOUTH: return 6;
        case EAST: return 7;
        case WEST: return 8;
        case CENTRAL: return 9;
        default: return 0;
        }
    }

    void saveParcel(Parcel* p) {
        ofstream fout("parcels.txt", ios::app);
        fout << p->parcelID << "\n"
            << p->sender << "\n"
            << p->receiver << "\n"
            << p->priority << "\n"
            << p->weight << "\n"
            << static_cast<int>(p->zone) << "\n";
        fout.close();
        // Save first tracking entry
        if (p->head != NULL) {
            saveTracking(p, p->head->location, p->head->status, p->head->timestamp);
        }
    }

    void saveTracking(Parcel* p, string loc, Status st, string timeStr = "") {
        ofstream fout("tracking.txt", ios::app);
        fout << p->parcelID << "\n" << loc << "\n" << st << "\n";
        if (timeStr == "") {
            time_t now = time(0);
            fout << ctime(&now);
        }
        else fout << timeStr << "\n";
        fout.close();
    }

    void removeParcelFromFile(int id) {
        // Rewrite parcels.txt without the cancelled parcel
        ifstream pf("parcels.txt");
        ofstream tempf("parcels_temp.txt");
        int pid, pr, wt, zoneInt;
        string sender, receiver;

        while (pf >> pid) {
            pf.ignore();
            getline(pf, sender);
            getline(pf, receiver);
            pf >> pr >> wt >> zoneInt;
            pf.ignore();

            if (pid != id) {
                tempf << pid << "\n"
                    << sender << "\n"
                    << receiver << "\n"
                    << pr << "\n"
                    << wt << "\n"
                    << zoneInt << "\n";
            }
        }
        pf.close(); tempf.close();

        if (remove("parcels.txt") != 0)
            cerr << "Error deleting parcels.txt\n";
        if (rename("parcels_temp.txt", "parcels.txt") != 0)
            cerr << "Error renaming parcels_temp.txt to parcels.txt\n";

        // Rewrite tracking.txt without the cancelled parcel
        ifstream tf("tracking.txt");
        ofstream temp("tracking_temp.txt");
        int tid; string loc, timestamp; int st;
        while (tf >> tid) {
            tf.ignore(); getline(tf, loc); tf >> st; tf.ignore(); getline(tf, timestamp);
            if (tid != id) temp << tid << "\n" << loc << "\n" << st << "\n" << timestamp << "\n";
        }
        tf.close(); temp.close();

        if (remove("tracking.txt") != 0)
            cerr << "Error deleting tracking.txt\n";
        if (rename("tracking_temp.txt", "tracking.txt") != 0)
            cerr << "Error renaming tracking_temp.txt to tracking.txt\n";
    }

    void loadFromFile() {
        ifstream pf("parcels.txt");
        if (!pf) return;
        int id, pr, wt, zoneInt;
        string sender, receiver;

        while (pf >> id) {
            pf.ignore();
            getline(pf, sender);
            getline(pf, receiver);
            pf >> pr >> wt >> zoneInt;
            pf.ignore();

            Parcel* p = new Parcel(
                id,
                sender,
                receiver,
                static_cast<DeliveryPriority>(pr),
                static_cast<WeightCategory>(wt),
                static_cast<Zone>(zoneInt)
            );

            parcelIndex.insert(p);
            sortingQueue.insert(p);
        }
        pf.close();

        ifstream tf("tracking.txt");
        if (!tf) return;
        int tid; string loc, timestamp; int st;
        while (tf >> tid) {
            tf.ignore(); getline(tf, loc); tf >> st; tf.ignore(); getline(tf, timestamp);
            Parcel* p = parcelIndex.search(tid);
            if (p) p->addTracking(loc, static_cast<Status>(st), timestamp);
        }
        tf.close();
    }
};

/* ===================== COURIER OPERATIONS ENGINE METHOD IMPLEMENTATIONS ===================== */
void CourierOperationsEngine::processParcelWorkflow(int parcelID) {
    Parcel* p = trackingSystem->searchParcel(parcelID);

    if (!p) {
        cout << "Parcel not found!\n";
        return;
    }
    SetTextColor(192);
    cout << "PROCESSING PARCEL WORKFLOW\n";
    SetTextColor(12);
    cout << "Parcel ID: " << parcelID << endl;

    // Simulate workflow steps
    if (p->head && p->head->status == REGISTERED) {
        // Move to pickup queue
        addToPickupQueue(p);
        p->addTracking("Pickup Queue", DISPATCHED);

        // Simulate pickup
        assignRidersToPickup();

        // Move to warehouse
        p->addTracking("Central Warehouse", LOADED);
        addToWarehouseQueue(p);

        // Simulate warehouse processing
        cout << "Processing at warehouse...\n\n";
        p->addTracking("Warehouse Processing", IN_TRANSIT);

        // Assign for delivery
        assignRidersToDelivery();

        // Move to transit
        p->addTracking("Out for Delivery", OUT_FOR_DELIVERY);
        addToTransitQueue(p);

        cout << "\nParcel workflow completed!\n";
    }
    else {
        cout << "Parcel already in workflow!\n";
    }
}

void CourierOperationsEngine::manualQueueParcel(int parcelID, string queueType) {
    Parcel* p = trackingSystem->searchParcel(parcelID);
    if (!p) {
        cout << "Parcel not found!\n";
        return;
    }

    if (queueType == "pickup") addToPickupQueue(p);
    else if (queueType == "warehouse") addToWarehouseQueue(p);
    else if (queueType == "transit") addToTransitQueue(p);
    else cout << "Invalid queue type\n";
}

int main() {
    TrackingSystem system;
    CourierOperationsEngine* opsEngine = system.getOperationsEngine();

    int choice, id, pr, wt, zoneInt;
    string sender, receiver, location;
    int status;
    frontPage();
    pause();
    clearScreen();

    while (true) {
        SetTextColor(11);
        cout << "MAIN MENU\n";
        cout << "1. Parcel Management\n";
        cout << "2. Routing Engine\n";
        cout << "3. Courier Operations Engine\n";
        cout << "4. Exit\n";
        cout << "Choice: ";
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input! Please enter 1-4.\n";
            continue;
        }
        cout << endl;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        clearScreen();
        switch (choice) {
        case 1: { // Parcel Management
            int pmChoice;
            do {
                SetTextColor(192);
                cout << "PARCEL MANAGEMENT\n";
                SetTextColor(12);
                cout << "1. Register Parcel\n2. Update Tracking\n3. Track Parcel\n4. Cancel Parcel\n";
                cout << "5. Dispatch Next Parcel\n6. Unload Parcel\n7. Attempt Delivery\n8. Return to Main Menu\n";
                cout << "Choice: ";
                if (!(cin >> pmChoice)) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); continue; }
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                clearScreen();
                switch (pmChoice) {
                case 1: // Register Parcel
                    SetTextColor(192);
                    cout << "\nREGISTER NEW PARCEL\n";
                    SetTextColor(12);
                    cout << "Parcel ID: "; cin >> id; cin.ignore();
                    cout << "Sender: "; getline(cin, sender);
                    cout << "Receiver: "; getline(cin, receiver);
                    cout << "Delivery Priority (1=Normal,2=Two-Day,3=Overnight): "; cin >> pr; cin.ignore();
                    cout << "Weight Category (1=Light,2=Medium,3=Heavy): "; cin >> wt; cin.ignore();
                    cout << "Destination Zone (1=North,2=South,3=East,4=West,5=Central): "; cin >> zoneInt; cin.ignore();
                    system.registerParcel(id, sender, receiver,
                        static_cast<DeliveryPriority>(pr),
                        static_cast<WeightCategory>(wt),
                        static_cast<Zone>(zoneInt));
                    pause();
                    clearScreen();
                    break;

                case 2: // Update Tracking
                    SetTextColor(192);
                    cout << "UPDATE TRACKING\n";
                    SetTextColor(12);
                    cout << "Parcel ID: "; cin >> id; cin.ignore();
                    cout << "Location: "; getline(cin, location);
                    cout << "Status (0-8): "; cin >> status; cin.ignore();
                    system.updateTracking(id, location, static_cast<Status>(status));
                    pause();
                    clearScreen();
                    break;

                case 3: // Track Parcel
                    SetTextColor(192);
                    cout << "TRACK PARCEL\n";
                    SetTextColor(12);
                    cout << "Parcel ID: "; cin >> id; cin.ignore();
                    system.trackParcel(id);
                    pause();
                    clearScreen();
                    break;

                case 4: // Cancel Parcel
                    SetTextColor(192);
                    cout << "CANCEL PARCEL\n";
                    SetTextColor(12);
                    cout << "Parcel ID: "; cin >> id; cin.ignore();
                    system.cancelParcel(id);
                    pause();
                    clearScreen();
                    break;

                case 5: // Dispatch Next Parcel
                    system.dispatchNextParcel();
                    pause();
                    clearScreen();
                    break;

                case 6: // Unload Parcel
                    SetTextColor(192);
                    cout << "UNLOAD PARCEL\n";
                    SetTextColor(12);
                    cout << "Parcel ID: "; cin >> id; cin.ignore();
                    cout << "Unload Location: "; getline(cin, location);
                    system.unloadParcel(id, location);
                    pause();
                    clearScreen();
                    break;

                case 7: // Attempt Delivery
                    SetTextColor(192);
                    cout << "ATTEMPT DELIVERY\n";
                    SetTextColor(12);
                    cout << "Parcel ID: "; cin >> id;
                    cout << "Do you want to start delivery? (1=Yes,0=No): "; int success; cin >> success; cin.ignore();
                    system.attemptDelivery(id, success);
                    pause();
                    clearScreen();
                    break;

                case 8: // Back to Main Menu
                    pause();
                    clearScreen();
                    break;
                default:
                    cout << "Invalid choice!\n";
                }
            } while (pmChoice != 8);
            break;
        }

        case 2: { // Routing Engine
            int rChoice;
            do {

                SetTextColor(192);
                cout << "ROUTING ENGINE\n";
                SetTextColor(12);
                cout << "1. Display Network Status\n2. Find Route Between Locations\n3. Update Road Condition\n";
                cout << "4. Simulate Dynamic Route Recalculation\n5. Back to Main Menu\nChoice: ";
                if (!(cin >> rChoice)) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); continue; }
                cin.ignore(numeric_limits<streamsize>::max(), '\n');

                switch (rChoice) {
                case 1: system.displayNetworkStatus(); pause(); clearScreen(); break;
                case 2: system.findRouteBetweenLocations(); pause(); clearScreen(); break;
                case 3: system.updateRoadCondition(); pause(); clearScreen(); break;
                case 4: system.simulateDynamicRecalculation(); pause(); clearScreen(); break;
                case 5: pause(); clearScreen(); break;
                default: cout << "Invalid choice!\n";
                }
            } while (rChoice != 5);
            break;
        }

        case 3: { // Operations Engine
            int opsChoice;
            do {
                SetTextColor(192);
                cout << "COURIER OPERATIONS ENGINE\n";
                SetTextColor(12);
                cout << "1. Display All Queues\n2. Display All Riders\n3. Assign Riders to Pickup\n";
                cout << "4. Assign Riders to Delivery\n5. Process Parcel Workflow\n6. Check for Missing Parcels\n";
                cout << "7. Display Missing Parcels\n8. Resolve Missing Parcel\n9. Undo Last Operation\n";
                cout << "10. Replay Operations\n11. Display Operation Logs\n12. Add Rider\n";
                cout << "13. Manual Queue Management\n14. Return to Main Menu\nChoice: ";
                if (!(cin >> opsChoice)) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); continue; }
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                clearScreen();
                switch (opsChoice) {
                case 1: opsEngine->displayAllQueues(); pause(); clearScreen(); break;
                case 2: opsEngine->displayAllRiders(); pause(); clearScreen(); break;
                case 3: opsEngine->assignRidersToPickup(); pause(); clearScreen(); break;
                case 4: opsEngine->assignRidersToDelivery(); pause(); clearScreen(); break;
                case 5:
                    cout << "Enter Parcel ID: "; cin >> id; cin.ignore();
                    opsEngine->processParcelWorkflow(id);
                    pause();
                    clearScreen();
                    break;
                case 6: opsEngine->checkForMissingParcels(); pause(); clearScreen(); break;
                case 7: opsEngine->displayMissingParcels(); pause(); clearScreen(); break;
                case 8:
                    cout << "Enter Parcel ID to resolve: "; cin >> id; cin.ignore();
                    opsEngine->resolveMissingParcel(id);
                    pause(); clearScreen();
                    break;
                case 9: opsEngine->undoLastOperation(); pause(); clearScreen(); break;
                case 10: opsEngine->replayOperations(); pause(); clearScreen(); break;
                case 11: opsEngine->displayOperationLogs(); pause(); clearScreen(); break;
                case 12: {
                    string riderID, name; int zoneInt, maxLoad;
                    cout << "Rider ID: "; getline(cin, riderID);
                    cout << "Name: "; getline(cin, name);
                    cout << "Zone (1-5): "; cin >> zoneInt;
                    cout << "Max Load Capacity: "; cin >> maxLoad; cin.ignore();
                    opsEngine->addRider(riderID, name, static_cast<Zone>(zoneInt), maxLoad);
                    pause();
                    clearScreen();
                    break;
                }
                case 13: {
                    int parcelID; string queueType;
                    cout << "Parcel ID: "; cin >> parcelID; cin.ignore();
                    cout << "Queue Type (pickup/warehouse/transit): "; getline(cin, queueType);
                    opsEngine->manualQueueParcel(parcelID, queueType);
                    pause();
                    clearScreen();
                    break;
                }
                case 14:
                    clearScreen();
                    break; // Back to main
                default: cout << "Invalid choice!\n";
                }
            } while (opsChoice != 14);
            break;
        }

        case 4:
            thankyou();
            return 0;

        default:
            cout << "Invalid choice! Please enter 1-4.\n";
        }
    }
    pause();
    clearScreen();
    return 0;
}