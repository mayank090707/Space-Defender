#include "Leaderboard.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>

Leaderboard::Leaderboard()
    : m_filepath(DEFAULT_FILEPATH)
    , m_currentAlgorithm(SortAlgorithm::InsertionSort)
    , m_isSorted(false)
{
    loadFromFile(m_filepath);
}

Leaderboard::Leaderboard(const std::string& filepath)
    : m_filepath(filepath)
    , m_currentAlgorithm(SortAlgorithm::InsertionSort)
    , m_isSorted(false)
{
    loadFromFile(m_filepath);
}

bool Leaderboard::isValidName(const std::string& name) {
    if (name.empty() || name.length() > MAX_NAME_LENGTH) {
        return false;
    }
    for (char c : name) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_') {
            return false;
        }
    }
    return true;
}

bool Leaderboard::addEntry(const std::string& name, int score) {
    if (!isValidName(name) || score < 0) {
        std::cerr << "[Leaderboard] Validation error: Invalid name or score." << std::endl;
        return false;
    }

    m_entries.push_back({name, score});
    m_isSorted = false;

    // Keep sorted by default using the active algorithm
    sort(m_currentAlgorithm);
    return true;
}

const std::vector<ScoreEntry>& Leaderboard::getEntries() const {
    return m_entries;
}

size_t Leaderboard::size() const {
    return m_entries.size();
}

bool Leaderboard::empty() const {
    return m_entries.empty();
}

void Leaderboard::clear() {
    m_entries.clear();
    m_isSorted = true;
}

bool Leaderboard::loadFromFile(const std::string& filepath) {
    m_filepath = filepath;
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cout << "[Leaderboard] Notice: No existing file found at '" << filepath
                  << "'. Initializing empty leaderboard." << std::endl;
        m_isSorted = true;
        return false;
    }

    m_entries.clear();
    std::string line;
    size_t lineNum = 0;

    while (std::getline(file, line)) {
        lineNum++;
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string name;
        int score = 0;

        if (iss >> name >> score) {
            if (isValidName(name) && score >= 0) {
                m_entries.push_back({name, score});
            } else {
                std::cerr << "[Leaderboard] Warning: Malformed record on line "
                          << lineNum << " skipped." << std::endl;
            }
        } else {
            std::cerr << "[Leaderboard] Warning: Unparseable line "
                      << lineNum << " skipped." << std::endl;
        }
    }

    m_isSorted = false;
    sort(m_currentAlgorithm);
    std::cout << "[Leaderboard] Successfully loaded " << m_entries.size()
              << " entries from '" << filepath << "'." << std::endl;
    return true;
}

bool Leaderboard::saveToFile(const std::string& filepath) const {
    std::ofstream file(filepath, std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "[Leaderboard] Error: Failed to open file for writing at '"
                  << filepath << "'." << std::endl;
        return false;
    }

    for (const auto& entry : m_entries) {
        file << entry.name << " " << entry.score << "\n";
    }

    if (!file.good()) {
        std::cerr << "[Leaderboard] Error: Failed to complete write to '"
                  << filepath << "'." << std::endl;
        return false;
    }

    std::cout << "[Leaderboard] Saved " << m_entries.size()
              << " entries to '" << filepath << "'." << std::endl;
    return true;
}

bool Leaderboard::hasHigherPriority(const ScoreEntry& a, const ScoreEntry& b) {
    if (a.score != b.score) {
        return a.score > b.score; // Primary: Higher score first (Descending)
    }
    return a.name < b.name;       // Secondary: Alphabetical ascending (A-Z)
}

