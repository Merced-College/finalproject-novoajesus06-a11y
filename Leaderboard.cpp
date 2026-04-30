/*
 * Author: [Your Name]
 * Class: Leaderboard
 *
 * Description:
 * Loads, maintains, and saves the top-10 score list.
 * Uses Insertion Sort to keep entries in descending score order.
 */

#include "Leaderboard.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>

Leaderboard::Leaderboard(const std::string& file) : filename(file) {
    loadFromFile();
}

void Leaderboard::addScore(const std::string& name, int score, int level) {
    entries.push_back({name, score, level});
    insertionSort();

    // Keep only top MAX_ENTRIES
    if ((int)entries.size() > MAX_ENTRIES)
        entries.resize(MAX_ENTRIES);

    saveToFile();
}

/*
 * Algorithm: Insertion Sort (descending by score)
 * Time Complexity: O(n) per call after one new insertion.
 */
void Leaderboard::insertionSort() {
    int n = (int)entries.size();
    for (int i = n - 1; i > 0; --i) {
        if (entries[i].score > entries[i - 1].score)
            std::swap(entries[i], entries[i - 1]);
        else
            break;   // already in place
    }
}

void Leaderboard::display() const {
    std::cout << "\n====== TOP SCORES ======\n";
    std::cout << std::left
              << std::setw(4)  << "#"
              << std::setw(14) << "Name"
              << std::setw(8)  << "Score"
              << "Level\n";
    std::cout << "------------------------\n";

    int rank = 1;
    for (const auto& e : entries) {
        std::cout << std::left
                  << std::setw(4)  << rank++
                  << std::setw(14) << e.name
                  << std::setw(8)  << e.score
                  << e.level << "\n";
    }
    std::cout << "========================\n";
}

const std::vector<ScoreEntry>& Leaderboard::getEntries() const { return entries; }

void Leaderboard::loadFromFile() {
    std::ifstream file(filename);
    if (!file.is_open()) return;

    ScoreEntry entry;
    while (file >> entry.name >> entry.score >> entry.level)
        entries.push_back(entry);

    insertionSort();
}

void Leaderboard::saveToFile() const {
    std::ofstream file(filename);
    if (!file.is_open()) return;

    for (const auto& e : entries)
        file << e.name << " " << e.score << " " << e.level << "\n";
}
