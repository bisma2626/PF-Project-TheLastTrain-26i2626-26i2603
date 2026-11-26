
#include "simulator.h"
#include "level.h"
#include "physics.h"
#include "switches.h"
#include "logger.h"
#include "viewer_iface.h"
#include "util.h"

#include <iostream>
#include <iomanip>
#include <cctype>
#include <limits>

using namespace std;


// Takes valid number input without crashing the program.
static bool readInt(int& value)
{
    if (cin >> value)
    {
        return true;
    }

    if (!cin.eof())
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    return false;
}


// ---------------------- MOVE ----------------------

bool handleMoveCommand(
    int targetFloor, int targetRoom,
    int& currentFloor, int& currentRoom,
    int& energy, int& time,
    const char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
    const char inventory[MAX_INVENTORY_CAPACITY],
    int inventoryCount,
    int& emergencyTripsLeft,
    bool lift2Operational)
{
    char connType = CONN_NONE;

    // First check if these rooms are actually connected.
    if (!isDirectlyConnected(currentFloor, currentRoom,
                             targetFloor, targetRoom,
                             horizontalConn, connType))
    {
        cout << "You cannot move there from here.\n";
        return false;
    }

    // Check if the connection is unlocked and usable.
    if (!canTraverse(targetFloor, targetRoom, connType,
                     inventory, inventoryCount,
                     emergencyTripsLeft, lift2Operational))
    {
        cout << "That way is blocked (locked door or Lift 2 not working).\n";
        return false;
    }

    // Different paths have different energy and time costs.
    if (connType == CONN_WINDOW)
    {
        applyResourceDelta(energy, time, COST_MOVE_WINDOW_ENERGY, COST_MOVE_WINDOW_TIME);
    }
    else if (connType == CONN_LIFT1 || connType == CONN_LIFT2)
    {
        applyResourceDelta(energy, time, COST_MOVE_LIFT_ENERGY, COST_MOVE_LIFT_TIME);
    }
    else
    {
        applyResourceDelta(energy, time, COST_MOVE_ORDINARY_ENERGY, COST_MOVE_ORDINARY_TIME);
    }

    // Emergency trips are used only when Lift 2 is not repaired.
    if (connType == CONN_LIFT2 && !lift2Operational)
    {
        emergencyTripsLeft--;
    }

    // Update the player's location.
    currentFloor = targetFloor;
    currentRoom = targetRoom;

    cout << "You moved to Floor " << currentFloor << ", Room " << currentRoom << ".\n";
    return true;
}


// --------------------- INSPECT ---------------------

bool handleInspectCommand(
    int currentFloor, int currentRoom,
    int& energy, int& time,
    const char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
    int entityCount,
    bool objectives[NUM_OBJECTIVES],
    int& score)
{
    applyResourceDelta(energy, time, COST_INSPECT_ENERGY, COST_INSPECT_TIME);

    cout << "Inspecting Floor " << currentFloor << ", Room " << currentRoom << "...\n";

    // Check if this room has any puzzle clues.
    for (int i = 0; i < entityCount && i < MAX_ENTITIES_PER_ROOM; i++)
    {
        char symbol = roomEntities[currentFloor - 1][currentRoom - 1][i];

        if (symbol == ENT_CLUE_4 && !objectives[OBJ_CLUE_4])
        {
            objectives[OBJ_CLUE_4] = true;
            applyScoreDelta(score, SCORE_CLUE_FOUND);
            cout << "Clue 4 found!\n";
        }

        if (symbol == ENT_CLUE_2 && !objectives[OBJ_CLUE_2])
        {
            objectives[OBJ_CLUE_2] = true;
            applyScoreDelta(score, SCORE_CLUE_FOUND);
            cout << "Clue 2 found!\n";
        }

        if (symbol == ENT_CLUE_7 && !objectives[OBJ_CLUE_7])
        {
            objectives[OBJ_CLUE_7] = true;
            applyScoreDelta(score, SCORE_CLUE_FOUND);
            cout << "Clue 7 found!\n";
        }
    }

    return true;
}


// --------------------- COLLECT ---------------------