void Leaderboard::bubbleSort(AlgorithmMetrics* metrics) {
    auto startTime = std::chrono::high_resolution_clock::now();
    size_t comparisons = 0;
    size_t swaps = 0;

    size_t n = m_entries.size();
    for (size_t i = 0; i < n; ++i) {
        bool swapped = false;
        for (size_t j = 0; j + 1 < n - i; ++j) {
            comparisons++;
            // If the right element has higher priority than the left element, swap
            if (hasHigherPriority(m_entries[j + 1], m_entries[j])) {
                std::swap(m_entries[j], m_entries[j + 1]);
                swaps++;
                swapped = true;
            }
        }
        // Early-exit optimization when no swaps occurred
        if (!swapped) {
            break;
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    if (metrics) {
        metrics->comparisons = comparisons;
        metrics->swapsOrMoves = swaps;
        metrics->durationMicroseconds = std::chrono::duration<double, std::micro>(endTime - startTime).count();
    }
    m_currentAlgorithm = SortAlgorithm::BubbleSort;
    m_isSorted = true;
}

void Leaderboard::selectionSort(AlgorithmMetrics* metrics) {
    auto startTime = std::chrono::high_resolution_clock::now();
    size_t comparisons = 0;
    size_t moves = 0;

    size_t n = m_entries.size();
    for (size_t i = 0; i < n; ++i) {
        size_t bestIdx = i;
        for (size_t j = i + 1; j < n; ++j) {
            comparisons++;
            if (hasHigherPriority(m_entries[j], m_entries[bestIdx])) {
                bestIdx = j;
            }
        }

        // Stable selection: shift elements rightward to insert bestIdx into position i
        if (bestIdx != i) {
            ScoreEntry bestEntry = std::move(m_entries[bestIdx]);
            for (size_t k = bestIdx; k > i; --k) {
                m_entries[k] = std::move(m_entries[k - 1]);
                moves++;
            }
            m_entries[i] = std::move(bestEntry);
            moves++;
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    if (metrics) {
        metrics->comparisons = comparisons;
        metrics->swapsOrMoves = moves;
        metrics->durationMicroseconds = std::chrono::duration<double, std::micro>(endTime - startTime).count();
    }
    m_currentAlgorithm = SortAlgorithm::SelectionSort;
    m_isSorted = true;
}

void Leaderboard::insertionSort(AlgorithmMetrics* metrics) {
    auto startTime = std::chrono::high_resolution_clock::now();
    size_t comparisons = 0;
    size_t shifts = 0;

    size_t n = m_entries.size();
    for (size_t i = 1; i < n; ++i) {
        ScoreEntry key = std::move(m_entries[i]);
        size_t j = i;

        while (j > 0) {
            comparisons++;
            if (hasHigherPriority(key, m_entries[j - 1])) {
                m_entries[j] = std::move(m_entries[j - 1]);
                shifts++;
                j--;
            } else {
                break;
            }
        }
        m_entries[j] = std::move(key);
        shifts++;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    if (metrics) {
        metrics->comparisons = comparisons;
        metrics->swapsOrMoves = shifts;
        metrics->durationMicroseconds = std::chrono::duration<double, std::micro>(endTime - startTime).count();
    }
    m_currentAlgorithm = SortAlgorithm::InsertionSort;
    m_isSorted = true;
}

void Leaderboard::sort(SortAlgorithm algo, AlgorithmMetrics* metrics) {
    switch (algo) {
        case SortAlgorithm::BubbleSort:
            bubbleSort(metrics);
            break;
        case SortAlgorithm::SelectionSort:
            selectionSort(metrics);
            break;
        case SortAlgorithm::InsertionSort:
            insertionSort(metrics);
            break;
    }
}

AlgorithmMetrics Leaderboard::benchmarkSort(SortAlgorithm algo) {
    // Benchmark on an independent duplicate copy to avoid side-effects
    Leaderboard clone = *this;
    AlgorithmMetrics metrics;
    clone.sort(algo, &metrics);
    return metrics;
}

std::vector<ScoreEntry> Leaderboard::linearSearch(const std::string& targetName) const {
    std::vector<ScoreEntry> matches;
    auto toLowerStr = [](std::string s) {
        for (char& c : s) {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        return s;
    };

    std::string query = toLowerStr(targetName);
    for (const auto& entry : m_entries) {
        if (toLowerStr(entry.name) == query) {
            matches.push_back(entry);
        }
    }
    return matches;
}

std::vector<ScoreEntry> Leaderboard::binarySearch(int targetScore) const {
    std::vector<ScoreEntry> matches;
    if (m_entries.empty() || !m_isSorted) {
        return matches;
    }

    int low = 0;
    int high = static_cast<int>(m_entries.size()) - 1;
    int firstIdx = -1;

    // 1. Lower Bound: Find the first occurrence in descending array
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (m_entries[mid].score == targetScore) {
            firstIdx = mid;
            high = mid - 1; // Continue searching left
        } else if (m_entries[mid].score < targetScore) {
            // Smaller scores reside on the right in descending array
            high = mid - 1;
        } else {
            low = mid + 1;
        }
    }

    if (firstIdx == -1) {
        return matches; // Score not found
    }

    // 2. Upper Bound: Find the last occurrence in descending array
    low = 0;
    high = static_cast<int>(m_entries.size()) - 1;
    int lastIdx = -1;

    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (m_entries[mid].score == targetScore) {
            lastIdx = mid;
            low = mid + 1; // Continue searching right
        } else if (m_entries[mid].score < targetScore) {
            high = mid - 1;
        } else {
            low = mid + 1;
        }
    }

    // 3. Collect all matching records
    for (int i = firstIdx; i <= lastIdx; ++i) {
        matches.push_back(m_entries[i]);
    }
    return matches;
}

SortAlgorithm Leaderboard::getCurrentAlgorithm() const {
    return m_currentAlgorithm;
}

std::string Leaderboard::getAlgorithmName(SortAlgorithm algo) {
    switch (algo) {
        case SortAlgorithm::BubbleSort: return "Bubble Sort";
        case SortAlgorithm::SelectionSort: return "Selection Sort";
        case SortAlgorithm::InsertionSort: return "Insertion Sort";
    }
    return "Unknown";
}

bool Leaderboard::isSorted() const {
    return m_isSorted;
}
