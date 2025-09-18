/******************************************************************************
 *
 * \file Lexicon.cpp
 *
 * \brief Implements the Lexicon system for word management, including a Trie-based
 *        word storage, prefix handling, and NSFW filtering.
 *
 * * The Lexicon system loads words into a Trie, allowing fast lookups and prefix-based
 *   word generation.
 * * It integrates with the GlobalAssetManager to retrieve dictionary, prefix, and
 *   NSFW word lists.
 * * The system also provides utility functions for word validation, prefix checking,
 *   and random prefix generation.
 *
 * \author Keegan Lim, Dylan, Edwin
 * \copyright 2024, Digipen Institute of Technology
 *
 ******************************************************************************/
#include "pch.h"
#include "lexicon.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "AssetManager.h"

namespace Framework {

    // Static singleton instance initialization
    std::unique_ptr<Framework::Lexicon> Framework::Lexicon::instance = nullptr;
    const std::vector<std::string>& prefixes = GlobalAssetManager.GetPrefixAssets();

    std::string trim(const std::string& str) {
        // Find the first non-space character
        auto start = std::find_if_not(str.begin(), str.end(), [](unsigned char c) {
            return std::isspace(c);
            });

        // If the string is all spaces, return an empty string
        if (start == str.end()) {
            return "";
        }

        // Find the last non-space character
        auto end = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char c) {
            return std::isspace(c);
            }).base();

            // Return the trimmed substring
            return std::string(start, end);
    }


    // Trie constructor
    Trie::Trie() {
        root = new TrieNode();
    }

    void Trie::insert(const std::string& word) {
        TrieNode* node = root;

        for (char c : word) {
            if (node->children.find(c) == node->children.end()) {
                node->children[c] = new TrieNode();
            }
            node = node->children[c];
        }

        node->isEndOfWord = true;
        words.insert(word); // 
    }

    void Trie::clear(TrieNode* node)
    {
        if (!node) return;

        // Recursively delete all child nodes
        for (auto& pair : node->children)
        {
            clear(pair.second);
        }

        delete node;  // Delete current node
    }

   

    bool Trie::search(const std::string& word) {
        TrieNode* node = root;
        for (char c : word) {
            if (node->children.find(c) == node->children.end()) {
                // The character is not found
                std::cout << "Search failed at character: " << c << " in word: " << word << std::endl;
                return false;
            }
            node = node->children[c];  // Move to the next node
            std::cout << "Found character: " << c << " in word: " << word << std::endl; // Debug
        }

        // Check if the last node marks the end of a word
        if (node->isEndOfWord) {
            std::cout << "Search successful: Word found in Trie: " << word << std::endl;
            return true;
        }
        else {
            std::cout << "Search unsuccessful: Word is a prefix but not a complete word: " << word << std::endl;
            return false;
        }
    }


    // Check if any word in the Trie starts with the given prefix
    bool Trie::startsWith(const std::string& prefix) {
        TrieNode* node = root;
        for (char c : prefix) {
            if (node->children.find(c) == node->children.end()) {
                return false;
            }
            node = node->children[c];
        }
        return true;
    }

    // Private constructor
    Lexicon::Lexicon(const std::string& wordFilename, const std::string& prefixFilename, const std::string& nsfwfilename) {
        //readWordsFromFile(wordFilename);   // Load words into the Trie
        //readPrefixesFromFile(prefixFilename); // Load prefixes into the vector
        //readNsfwWordsFromFile(nsfwfilename);
        (void)wordFilename;
        (void)prefixFilename;
        (void)nsfwfilename;
        srand(static_cast<unsigned int>(time(0))); // Seed only once here

    }

    // Destructor
    Lexicon::~Lexicon() {
        std::cout << "Lexicon destroyed." << std::endl;
    }

    // Static method to get the singleton instance
    Lexicon* Lexicon::GetInstance() 
    {
        if (!instance) 
        {
            std::cerr << "Error: Lexicon instance is not initialized!" << std::endl;
            return nullptr;
        }
        return instance.get();
    }

    // Static method to initialize the singleton instance
    void Lexicon::Initialize(const std::string& wordFilename, const std::string& prefixFilename, const std::string& nsfwFilename) 
    {
        if (!instance) 
        {
            instance = std::make_unique<Lexicon>(wordFilename, prefixFilename, nsfwFilename);
            std::cout << "System: Lexicon Initialized" << std::endl;

            GlobalAssetManager.UE_LoadDictionary(wordFilename);
            GlobalAssetManager.UE_LoadPrefixes(prefixFilename);
            GlobalAssetManager.UE_LoadNSFW(nsfwFilename);
        }
    }
 
    // ISystem Initialize method (can be used for additional setup if needed)
    void Lexicon::Initialize() {
        std::cout << "Lexicon system initialized." << std::endl;
    }

    // ISystem Update method (no real use in this case)
    void Lexicon::Update(float deltaTime) {
        (void)deltaTime;  // Suppress unused parameter warning
        // No periodic updates needed for the Lexicon system
    }

    std::string Lexicon::getRandomPrefix() {
        if (prefixes.empty()) {
            std::cerr << "Error: No prefixes loaded!" << std::endl;
            return "";
        }
        int randomIndex = rand() % prefixes.size();
        return prefixes[randomIndex];
    }

    bool Lexicon::CheckPrefixHasMinimumWords(const std::string& prefix, int MinAmount) {
        const auto& wordSet = trie.getAllWords(); // Get reference to stored words

        if (wordSet.empty()) {
            std::cerr << "Error: No words available in the Trie!" << std::endl;
            return false;
        }

        int count = 0;

        // Efficient word count with early exit
        for (const std::string& word : wordSet) {
            if (word.find(prefix) == 0) { //  Efficient prefix check
                count++;
                if (count >= MinAmount) return true; //  Early exit when condition met
            }
        }

        return false; //  Not enough words found
    }

    std::string Lexicon::GeneratePrefixFromRandomWord(int length, bool Randomize) {
        const auto& wordSet = trie.getAllWords();

        if (wordSet.empty()) {
            std::cerr << "Error: No words available in the Trie!" << std::endl;
            return "";
        }

        std::string randomPrefix;
        do {
            // **Random selection using iterator (O(1) instead of O(n))**
            auto it = wordSet.begin();
            std::advance(it, rand() % wordSet.size());
            std::string randomWord = *it;

            if (randomWord.length() < 2) continue; // Ensure word is long enough

            int prefixLength = (Randomize) ? (rand() % length) + 1 : length;
            prefixLength = std::min(prefixLength, static_cast<int>(randomWord.length()));

            randomPrefix = randomWord.substr(0, prefixLength);

        } while (!CheckPrefixHasMinimumWords(randomPrefix, 20)); //  Ensure at least n words exist (increase for easier prefix)

        return randomPrefix;
    }

    bool Lexicon::checkUserWord(const std::string& userWord) {
        std::string normalizedWord = trim(userWord); // Trim spaces
        std::transform(normalizedWord.begin(), normalizedWord.end(), normalizedWord.begin(), ::tolower);

        if (trie.search(normalizedWord)) {
           // std::cout << "Word found in Trie: " << normalizedWord << std::endl;
            return true;
        }
        else {
            //std::cout << "Word not found in Trie: " << normalizedWord << std::endl;
            return false;
        }
    }

    bool Lexicon::isNsfwWord(const std::string& word) {
        std::string normalizedWord = trim(word); // Trim spaces
        std::transform(normalizedWord.begin(), normalizedWord.end(), normalizedWord.begin(), ::tolower); // Normalize to lowercase
        return nsfwTrie.search(normalizedWord); // Search in NSFW Trie
    }

    size_t Lexicon::countLetters(const std::string& word) {
        size_t count = 0;
        for (char c : word) {
            if (std::isalpha(c)) { // Count only alphabetic characters
                count++;
            }
        }
        return count;
    }
}  // namespace Framework