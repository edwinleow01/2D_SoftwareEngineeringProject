/******************************************************************************
 *
 * \file Lexicon.h
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
#pragma once
#ifndef LEXICON_H
#define LEXICON_H

#include "System.h"  // Assuming System.h defines ISystem
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <cctype> // For isspace
#include <unordered_set>

// TrieNode class to represent a node in the Trie
class TrieNode {
public:
    std::unordered_map<char, TrieNode*> children;           // Use unique_ptr for automatic memory management
    bool isEndOfWord;                                       // True if this node marks the end of a word
    TrieNode() : isEndOfWord(false) {}
};

// Lexicon class declaration (inherits from ISystem)
namespace Framework 
{
    std::string trim(const std::string& str);

    // Trie class to handle word insertion and prefix checking
    class Trie 
    {
    public:
        Trie();
        ~Trie() { clear(root); 
        }
        void insert(const std::string& word);
        bool search(const std::string& word);
        bool startsWith(const std::string& prefix);

        // **Optimized retrieval: Return reference to word set (O(1) access)**
        const std::unordered_set<std::string>& getAllWords() const { return words; }

    private:
        TrieNode* root;
        std::unordered_set<std::string> words; // **Use unordered_set instead of vector**

        void clear(TrieNode* node);
    };


    class Lexicon : public ISystem {
    public:
        // Delete copy constructor and assignment operator to prevent copying
        Lexicon() {};
        Lexicon(const Lexicon&) = delete;
        Lexicon& operator=(const Lexicon&) = delete;

        // Destructor
        ~Lexicon();

        // Static method to access the singleton instance of Lexicon
        static Lexicon* GetInstance();

        // Initialize the singleton instance (only called once)
        static void Initialize(const std::string& wordFilename, const std::string& prefixFilename, const std::string& nsfwFilename);

        // ISystem overrides
        void Initialize() override;  // Initialize system
        void Update(float deltaTime) override;  // Update system
        std::string GetName() override { return "Lexicon"; }

        std::string getRandomPrefix();

        // Function to check if the user's word exists and starts with the prefix
        bool checkUserWord(const std::string& userWord);

        // Function to check if a word is NSFW
        bool isNsfwWord(const std::string& word);

        // Function to count the number of letters in a word
        size_t countLetters(const std::string& word);

        std::string GeneratePrefixFromRandomWord(int length = 2, bool Randomize = false);

        bool CheckPrefixHasMinimumWords(const std::string& prefix, int MinAmount);

        Trie& GetTrie() { return trie; }    // Read-Write Access
        Trie& GetNSFW() { return nsfwTrie; }

        Trie nsfwTrie;  // Trie to store NSFW words

        // Singleton instance
        static std::unique_ptr<Lexicon> instance;

        // Private constructor for singleton pattern
        Lexicon(const std::string& wordFilename, const std::string& prefixFilename, const std::string& nsfwFilename);

    private:
        Trie trie;                          // Trie to store words
    };

}  // namespace Framework

#endif