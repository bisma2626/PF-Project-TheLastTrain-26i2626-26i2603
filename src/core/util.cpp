
#include "util.h"
#include <iostream>

using namespace std;

// Keeps the value inside its allowed range. No going overboard!
int clampInt(int number, int minimum, int maximum) {
    if (number < minimum) {
        return minimum;
    }
    else if (number > maximum) {
        return maximum;
    }
    else {
        return number;
    }
}

// Removes extra whitespace from the beginning and end. Clean strings only!
void trimWhitespace(char* str) {
    if (str == nullptr) {
        return;
    }

    int firstChar = 0;

    // Skip all the extra whitespace at the beginning.
    while (str[firstChar] == ' ' || str[firstChar] == '\t' ||
           str[firstChar] == '\n' || str[firstChar] == '\r') {
        firstChar++;
    }

    // If there's nothing left, make the string empty.
    if (str[firstChar] == '\0') {
        str[0] = '\0';
        return;
    }

    int lastChar = firstChar;

    // Find where the actual text ends.
    while (str[lastChar] != '\0') {
        lastChar++;
    }

    lastChar--;

    // Remove extra whitespace from the end too.
    while (str[lastChar] == ' ' || str[lastChar] == '\t' ||
           str[lastChar] == '\n' || str[lastChar] == '\r') {
        lastChar--;
    }

    // Shift the clean text to the beginning. That's the final look!
    int newPosition = 0;

    while (firstChar <= lastChar) {
        str[newPosition] = str[firstChar];
        newPosition++;
        firstChar++;
    }

    str[newPosition] = '\0';
}

// Checks if two strings are exactly the same. No almost-matches!
bool stringEquals(const char* firstString, const char* secondString) {
    if (firstString == nullptr || secondString == nullptr) {
        return false;
    }

    int position = 0;

    while (firstString[position] != '\0' &&
           secondString[position] != '\0') {

        if (firstString[position] != secondString[position]) {
            return false;
        }

        position++;
    }

    // Both strings must finish at the same time.
    return firstString[position] == '\0' &&
           secondString[position] == '\0';
}

// Gives each section a clean heading.
void printHeader(const char* title) {
    if (title == nullptr) {
        title = "";
    }

    cout << "===========================================\n";
    cout << " " << title << "\n";
    cout << "===========================================\n";
}

// Gives the terminal a fresh start.
void clearScreen() {
    cout << "\033[2J\033[H";
}
