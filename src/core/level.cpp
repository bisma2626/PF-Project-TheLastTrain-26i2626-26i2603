
#include "level.h"
#include "util.h"
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>

using namespace std;


// Checks whether a symbol is an actual game item.
static bool isEntitySymbol(char itemSymbol) {

    switch (itemSymbol) {
        case ENT_MASTER_KEY:
        case ENT_ENERGY_PACK:
        case ENT_EMERGENCY_TOOLKIT:
        case ENT_CLUE_4:
        case ENT_FUSE:
        case ENT_CAT:
        case ENT_CONTROL_PANEL:
        case ENT_LIFT_PASS:
        case ENT_PASSENGER:
        case ENT_MEDICAL_KIT:
        case ENT_EMERGENCY_CARD:
        case ENT_CLUE_2:
        case ENT_CLUE_7:
        case ENT_CABINET_TRAIN_KEY:
        case ENT_CAT_CARRIER:
        case ENT_OBSERVATION_TOKEN:
            return true;

        default:
            return false;
    }
}


// Finds where each room label is chilling in the floor line.
static bool findRoomPositions(const char* floorLine, int floorNumber,
                              int roomPositions[ROOMS_PER_FLOOR]) {

    for (int roomNumber = 1; roomNumber <= ROOMS_PER_FLOOR; roomNumber++) {

        char roomLabel[10];

        // Creates labels like F1-R1 and F3-R4.
        snprintf(roomLabel, sizeof(roomLabel),
                 "F%d-R%d", floorNumber, roomNumber);

        const char* labelLocation = strstr(floorLine, roomLabel);

        // Every room needs to have its label.
        if (labelLocation == nullptr) {
            return false;
        }

        roomPositions[roomNumber - 1] = labelLocation - floorLine;
    }

    return true;
}


// Converts game symbols into readable item names.
const char* getEntityName(char itemSymbol) {

    switch (itemSymbol) {

        case ENT_MASTER_KEY:
            return "Master Key";

        case ENT_ENERGY_PACK:
            return "Energy Pack";

        case ENT_EMERGENCY_TOOLKIT:
            return "Emergency Toolkit";

        case ENT_CLUE_4:
            return "Clue 4";

        case ENT_FUSE:
            return "Fuse";

        case ENT_CAT:
            return "Cat";

        case ENT_CONTROL_PANEL:
            return "Control Panel";

        case ENT_LIFT_PASS:
            return "Lift Pass";

        case ENT_PASSENGER:
            return "Passenger";

        case ENT_MEDICAL_KIT:
            return "Medical Kit";

        case ENT_EMERGENCY_CARD:
            return "Emergency Access Card";

        case ENT_CLUE_2:
            return "Clue 2";

        case ENT_CLUE_7:
            return "Clue 7";

        case ENT_CABINET_TRAIN_KEY:
            return "Cabinet Train Key";

        case ENT_CAT_CARRIER:
            return "Cat Carrier";

        case ENT_OBSERVATION_TOKEN:
            return "Observation Token";

        default:
            return "Unknown Item";
    }
}