bool handleCollectCommand(
    char itemSymbol,
    int currentFloor, int currentRoom,
    int& energy, int& time, int& score,
    char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
    int& entityCount,
    char inventory[MAX_INVENTORY_CAPACITY],
    int& inventoryCount,
    bool objectives[NUM_OBJECTIVES])
{
    int floorIndex = currentFloor - 1;
    int roomIndex = currentRoom - 1;

    // Some things are clues or characters, not inventory items.
    if (itemSymbol == ENT_CLUE_4 || itemSymbol == ENT_CLUE_2 ||
        itemSymbol == ENT_CLUE_7 || itemSymbol == ENT_CAT ||
        itemSymbol == ENT_PASSENGER || itemSymbol == ENT_CONTROL_PANEL)
    {
        cout << "You cannot pick that up.\n";
        return false;
    }

    // Find the requested item in the current room.
    int itemIndex = -1;

    for (int i = 0; i < entityCount && i < MAX_ENTITIES_PER_ROOM; i++)
    {
        if (roomEntities[floorIndex][roomIndex][i] == itemSymbol)
        {
            itemIndex = i;
            break;
        }
    }

    if (itemIndex == -1)
    {
        cout << "That item is not in this room.\n";
        return false;
    }

    // The Train Key stays unavailable until the cabinet is unlocked.
    if (itemSymbol == ENT_CABINET_TRAIN_KEY && !objectives[OBJ_CABINET_UNLOCKED])
    {
        cout << "The cabinet is locked.\n";
        return false;
    }

    // These items give their bonus immediately.
    bool instantItem = (itemSymbol == ENT_ENERGY_PACK ||
                        itemSymbol == ENT_EMERGENCY_TOOLKIT ||
                        itemSymbol == ENT_OBSERVATION_TOKEN);

    if (!instantItem && !addItem(inventory, inventoryCount, itemSymbol))
    {
        cout << "Your inventory is full.\n";
        return false;
    }

    // Remove the item and shift the remaining items forward.
    for (int i = itemIndex; i < entityCount - 1; i++)
    {
        roomEntities[floorIndex][roomIndex][i] =
            roomEntities[floorIndex][roomIndex][i + 1];
    }

    entityCount--;
    roomEntities[floorIndex][roomIndex][entityCount] = ENT_NONE;

    applyResourceDelta(energy, time, COST_ITEM_PICKUP_ENERGY, COST_ITEM_PICKUP_TIME);

    cout << "Item collected: " << getEntityName(itemSymbol) << "\n";

    // Give bonuses and update the matching objectives.
    if (itemSymbol == ENT_ENERGY_PACK)
    {
        applyResourceDelta(energy, time, BONUS_ENERGY_PACK, 0);
    }
    else if (itemSymbol == ENT_EMERGENCY_TOOLKIT)
    {
        applyResourceDelta(energy, time, BONUS_EMERGENCY_TOOLKIT, 0);
    }
    else if (itemSymbol == ENT_OBSERVATION_TOKEN)
    {
        applyResourceDelta(energy, time, 0, BONUS_OBSERVATION_TOKEN);
    }
    else if (itemSymbol == ENT_MASTER_KEY)
    {
        objectives[OBJ_MASTER_KEY] = true;
        applyScoreDelta(score, SCORE_KEY_COLLECT);
    }
    else if (itemSymbol == ENT_FUSE)
    {
        objectives[OBJ_FUSE_COLLECTED] = true;
    }
    else if (itemSymbol == ENT_LIFT_PASS)
    {
        objectives[OBJ_LIFT_PASS] = true;
        applyScoreDelta(score, SCORE_LIFT_PASS);

        // Lift 2 becomes active once the fuse is installed too.
        if (objectives[OBJ_FUSE_INSTALLED])
        {
            objectives[OBJ_LIFT2_ACTIVE] = true;
        }
    }
    else if (itemSymbol == ENT_CAT_CARRIER)
    {
        objectives[OBJ_CAT_CARRIER] = true;
    }
    else if (itemSymbol == ENT_CABINET_TRAIN_KEY)
    {
        objectives[OBJ_TRAIN_KEY] = true;
        applyScoreDelta(score, SCORE_TRAIN_KEY);
    }

    return true;
}


// ----------------------- USE -----------------------

