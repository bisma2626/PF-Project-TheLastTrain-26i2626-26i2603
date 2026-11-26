
#include "logger.h"
#include <iostream>
#include <fstream>
using namespace std;

// Saves the current game.
bool saveGameStateToFile(const char* filename,
                         int pFloor, int pRoom,
                         int energy, int time, int score,
                         const char inventory[MAX_INVENTORY_CAPACITY],
                         int itemCount,
                         const bool objectives[NUM_OBJECTIVES],
                         int tripsLeft,
                         const char roomItems[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                         const int itemCounts[NUM_FLOORS][ROOMS_PER_FLOOR]) {

    if (filename == nullptr || itemCount < 0 ||
        itemCount > MAX_INVENTORY_CAPACITY) {
        return false;
    }

    ofstream file(filename);

    if (!file) {
        return false;
    }

    file << pFloor << " " << pRoom << " "
         << energy << " " << time << " " << score << " "
         << itemCount << " " << tripsLeft << "\n";

    // Save inventory.
    for (int i = 0; i < itemCount; i++) {
        file << inventory[i] << " ";
    }
    file << "\n";

    // Save completed objectives.
    for (int i = 0; i < NUM_OBJECTIVES; i++) {
        file << (objectives[i] ? 1 : 0) << " ";
    }
    file << "\n";

    // Save items left in each room.
    for (int floorIndex = 0; floorIndex < NUM_FLOORS; floorIndex++) {
        for (int roomIndex = 0; roomIndex < ROOMS_PER_FLOOR; roomIndex++) {

            int count = itemCounts[floorIndex][roomIndex];

            if (count < 0 || count > MAX_ENTITIES_PER_ROOM) {
                return false;
            }

            file << count << " ";

            for (int i = 0; i < count; i++) {
                file << roomItems[floorIndex][roomIndex][i] << " ";
            }

            file << "\n";
        }
    }

    return file.good();
}


// Loads a saved game.
bool loadGameStateFromFile(const char* filename,
                           int& pFloor, int& pRoom,
                           int& energy, int& time, int& score,
                           char inventory[MAX_INVENTORY_CAPACITY],
                           int& itemCount,
                           bool objectives[NUM_OBJECTIVES],
                           int& tripsLeft,
                           char roomItems[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                           int itemCounts[NUM_FLOORS][ROOMS_PER_FLOOR]) {

    if (filename == nullptr) {
        return false;
    }

    ifstream file(filename);

    if (!file) {
        return false;
    }

    // Read into temporary variables first.
    int savedFloor, savedRoom, savedEnergy, savedTime;
    int savedScore, savedCount, savedTrips;

    if (!(file >> savedFloor >> savedRoom >> savedEnergy
               >> savedTime >> savedScore >> savedCount >> savedTrips)) {
        return false;
    }

    if (savedFloor < 1 || savedFloor > NUM_FLOORS ||
        savedRoom < 1 || savedRoom > ROOMS_PER_FLOOR ||
        savedEnergy < 0 || savedEnergy > MAX_ENERGY ||
        savedTime < 0 || savedTime > MAX_TIME ||
        savedScore < 0 || savedTrips < 0 ||
        savedCount < 0 || savedCount > MAX_INVENTORY_CAPACITY) {
        return false;
    }

    // Load inventory.
    char savedItems[MAX_INVENTORY_CAPACITY] = {};

    for (int i = 0; i < savedCount; i++) {
        if (!(file >> savedItems[i])) {
            return false;
        }
    }

    // Load objective progress.
    bool savedObjectives[NUM_OBJECTIVES] = {};

    for (int i = 0; i < NUM_OBJECTIVES; i++) {
        int value;

        if (!(file >> value) || (value != 0 && value != 1)) {
            return false;
        }

        savedObjectives[i] = (value == 1);
    }

    // Load items from all rooms.
    char savedRooms[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM] = {};
    int savedCounts[NUM_FLOORS][ROOMS_PER_FLOOR] = {};

    for (int floorIndex = 0; floorIndex < NUM_FLOORS; floorIndex++) {
        for (int roomIndex = 0; roomIndex < ROOMS_PER_FLOOR; roomIndex++) {

            int count;

            if (!(file >> count) || count < 0 ||
                count > MAX_ENTITIES_PER_ROOM) {
                return false;
            }

            savedCounts[floorIndex][roomIndex] = count;

            for (int i = 0; i < count; i++) {
                if (!(file >> savedRooms[floorIndex][roomIndex][i])) {
                    return false;
                }
            }
        }
    }

    // Everything is valid, so update the actual game.
    pFloor = savedFloor;
    pRoom = savedRoom;
    energy = savedEnergy;
    time = savedTime;
    score = savedScore;
    itemCount = savedCount;
    tripsLeft = savedTrips;

    for (int i = 0; i < MAX_INVENTORY_CAPACITY; i++) {
        inventory[i] = savedItems[i];
    }

    for (int i = 0; i < NUM_OBJECTIVES; i++) {
        objectives[i] = savedObjectives[i];
    }

    for (int floorIndex = 0; floorIndex < NUM_FLOORS; floorIndex++) {
        for (int roomIndex = 0; roomIndex < ROOMS_PER_FLOOR; roomIndex++) {

            itemCounts[floorIndex][roomIndex] =
                savedCounts[floorIndex][roomIndex];

            for (int i = 0; i < MAX_ENTITIES_PER_ROOM; i++) {
                roomItems[floorIndex][roomIndex][i] =
                    savedRooms[floorIndex][roomIndex][i];
            }
        }
    }

    return true;
}


// Creates the final game report.
void writeGameSummaryReport(const char* filename,
                            int score, int energy, int time,
                            const bool objectives[NUM_OBJECTIVES],
                            bool victory) {

    if (filename == nullptr) {
        return;
    }

    ofstream file(filename);

    if (!file) {
        return;
    }

    const char* names[OBJ_MISSION_WON] = {
        "Collect Master Key",
        "Find Clue 4",
        "Find Clue 2",
        "Find Clue 7",
        "Unlock Cabinet",
        "Collect Train Key",
        "Collect Fuse",
        "Install Fuse",
        "Collect Lift Pass",
        "Activate Lift 2",
        "Rescue Passenger",
        "Collect Cat Carrier",
        "Secure Cat",
        "Prepare Train"
    };

    file << "========== THE LAST TRAIN ==========\n";

    if (victory) {
        file << "Result: VICTORY\n";
    }
    else if (objectives[OBJ_MISSION_FAILED]) {
        file << "Result: MISSION FAILED\n";
    }
    else {
        file << "Result: INCOMPLETE (player quit)\n";
    }

    file << "Final Score: " << score << "\n";
    file << "Remaining Energy: " << energy << "\n";
    file << "Remaining Time: " << time << "\n";

    file << "\nOBJECTIVES:\n";

    int done = 0;

    for (int i = 0; i < OBJ_MISSION_WON; i++) {
        file << (objectives[i] ? "[X] " : "[ ] ")
             << names[i] << "\n";

        if (objectives[i]) {
            done++;
        }
    }

    file << "\nSession audit: " << done << " of "
         << OBJ_MISSION_WON << " objectives completed.\n";

    return;
}
