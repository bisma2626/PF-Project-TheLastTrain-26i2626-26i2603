
#include "viewer_iface.h"
#include "level.h"
#include <iostream>
using namespace std;

// Shows the player's stats and inventory.
void renderDashboard(int pFloor, int pRoom, int energy, int time,
                     int score,
                     const char inventory[MAX_INVENTORY_CAPACITY],
                     int itemCount, int emergencyTripsLeft) {

    cout << "\n====================================\n";
    cout << "          THE LAST TRAIN\n";
    cout << "====================================\n";

    cout << "Current Location : Floor " << pFloor
         << ", Room " << pRoom << endl;

    cout << "Energy           : " << energy << endl;
    cout << "Time Remaining   : " << time << endl;
    cout << "Score            : " << score << endl;
    cout << "Emergency Trips  : " << emergencyTripsLeft << endl;

    cout << "\nInventory (type the symbol to use an item):\n";

    if (itemCount == 0) {
        cout << "Your inventory is empty.\n";
    }
    else {
        for (int itemIndex = 0;
             itemIndex < itemCount && itemIndex < MAX_INVENTORY_CAPACITY;
             itemIndex++) {

            cout << "- [" << inventory[itemIndex] << "] "
                 << getEntityName(inventory[itemIndex]) << endl;
        }
    }

    cout << "====================================\n";
}

// Shows what's available in the current room.
void renderRoomDetails(int floorNum, int roomNum,
                       const char roomItems[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                       int itemCount) {

    cout << "\n--- ROOM DETAILS ---\n";
    cout << "Floor: " << floorNum << endl;
    cout << "Room : " << roomNum << endl;

    if (floorNum < 1 || floorNum > NUM_FLOORS ||
        roomNum < 1 || roomNum > ROOMS_PER_FLOOR) {

        cout << "Invalid room.\n";
        return;
    }

    if (itemCount <= 0) {
        cout << "No items or people found here.\n";
        return;
    }

    if (itemCount > MAX_ENTITIES_PER_ROOM) {
        itemCount = MAX_ENTITIES_PER_ROOM;
    }

    cout << "Found in this room:\n";

    for (int itemIndex = 0; itemIndex < itemCount; itemIndex++) {

        char itemSymbol = roomItems[floorNum - 1][roomNum - 1][itemIndex];

        if (itemSymbol != ENT_NONE) {
            cout << "- [" << itemSymbol << "] "
                 << getEntityName(itemSymbol) << endl;
        }
    }
}

// Shows the building and where the player currently is.
void renderAsciiBuilding(int pFloor, int pRoom) {

    // Map connections for each floor.
    const char* mapLinks[NUM_FLOORS][ROOMS_PER_FLOOR - 1] = {
        {"W", "L1", "D"},
        {"D", "L1", "D"},
        {"L", "W", "D"},
        {"W", "#", "D"},
        {"D", "D", "W"}
    };

    cout << "\n========== BUILDING MAP ==========\n";

    for (int floorNum = NUM_FLOORS; floorNum >= 1; floorNum--) {

        cout << "Floor " << floorNum << " : ";

        for (int roomNum = 1; roomNum <= ROOMS_PER_FLOOR; roomNum++) {

            if (floorNum == pFloor && roomNum == pRoom) {
                cout << "[P ]";
            }
            else {
                cout << "[R" << roomNum << "]";
            }

            if (roomNum < ROOMS_PER_FLOOR) {
                cout << "--" << mapLinks[floorNum - 1][roomNum - 1] << "--";
            }
        }

        cout << endl;
    }

    cout << "\nP = Your Current Position\n";
    cout << "D = Door | W = Window | L = Locked Door\n";
    cout << "L1 = Lift 1 | # = Wall\n";
    cout << "==================================\n";
}

// Shows which objectives are completed.
void renderObjectivesChecklist(const bool objectives[NUM_OBJECTIVES]) {

    const char* objectiveNames[NUM_OBJECTIVES] = {
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
        "Prepare Train",
        "Win Mission",
        "Mission Failed"
    };

    cout << "\n========== OBJECTIVES ==========\n";

    // Only show the actual objectives, not the final game status.
    for (int objIndex = 0; objIndex < OBJ_MISSION_WON; objIndex++) {

        if (objectives[objIndex]) {
            cout << "[X] ";
        }
        else {
            cout << "[ ] ";
        }

        cout << objectiveNames[objIndex] << endl;
    }

    cout << "================================\n";
}