// Loads the building map, its connections and all room items.
bool loadBuildingMap(const char* filename,
                     char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
                     char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                     int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR]) {

    if (filename == nullptr) {
        return false;
    }

    FILE* mapFile = fopen(filename, "r");

    // No map file? We cannot start the game like this!
    if (mapFile == nullptr) {
        return false;
    }

    // Give every room a fresh, empty setup.
    for (int floorIndex = 0; floorIndex < NUM_FLOORS; floorIndex++) {

        for (int connectionIndex = 0;
             connectionIndex < ROOMS_PER_FLOOR - 1;
             connectionIndex++) {

            horizontalConn[floorIndex][connectionIndex] = CONN_NONE;
        }

        for (int roomIndex = 0; roomIndex < ROOMS_PER_FLOOR; roomIndex++) {

            entityCounts[floorIndex][roomIndex] = 0;

            for (int itemIndex = 0;
                 itemIndex < MAX_ENTITIES_PER_ROOM;
                 itemIndex++) {

                roomEntities[floorIndex][roomIndex][itemIndex] = ENT_NONE;
            }
        }
    }

    bool floorLoaded[NUM_FLOORS] = {false};

    char floorLine[MAX_LINE_LEN];
    char itemsLine[MAX_LINE_LEN];

    // Let's read the building map one line at a time.
    while (fgets(floorLine, MAX_LINE_LEN, mapFile) != nullptr) {

        int floorNumber = 0;

        // Figure out which floor this line belongs to.
        for (int checkFloor = 1; checkFloor <= NUM_FLOORS; checkFloor++) {

            char firstRoomLabel[10];

            snprintf(firstRoomLabel, sizeof(firstRoomLabel),
                     "F%d-R1", checkFloor);

            if (strstr(floorLine, firstRoomLabel) != nullptr) {
                floorNumber = checkFloor;
                break;
            }
        }

        // Not a floor line? We don't need it.
        if (floorNumber == 0) {
            continue;
        }

        int roomPositions[ROOMS_PER_FLOOR];

        // Make sure all four room labels are present.
        if (!findRoomPositions(floorLine, floorNumber, roomPositions)) {
            continue;
        }

        // The next line tells us what's inside the rooms.
        if (fgets(itemsLine, MAX_LINE_LEN, mapFile) == nullptr) {
            fclose(mapFile);
            return false;
        }

        int floorIndex = floorNumber - 1;

        // Each floor should only be loaded once.
        if (floorLoaded[floorIndex]) {
            fclose(mapFile);
            return false;
        }

        floorLoaded[floorIndex] = true;

        // PART 1: Find the connections between rooms.

        for (int roomIndex = 0;
             roomIndex < ROOMS_PER_FLOOR - 1;
             roomIndex++) {

            int connectionStart = roomPositions[roomIndex] + 5;
            int connectionEnd = roomPositions[roomIndex + 1];

            for (int position = connectionStart;
                 position < connectionEnd;
                 position++) {

                char connectionSymbol = floorLine[position];

                // L1 is a lift, not a locked door. Different situation!
                if (connectionSymbol == 'L' &&
                    position + 1 < MAX_LINE_LEN &&
                    floorLine[position + 1] == '1') {

                    position++;
                    continue;
                }

                if (connectionSymbol == CONN_DOOR ||
                    connectionSymbol == CONN_WINDOW ||
                    connectionSymbol == CONN_LOCKED_DOOR ||
                    connectionSymbol == CONN_SOLID_WALL) {

                    horizontalConn[floorIndex][roomIndex] = connectionSymbol;
                    break;
                }
            }
        }

        // PART 2: Find the items inside each room.

        for (int roomIndex = 0;
             roomIndex < ROOMS_PER_FLOOR;
             roomIndex++) {

            int itemStart = roomPositions[roomIndex];
            int itemEnd;

            // Stop reading when the next room begins.
            if (roomIndex < ROOMS_PER_FLOOR - 1) {
                itemEnd = roomPositions[roomIndex + 1];
            }
            else {
                itemEnd = strlen(itemsLine);
            }

            for (int position = itemStart;
                 position < itemEnd;
                 position++) {

                char itemSymbol = itemsLine[position];

                // Stop at the room border. No stealing from the next area!
                if (itemSymbol == '|') {
                    break;
                }

                if (isEntitySymbol(itemSymbol)) {

                    int& roomItemCount = entityCounts[floorIndex][roomIndex];

                    // Each room has a limited item capacity.
                    if (roomItemCount >= MAX_ENTITIES_PER_ROOM) {
                        fclose(mapFile);
                        return false;
                    }

                    roomEntities[floorIndex][roomIndex][roomItemCount] = itemSymbol;
                    roomItemCount++;
                }
            }
        }
    }

    fclose(mapFile);

    // All five floors should be loaded. Every floor matters!
    for (int floorIndex = 0; floorIndex < NUM_FLOORS; floorIndex++) {

        if (!floorLoaded[floorIndex]) {
            return false;
        }
    }

    return true;
}


