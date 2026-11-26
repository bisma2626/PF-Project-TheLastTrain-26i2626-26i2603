
#include "switches.h"
#include "physics.h"
#include <cstring>

// Lift 1 only goes between Floor 1 and Floor 2.
bool canUseLift1(int currentFloor, int targetFloor) {
    if ((currentFloor == 1 && targetFloor == 2) ||
        (currentFloor == 2 && targetFloor == 1)) {
        return true;
    }

    return false;
}

// Lift 2 connects Floors 2, 4 and 5.
// It works if the fuse is installed or an emergency trip is available.
bool canUseLift2(int currentFloor, int targetFloor,
                 bool isLift2Working, int emergencyTripsLeft) {

    bool r_FloorsConnected =
        ((currentFloor == 2 || currentFloor == 4 || currentFloor == 5) &&
         (targetFloor == 2 || targetFloor == 4 || targetFloor == 5) &&
         currentFloor != targetFloor);

    if (!r_FloorsConnected) {
        return false;
    }

    if (isLift2Working || emergencyTripsLeft > 0) {
        return true;
    }

    return false;
}

// Need the Master Key to get through this locked door.
bool unlockFloor3Door(const char inventory[MAX_INVENTORY_CAPACITY],
                      int inventoryCount) {

    if (hasItem(inventory, inventoryCount, ENT_MASTER_KEY)) {
        return true;
    }

    return false;
}

// Install the Fuse to activate the control panel and get the points.
bool installFuseControlPanel(char inventory[MAX_INVENTORY_CAPACITY],
                             int& inventoryCount,
                             bool objectives[NUM_OBJECTIVES],
                             int& score) {

    if (objectives[OBJ_FUSE_INSTALLED]) {
        return false;
    }

    if (!hasItem(inventory, inventoryCount, ENT_FUSE)) {
        return false;
    }

    if (!removeItem(inventory, inventoryCount, ENT_FUSE)) {
        return false;
    }

    objectives[OBJ_FUSE_COLLECTED] = true;
    objectives[OBJ_FUSE_INSTALLED] = true;

    applyScoreDelta(score, SCORE_INSTALL_FUSE);

    // Got the Lift Pass already? Then Lift 2 can start working too.
    // It doesn't matter whether we collect the pass or install the fuse first.
    if (objectives[OBJ_LIFT_PASS]) {
        objectives[OBJ_LIFT2_ACTIVE] = true;
    }

    return true;
}

// Enter the code 427 after collecting all three clues.
bool verifyCabinetCode(const char* enteredCode,
                       const bool objectives[NUM_OBJECTIVES]) {

    if (enteredCode == nullptr) {
        return false;
    }

    if (!objectives[OBJ_CLUE_4] ||
        !objectives[OBJ_CLUE_2] ||
        !objectives[OBJ_CLUE_7]) {
        return false;
    }

    if (strcmp(enteredCode, "427") == 0) {
        return true;
    }

    return false;
}
