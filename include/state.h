
#ifndef STATE_H
#define STATE_H

#include "config.h"

// Connection types
const char CONN_NONE = ' ';
const char CONN_DOOR = 'D';
const char CONN_WINDOW = 'W';
const char CONN_LOCKED_DOOR = 'L';
const char CONN_SOLID_WALL = '#';
const char CONN_STAIRS = 'S';
const char CONN_LIFT1 = '1';
const char CONN_LIFT2 = '2';

// Empty room
const char ENT_NONE = ' ';

// Item symbols
const char ENT_MASTER_KEY = '<';
const char ENT_ENERGY_PACK = ')';
const char ENT_EMERGENCY_TOOLKIT = '\\';
const char ENT_CLUE_4 = '4';
const char ENT_FUSE = '*';
const char ENT_CAT = '~';
const char ENT_CONTROL_PANEL = ';';
const char ENT_LIFT_PASS = '(';
const char ENT_PASSENGER = '!';
const char ENT_MEDICAL_KIT = '%';
const char ENT_EMERGENCY_CARD = '$';
const char ENT_CLUE_2 = '2';
const char ENT_CLUE_7 = '7';
const char ENT_CABINET_TRAIN_KEY = '^';
const char ENT_CAT_CARRIER = '@';
const char ENT_OBSERVATION_TOKEN = '&';

// Objective indexes
const int OBJ_MASTER_KEY = 0;
const int OBJ_CLUE_4 = 1;
const int OBJ_CLUE_2 = 2;
const int OBJ_CLUE_7 = 3;
const int OBJ_CABINET_UNLOCKED = 4;
const int OBJ_TRAIN_KEY = 5;
const int OBJ_FUSE_COLLECTED = 6;
const int OBJ_FUSE_INSTALLED = 7;
const int OBJ_LIFT_PASS = 8;
const int OBJ_LIFT2_ACTIVE = 9;
const int OBJ_PASSENGER_RESCUED = 10;
const int OBJ_CAT_CARRIER = 11;
const int OBJ_CAT_SECURED = 12;
const int OBJ_TRAIN_PREPARED = 13;
const int OBJ_MISSION_WON = 14;
const int OBJ_MISSION_FAILED = 15;

const int NUM_OBJECTIVES = 16;

#endif