bool handleUseCommand(
    char itemSymbol,
    int currentFloor, int currentRoom,
    int& energy, int& time, int& score,
    char inventory[MAX_INVENTORY_CAPACITY],
    int& inventoryCount,
    bool objectives[NUM_OBJECTIVES])
{
    if (!hasItem(inventory, inventoryCount, itemSymbol))
    {
        cout << "You do not have this item.\n";
        return false;
    }

    if (itemSymbol == ENT_FUSE)
    {
        // The fuse belongs in the control panel on Floor 3, Room 4.
        if (currentFloor != 3 || currentRoom != 4 ||
            !installFuseControlPanel(inventory, inventoryCount, objectives, score))
        {
            cout << "Use the fuse at the control panel (Floor 3, Room 4).\n";
            return false;
        }

        cout << "Fuse installed!\n";
    }
    else if (itemSymbol == ENT_MEDICAL_KIT)
    {
        // The medical kit is used to rescue the passenger.
        if (currentFloor != 4 || currentRoom != 1 ||
            objectives[OBJ_PASSENGER_RESCUED])
        {
            cout << "Use the medical kit on the passenger (Floor 4, Room 1).\n";
            return false;
        }

        objectives[OBJ_PASSENGER_RESCUED] = true;
        applyScoreDelta(score, SCORE_RESCUE_PASSENGER);
        removeItem(inventory, inventoryCount, itemSymbol);

        cout << "Passenger rescued!\n";
    }
    else if (itemSymbol == ENT_CAT_CARRIER)
    {
        // Take the carrier to the cat on Floor 3, Room 3.
        if (currentFloor != 3 || currentRoom != 3 ||
            objectives[OBJ_CAT_SECURED])
        {
            cout << "Use the carrier on the cat (Floor 3, Room 3).\n";
            return false;
        }

        objectives[OBJ_CAT_SECURED] = true;
        applyScoreDelta(score, SCORE_SECURE_CAT);
        removeItem(inventory, inventoryCount, itemSymbol);

        cout << "The cat is safe!\n";
    }
    else if (itemSymbol == ENT_CABINET_TRAIN_KEY)
    {
        // Bring the Train Key to the train on Floor 5, Room 2.
        if (currentFloor != 5 || currentRoom != 2 ||
            !objectives[OBJ_PASSENGER_RESCUED] ||
            !objectives[OBJ_CAT_SECURED])
        {
            cout << "Rescue the passenger and the cat, then use the key at Floor 5, Room 2.\n";
            return false;
        }

        objectives[OBJ_TRAIN_PREPARED] = true;
        applyScoreDelta(score, SCORE_PREPARE_TRAIN);
        removeItem(inventory, inventoryCount, itemSymbol);

        cout << "The train is prepared and boarded!\n";
    }
    else
    {
        cout << "This item cannot be used here.\n";
        return false;
    }

    // Every successful item use costs energy and time.
    applyResourceDelta(energy, time, COST_ITEM_USE_ENERGY, COST_ITEM_USE_TIME);
    return true;
}


// -------------------- UNLOCK CABINET --------------------

bool handleUnlockCabinetCommand(
    const char* code,
    int currentFloor, int currentRoom,
    bool objectives[NUM_OBJECTIVES],
    int& score,
    char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
    int& entityCount)
{
    // The Train Key is already listed in the map file.
    (void)roomEntities;
    (void)entityCount;

    if (currentFloor != 5 || currentRoom != 1)
    {
        cout << "The cabinet is on Floor 5, Room 1.\n";
        return false;
    }

    if (objectives[OBJ_CABINET_UNLOCKED] ||
        !verifyCabinetCode(code, objectives))
    {
        cout << "Already open, wrong code, or clues missing.\n";
        return false;
    }

    objectives[OBJ_CABINET_UNLOCKED] = true;
    applyScoreDelta(score, SCORE_CABINET_UNLOCK);

    cout << "Cabinet unlocked! You can now collect the Train Key (^).\n";
    return true;
}


// -------------------- END CONDITIONS --------------------

bool checkEndConditions(
    int energy, int time,
    const bool objectives[NUM_OBJECTIVES])
{
    return objectives[OBJ_MISSION_WON] ||
           objectives[OBJ_MISSION_FAILED] ||
           energy <= 0 || time <= 0;
}


// ------------------- MAIN SIMULATION -------------------