// Loads the starting game information from initial-state.txt.
bool loadInitialState(const char* filename,
                      int& playerFloor, int& playerRoom,
                      int& energy, int& time, int& score,
                      char inventory[MAX_INVENTORY_CAPACITY],
                      int& inventoryCount,
                      bool objectives[NUM_OBJECTIVES],
                      int& emergencyTripsLeft) {

    if (filename == nullptr) {
        return false;
    }

    FILE* stateFile = fopen(filename, "r");

    // The game needs its initial settings to get started.
    if (stateFile == nullptr) {
        return false;
    }

    // Set up the starting player position and resources.
    playerFloor = 1;
    playerRoom = 1;

    energy = INITIAL_ENERGY;
    time = INITIAL_TIME;
    score = INITIAL_SCORE;

    inventoryCount = 0;
    emergencyTripsLeft = 0;

    // Start with an empty inventory.
    for (int itemIndex = 0;
         itemIndex < MAX_INVENTORY_CAPACITY;
         itemIndex++) {

        inventory[itemIndex] = ENT_NONE;
    }

    // No objectives have been completed yet.
    for (int objectiveIndex = 0;
         objectiveIndex < NUM_OBJECTIVES;
         objectiveIndex++) {

        objectives[objectiveIndex] = false;
    }

    char settingLine[MAX_LINE_LEN];
    char settingName[MAX_NAME_LEN];
    char settingValue[MAX_NAME_LEN];

    // Read each setting from the file.
    while (fgets(settingLine, MAX_LINE_LEN, stateFile) != nullptr) {

        trimWhitespace(settingLine);

        // Skip empty lines. We only need actual settings.
        if (settingLine[0] == '\0') {
            continue;
        }

        // Remove the colon if the setting uses one.
        char* colonPosition = strchr(settingLine, ':');

        if (colonPosition != nullptr) {
            *colonPosition = ' ';
        }

        settingName[0] = '\0';
        settingValue[0] = '\0';

        // Separate the setting name from its value.
        if (sscanf(settingLine, "%31s %31s",
                   settingName, settingValue) != 2) {
            continue;
        }

        // Load the starting room if a specific one is provided.
        if (stringEquals(settingName, "START_ROOM")) {

            // PLAYER_SELECTS means the default starting room stays.
            if (settingValue[0] == 'F') {

                int selectedFloor;
                int selectedRoom;

                if (sscanf(settingValue, "F%d-R%d",
                           &selectedFloor, &selectedRoom) == 2) {

                    playerFloor = selectedFloor;
                    playerRoom = selectedRoom;
                }
            }
        }

        else if (stringEquals(settingName, "INITIAL_ENERGY")) {
            energy = atoi(settingValue);
        }

        else if (stringEquals(settingName, "INITIAL_TIME")) {
            time = atoi(settingValue);
        }

        else if (stringEquals(settingName, "INITIAL_SCORE")) {
            score = atoi(settingValue);
        }

        else if (stringEquals(settingName, "PASSENGER_RESCUED")) {
            objectives[OBJ_PASSENGER_RESCUED] =
                stringEquals(settingValue, "YES");
        }

        else if (stringEquals(settingName, "CAT_RESCUED")) {
            objectives[OBJ_CAT_SECURED] =
                stringEquals(settingValue, "YES");
        }

        else if (stringEquals(settingName, "CABINET_UNLOCKED")) {
            objectives[OBJ_CABINET_UNLOCKED] =
                stringEquals(settingValue, "YES");
        }

        else if (stringEquals(settingName, "TRAIN_KEY_COLLECTED")) {
            objectives[OBJ_TRAIN_KEY] =
                stringEquals(settingValue, "YES");
        }

        else if (stringEquals(settingName, "FUSE_INSTALLED")) {
            objectives[OBJ_FUSE_INSTALLED] =
                stringEquals(settingValue, "YES");
        }

        else if (stringEquals(settingName, "LIFT_PASS_COLLECTED")) {
            objectives[OBJ_LIFT_PASS] =
                stringEquals(settingValue, "YES");
        }

        else if (stringEquals(settingName, "TRAIN_PREPARED")) {
            objectives[OBJ_TRAIN_PREPARED] =
                stringEquals(settingValue, "YES");
        }

        // INITIAL_INVENTORY is NONE, so the inventory stays empty.
    }

    bool fileReadSuccessfully = !ferror(stateFile);

    fclose(stateFile);

    // Keep energy and time within the game's limits.
    energy = clampInt(energy, 0, MAX_ENERGY);
    time = clampInt(time, 0, MAX_TIME);

    if (score < 0) {
        score = 0;
    }

    // Make sure the starting position is actually inside the building.
    if (playerFloor < 1 || playerFloor > NUM_FLOORS ||
        playerRoom < 1 || playerRoom > ROOMS_PER_FLOOR) {
        return false;
    }

    return fileReadSuccessfully;
}


// Checks whether the selected starting room has a possible connection.
bool validateStartingRoom(int startFloor, int startRoom,
                          const char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1]) {

    // First, make sure the selected floor and room exist.
    if (startFloor < 1 || startFloor > NUM_FLOORS ||
        startRoom < 1 || startRoom > ROOMS_PER_FLOOR) {
        return false;
    }

    int floorIndex = startFloor - 1;
    bool hasConnection = false;

    // Check the room on the left, if there is one.
    if (startRoom > 1) {

        char leftConnection = horizontalConn[floorIndex][startRoom - 2];

        if (leftConnection == CONN_DOOR ||
            leftConnection == CONN_WINDOW ||
            leftConnection == CONN_LOCKED_DOOR) {

            hasConnection = true;
        }
    }

    // Check the room on the right, if there is one.
    if (startRoom < ROOMS_PER_FLOOR) {

        char rightConnection = horizontalConn[floorIndex][startRoom - 1];

        if (rightConnection == CONN_DOOR ||
            rightConnection == CONN_WINDOW ||
            rightConnection == CONN_LOCKED_DOOR) {

            hasConnection = true;
        }
    }

    // Room 1 has staircase access.
    if (startRoom == 1) {
        hasConnection = true;
    }

    // Rooms 2 and 3 have Lift 1 access on floors 1 and 2.
    if ((startRoom == 2 || startRoom == 3) &&
        (startFloor == 1 || startFloor == 2)) {

        hasConnection = true;
    }

    // Room 4 has Lift 2 access on floors 2, 4 and 5.
    if (startRoom == 4 &&
        (startFloor == 2 || startFloor == 4 || startFloor == 5)) {

        hasConnection = true;
    }

    return hasConnection;
}
