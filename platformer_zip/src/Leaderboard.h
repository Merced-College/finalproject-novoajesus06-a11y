#pragma once
#include <string>
#include <vector>

/*
 * Author: [Jesus Novoa vasquez]
 * Class: Leaderboard
 *
 * Description:
 * Stores the top scores using a vector of ScoreEntry structs.
 * Implements Insertion Sort to maintain sorted order after each new entry.
 * Persists scores to a text file.
 *
 * Data Structure: std::vector<ScoreEntry> — dynamic array for scores
 */

struct ScoreEntry {
    std::string name;
    int         score;
    int         level;   // highest level reached
};

class Leaderboard {
public:
    Leaderboard(const std::string& filename = "scores.txt");

    void addScore(const std::string& name, int score, int level);
    void display() const;

    const std::vector<ScoreEntry>& getEntries() const;

private:
    /*
     * Algorithm: Insertion Sort
     *
     * Keeps the scores vector sorted in descending order by score.
     * After inserting a new entry at the end, walk it left until
     * it is in the correct position.
     *
     * Steps:
     * 1. Append the new entry to the back of the vector.
     * 2. Start at the last index (i = size - 1).
     * 3. While i > 0 AND entry[i].score > entry[i-1].score:
     *    a. Swap entry[i] with entry[i-1]
     *    b. Decrement i
     * 4. The vector is now sorted descending.
     *
     * Time Complexity: O(n) average per insert, O(n^2) for n inserts
     */
    void insertionSort();

    void loadFromFile();
    void saveToFile() const;

    std::string            filename;
    std::vector<ScoreEntry> entries;
    static const int       MAX_ENTRIES = 10;
};