void runSimulation(
    int& playerFloor, int& playerRoom,
    int& energy, int& time, int& score,
    char inventory[MAX_INVENTORY_CAPACITY],
    int& inventoryCount,
    bool objectives[NUM_OBJECTIVES],
    int& emergencyTripsLeft,
    char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
    char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
    int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR])
{
    bool gameRunning = true;

    char command = ' ';
    char itemSymbol = ' ';
    char confirm = ' ';
    char cabinetCode[20];

    int targetFloor = 0;
    int targetRoom = 0;
    int startFloor = 0;
    int startRoom = 0;

    // Pick where your journey starts.
    cout << "Choose your starting floor and room (for example: 1 1): ";

    while (!(readInt(startFloor) && readInt(startRoom) &&
             validateStartingRoom(startFloor, startRoom, horizontalConn)))
    {
        if (cin.eof())
        {
            return;
        }

        cout << "Not a valid starting room. Try again: ";
    }

    playerFloor = startFloor;
    playerRoom = startRoom;

    while (gameRunning)
    {
        // Check if all mission goals are completed.
        if (objectives[OBJ_PASSENGER_RESCUED] &&
            objectives[OBJ_CAT_SECURED] &&
            objectives[OBJ_TRAIN_PREPARED])
        {
            objectives[OBJ_MISSION_WON] = true;
            applyScoreDelta(score, SCORE_MISSION_VICTORY);
        }
        else if (energy <= 0 || time <= 0)
        {
            objectives[OBJ_MISSION_FAILED] = true;
        }

        if (checkEndConditions(energy, time, objectives))
        {
            break;
        }

        // Show the current game status and building map.
        renderDashboard(playerFloor, playerRoom, energy, time, score,
                        inventory, inventoryCount, emergencyTripsLeft);

        renderAsciiBuilding(playerFloor, playerRoom);
        renderObjectivesChecklist(objectives);

        cout << "\nM-Move  I-Inspect  C-Collect  U-Use  K-Unlock cabinet  S-Save  L-Load  Q-Quit\n";
        cout << "Enter command: ";

        if (!(cin >> command))
        {
            cout << "\nInput ended. Exiting the game.\n";
            break;
        }

        command = static_cast<char>(
            tolower(static_cast<unsigned char>(command))
        );

        if (command == 'm')
        {
            cout << "Enter destination floor and room: ";

            if (readInt(targetFloor) && readInt(targetRoom))
            {
                handleMoveCommand(targetFloor, targetRoom,
                                  playerFloor, playerRoom,
                                  energy, time, horizontalConn,
                                  inventory, inventoryCount,
                                  emergencyTripsLeft,
                                  objectives[OBJ_LIFT2_ACTIVE]);
            }
            else
            {
                cout << "Please enter numbers.\n";
            }
        }
        else if (command == 'i')
        {
            handleInspectCommand(playerFloor, playerRoom, energy, time,
                                 roomEntities,
                                 entityCounts[playerFloor - 1][playerRoom - 1],
                                 objectives, score);

            renderRoomDetails(playerFloor, playerRoom, roomEntities,
                              entityCounts[playerFloor - 1][playerRoom - 1]);
        }
        else if (command == 'c')
        {
            cout << "Enter item symbol to collect: ";

            if (cin >> itemSymbol &&
                handleCollectCommand(itemSymbol, playerFloor, playerRoom,
                                     energy, time, score, roomEntities,
                                     entityCounts[playerFloor - 1][playerRoom - 1],
                                     inventory, inventoryCount, objectives) &&
                itemSymbol == ENT_EMERGENCY_CARD)
            {
                // The card gives you three emergency Lift 2 trips.
                emergencyTripsLeft = 3;
                cout << "You can make 3 emergency trips on Lift 2.\n";
            }
        }
        else if (command == 'u')
        {
            cout << "Enter item symbol to use: ";

            if (cin >> itemSymbol)
            {
                handleUseCommand(itemSymbol, playerFloor, playerRoom,
                                 energy, time, score,
                                 inventory, inventoryCount, objectives);
            }
        }
        else if (command == 'k')
        {
            cout << "Enter cabinet code: ";

            if (cin >> setw(20) >> cabinetCode &&
                handleUnlockCabinetCommand(cabinetCode, playerFloor, playerRoom,
                                           objectives, score,
                                           roomEntities, entityCounts[4][0]))
            {
                // Solving the cabinet puzzle also uses resources.
                applyResourceDelta(energy, time,
                                   COST_ITEM_USE_ENERGY, COST_ITEM_USE_TIME);
            }
        }
        else if (command == 's')
        {
            if (saveGameStateToFile("out/savegame.txt",
                                    playerFloor, playerRoom, energy, time, score,
                                    inventory, inventoryCount, objectives,
                                    emergencyTripsLeft, roomEntities, entityCounts))
            {
                cout << "Game saved.\n";
            }
            else
            {
                cout << "Could not save the game.\n";
            }
        }
        else if (command == 'l')
        {
            if (loadGameStateFromFile("out/savegame.txt",
                                      playerFloor, playerRoom, energy, time, score,
                                      inventory, inventoryCount, objectives,
                                      emergencyTripsLeft, roomEntities, entityCounts))
            {
                cout << "Game loaded.\n";
            }
            else
            {
                cout << "No saved game could be loaded.\n";
            }
        }
        else if (command == 'q')
        {
            cout << "Are you sure you want to quit? (y/n): ";

            if (cin >> confirm &&
                tolower(static_cast<unsigned char>(confirm)) == 'y')
            {
                gameRunning = false;
            }
        }
        else
        {
            cout << "Invalid command. Try again.\n";
        }
    }

    // Show the final result.
    if (objectives[OBJ_MISSION_WON])
    {
        cout << "\nMISSION COMPLETE! You saved the passenger and the cat.\n";
    }
    else if (objectives[OBJ_MISSION_FAILED])
    {
        cout << "\nMISSION FAILED. You ran out of energy or time.\n";
    }
    else
    {
        cout << "\nGAME EXITED.\n";
    }

    cout << "Final score: " << score << "\n";
    cout << "Remaining energy: " << energy << "\n";
    cout << "Remaining time: " << time << "\n";

    writeGameSummaryReport("out/game_summary.txt", score, energy, time,
                           objectives, objectives[OBJ_MISSION_WON]);
}
