// The MIT License( MIT )
//
// Copyright( c ) 2020 Scott Aron Bloom
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <cctype>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <memory>
#include "Timer.h"

struct SCharNode
{
    char fChar{ 0 };
    bool fIsWord{ false };
    std::vector<std::unique_ptr< SCharNode > > fChildren;

    void addWord(const std::string& str, std::size_t pos = 0)
    {
        auto curr = str[pos] - 'a';
        if (curr >= fChildren.size())
        {
            fChildren.resize(curr + 1);
        }

        if (!fChildren[curr])
        {
            fChildren[curr].reset(new SCharNode);
            fChildren[curr]->fChar = str[pos];
        }

        bool atEnd = pos == (str.length() - 1);
        if (atEnd)
            fChildren[curr]->fIsWord = atEnd;
        else
            fChildren[curr]->addWord(str, pos + 1);
    }

    bool isWord(const std::string& str, bool wholeWordOnly = true, std::size_t pos = 0)
    {
        auto curr = str[pos] - 'a';
        if (curr >= fChildren.size())
            return false;
        if (!fChildren[curr])
            return false;

        bool atEnd = pos == (str.length() - 1);
        if (atEnd)
        {
            if (wholeWordOnly)
                return fChildren[curr]->fIsWord;
            return true;
        }
        return fChildren[curr]->isWord(str, wholeWordOnly, pos + 1);
    }

    bool isWordStart(const std::string& str)
    {
        return isWord(str, false, 0);
    }
};


void showHelp()
{
    std::cout << "Usage FindAlLWords --dict <dictionary> --scramble <letters>" << std::endl;
}

// trim from both ends (in place)
static inline void cleanup(std::string& str)
{
    for (auto ii = str.begin(); ii != str.end(); )
    {
        *ii = std::tolower(*ii);
        if ((*ii < 'a') || (*ii > 'z'))
            ii = str.erase(ii);
        else
            ii++;
    }
}

int main(int argc, char** argv)
{
    //199711L	C++98
    //201103L	C++11
    //201402L	C++14
    //201703L	C++17
    //202002L	C++20
    //std::cout << __cplusplus << std::endl;

    CTimer timer;

    std::string dictionaryFile;
    std::list< std::string > scrambles;
    for (int ii = 1; ii < argc; ++ii)
    {
        if (std::strncmp(argv[ii], "--dict", 6) == 0)
        {
            if (ii == (argc - 1))
            {
                showHelp();
                return 1;
            }
            dictionaryFile = argv[++ii];
        }
        else if (std::strncmp(argv[ii], "--scramble", 10) == 0)
        {
            if (ii == (argc - 1))
            {
                showHelp();
                return 1;
            }
            std::string currScramble = argv[++ii];
            std::transform(currScramble.begin(), currScramble.end(), currScramble.begin(), [](unsigned char ch) { return std::tolower(ch); });
            scrambles.push_back(currScramble);
        }
    }

    if (dictionaryFile.empty() || scrambles.empty())
    {
        showHelp();
        return 1;
    }

    auto ifs = std::ifstream(dictionaryFile);
    if (!ifs.is_open())
    {
        std::cerr << "Could not open dictionary '" << dictionaryFile << "'";
        return 1;
    }

    SCharNode dictionary;

    //std::unordered_map< std::string, bool > dictionary;

    for (std::string currLine; std::getline(ifs, currLine); )
    {
        cleanup(currLine);
        dictionary.addWord(currLine);
    }

    auto isWordFunc =
        [&dictionary](const std::string& word)
    {
        return dictionary.isWord(word);
    };

    auto isWordStartFunc =
        [&dictionary](const std::string& word)
    {
        return dictionary.isWordStart(word);
    };

    for (auto&& scramble : scrambles)
    {
        std::cout
            << "=============================================" << std::endl
            << "Searching for words in: " << scramble << std::endl
            ;
        std::sort(scramble.begin(), scramble.end());
        std::cout
            << "Starting with : " << scramble << std::endl
            << "=============================================" << std::endl;

        size_t numPermutations = 0;
        do
        {
            if (isWordFunc(scramble))
            {
                std::cout << scramble << std::endl;
            }
        } while (std::next_permutation(scramble.begin(), scramble.end()));

        do
        {
            if (isWordFunc(scramble))
            {
                std::cout << scramble << std::endl;
            }
        } while (std::next_permutation(scramble.begin(), scramble.end()));
    }

    return 0;
}
