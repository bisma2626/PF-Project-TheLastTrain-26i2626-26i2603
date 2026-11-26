
#include "physics.h"
#include "util.h"

// Checks if two rooms actually have a connection. No random teleporting!
bool isDirectlyConnected(int currentFloor, int currentRoom,
                         int targetFloor, int targetRoom,
                         const char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
                         char& connectionType) {

    // Start with no connection.
    connectionType = CONN_NONE;

    // First, make sure both room locations are valid.
    if (currentFloor < 1 || currentFloor > NUM_FLOORS ||
        targetFloor < 1 || targetFloor > NUM_FLOORS ||
        currentRoom < 1 || currentRoom > ROOMS_PER_FLOOR ||
        targetRoom < 1 || targetRoom > ROOMS_PER_FLOOR) {
        return false;
    }

    // A room cannot connect to itself. Obviously!
    if (currentFloor == targetFloor && currentRoom == targetRoom) {
        return false;
    }

    // Different floors? Let's check stairs and lifts.
    if (currentFloor != targetFloor) {

        // Stairs connect room 1 on floors 1-2, 2-3 and 4-5.
        if (currentRoom == 1 && targetRoom == 1) {

            int lowerFloor = currentFloor;

            if (targetFloor < currentFloor) {
                lowerFloor = targetFloor;
            }

            int floorDifference = currentFloor - targetFloor;

            if (floorDifference < 0) {
                floorDifference = -floorDifference;
            }

            if (floorDifference == 1 &&
                (lowerFloor == 1 || lowerFloor == 2 || lowerFloor == 4)) {

                connectionType = CONN_STAIRS;
                return true;
            }
        }

        // Lift 1 connects rooms 2 and 3 on floors 1 and 2.
        if (currentFloor <= 2 && targetFloor <= 2 &&
            (currentRoom == 2 || currentRoom == 3) &&
            (targetRoom == 2 || targetRoom == 3)) {

            connectionType = CONN_LIFT1;
            return true;
        }

        // Lift 2 connects room 4 on floors 2, 4 and 5.
        if (currentRoom == 4 && targetRoom == 4 &&
            (currentFloor == 2 || currentFloor == 4 || currentFloor == 5) &&
            (targetFloor == 2 || targetFloor == 4 || targetFloor == 5)) {

            connectionType = CONN_LIFT2;
            return true;
        }

        // No matching stairs or lift? No connection.
        return false;
    }

    // Same floor: rooms must be next to each other.
    if (currentRoom - targetRoom != 1 &&
        targetRoom - currentRoom != 1) {
        return false;
    }

    // Find the position of the connection between the two rooms.
    int leftRoom = currentRoom;

    if (targetRoom < currentRoom) {
        leftRoom = targetRoom;
    }

    connectionType = horizontalConn[currentFloor - 1][leftRoom - 1];

    // Doors, windows and locked doors are valid passages.
    if (connectionType == CONN_DOOR ||
        connectionType == CONN_WINDOW ||
        connectionType == CONN_LOCKED_DOOR) {
        return true;
    }

    // Keep the wall symbol so the game can identify a wall.
    // Anything else means there is no connection.
    if (connectionType != CONN_SOLID_WALL) {
        connectionType = CONN_NONE;
    }

    return false;
}


// Checks if the player is allowed to cross a connection.
bool canTraverse(int targetFloor, int targetRoom, char connectionType,
                 const char inventory[MAX_INVENTORY_CAPACITY],
                 int inventoryCount, int emergencyTripsLeft,
                 bool lift2Operational) {

    // Destination must be inside the building.
    if (targetFloor < 1 || targetFloor > NUM_FLOORS ||
        targetRoom < 1 || targetRoom > ROOMS_PER_FLOOR) {
        return false;
    }

    // These connections are always accessible.
    if (connectionType == CONN_DOOR ||
        connectionType == CONN_WINDOW ||
        connectionType == CONN_STAIRS ||
        connectionType == CONN_LIFT1) {
        return true;
    }

    // Locked doors need the Master Key. No key, no entry!
    if (connectionType == CONN_LOCKED_DOOR) {
        return hasItem(inventory, inventoryCount, ENT_MASTER_KEY);
    }

    // Lift 2 needs to be repaired or have an emergency trip available.
    if (connectionType == CONN_LIFT2) {
        return lift2Operational || emergencyTripsLeft > 0;
    }

    // Walls, empty spaces and unknown connections are not allowed.
    return false;
}


// Updates energy and time without letting them exceed their limits.
void applyResourceDelta(int& energy, int& time,
                        int energyChange, int timeChange) {

    energy = clampInt(energy + energyChange, 0, MAX_ENERGY);
    time = clampInt(time + timeChange, 0, MAX_TIME);
}


// Updates the score. It can increase or decrease, but never go below zero.
void applyScoreDelta(int& score, int scoreChange) {

    score = score + scoreChange;

    if (score < 0) {
        score = 0;
    }
}


// Checks if the player already has a particular item.
bool hasItem(const char inventory[MAX_INVENTORY_CAPACITY],
             int inventoryCount, char itemSymbol) {

    // Check each occupied inventory position.
    for (int itemPosition = 0;
         itemPosition < inventoryCount && itemPosition < MAX_INVENTORY_CAPACITY;
         itemPosition++) {

        if (inventory[itemPosition] == itemSymbol) {
            return true;
        }
    }

    return false;
}


// Adds an item to the inventory. No space? No pickup!
bool addItem(char inventory[MAX_INVENTORY_CAPACITY],
             int& inventoryCount, char itemSymbol) {

    // Check if the inventory is full or the count is invalid.
    if (inventoryCount < 0 || inventoryCount >= MAX_INVENTORY_CAPACITY) {
        return false;
    }

    // Empty symbols are not actual items.
    if (itemSymbol == ENT_NONE) {
        return false;
    }

    // Put the item in the next available position.
    inventory[inventoryCount] = itemSymbol;
    inventoryCount++;

    return true;
}


// Removes one copy of an item and closes the gap.
bool removeItem(char inventory[MAX_INVENTORY_CAPACITY],
                int& inventoryCount, char itemSymbol) {

    // Look for the requested item.
    for (int itemPosition = 0;
         itemPosition < inventoryCount && itemPosition < MAX_INVENTORY_CAPACITY;
         itemPosition++) {

        if (inventory[itemPosition] == itemSymbol) {

            // Shift the remaining items one position to the left.
            for (int nextPosition = itemPosition;
                 nextPosition < inventoryCount - 1;
                 nextPosition++) {

                inventory[nextPosition] = inventory[nextPosition + 1];
            }

            // One item has been removed.
            inventoryCount--;

            return true;
        }
    }

    // The item was not in the inventory.
    return false;
}
