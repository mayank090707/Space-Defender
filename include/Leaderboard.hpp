#pragma once

#include <string>
#include <vector>
#include <chrono>

/**
 * @brief Represents an individual score entry on the galactic leaderboard.
 */
struct ScoreEntry {
    std::string name;
    int score;

    bool operator==(const ScoreEntry& other) const {
        return name == other.name && score == other.score;
    }
};

/**
 * @brief Sorting algorithms available for educational analysis.
 */
enum class SortAlgorithm {
    BubbleSort,
    SelectionSort,
    InsertionSort
};

/**
 * @brief Instrumentation metrics capturing execution statistics of sorting algorithms.
 */
struct AlgorithmMetrics {
    size_t comparisons = 0;
    size_t swapsOrMoves = 0;
    double durationMicroseconds = 0.0;
};

/**
 * @brief Manages score records, manual sorting algorithms, manual searching algorithms,
 *        and local file persistence for Space Defender.
 */
class Leaderboard {
public:
    static constexpr size_t MAX_NAME_LENGTH = 15;
    static constexpr size_t MAX_DISPLAY_ENTRIES = 10;
    static constexpr const char* DEFAULT_FILEPATH = "data/leaderboard.txt";

    Leaderboard();
    explicit Leaderboard(const std::string& filepath);
    ~Leaderboard() = default;

    // --- Validation ---
    static bool isValidName(const std::string& name);

    // --- Core Operations ---
    bool addEntry(const std::string& name, int score);
    [[nodiscard]] const std::vector<ScoreEntry>& getEntries() const;
    [[nodiscard]] size_t size() const;
    [[nodiscard]] bool empty() const;
    void clear();

    // --- Persistence ---
    bool loadFromFile(const std::string& filepath = DEFAULT_FILEPATH);
    bool saveToFile(const std::string& filepath = DEFAULT_FILEPATH) const;

    // --- Sorting Algorithms (Manual DSA Implementations) ---
    void bubbleSort(AlgorithmMetrics* metrics = nullptr);
    void selectionSort(AlgorithmMetrics* metrics = nullptr);
    void insertionSort(AlgorithmMetrics* metrics = nullptr);
    void sort(SortAlgorithm algo, AlgorithmMetrics* metrics = nullptr);

    // --- Searching Algorithms (Manual DSA Implementations) ---
    [[nodiscard]] std::vector<ScoreEntry> linearSearch(const std::string& targetName) const;
    [[nodiscard]] std::vector<ScoreEntry> binarySearch(int targetScore) const;

    // --- Educational Analysis & Metadata ---
    AlgorithmMetrics benchmarkSort(SortAlgorithm algo);
    [[nodiscard]] SortAlgorithm getCurrentAlgorithm() const;
    static std::string getAlgorithmName(SortAlgorithm algo);
    [[nodiscard]] bool isSorted() const;

    // Primary deterministic comparator: higher score first, tie-break by name ascending
    static bool hasHigherPriority(const ScoreEntry& a, const ScoreEntry& b);

private:
    std::vector<ScoreEntry> m_entries;
    std::string m_filepath;
    SortAlgorithm m_currentAlgorithm;
    bool m_isSorted;
};
