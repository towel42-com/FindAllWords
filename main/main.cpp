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

#include "timer.h"

void showHelp()
{
    std::cout << "Usage FindAlLWords --dict <dictionary> --scramble <letters>" << std::endl;
}

void permuteFast(const std::string& s, const std::string& l, const std::function< bool(const std::string& word) >& isWordStart, const std::function< bool(const std::string& word) >& isWord)
{
    if (s.length() < 1)
    {
        auto newWord = l + s;
        if (isWord(newWord))
            std::cout << newWord << std::endl;
    }
    std::unordered_set<char> uset;
    for (size_t ii = 0; ii < s.length(); ii++)
    {
        if (uset.find(s[ii]) != uset.end())
            continue;
        else
            uset.insert(s[ii]);
        std::string temp = "";
        if (ii < s.length() - 1)
            temp = s.substr(0, ii) + s.substr(ii + 1);
        else
            temp = s.substr(0, ii);
        auto nextWordStart = l + s[ii];

        if ( isWordStart( nextWordStart ) )
            permuteFast(temp, nextWordStart, isWordStart, isWord);
    }
}


// trim from start (in place)
static inline void ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
        }));
}

// trim from end (in place)
static inline void rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
        }).base(), s.end());
}

// trim from both ends (in place)
static inline void cleanup(std::string& str)
{
    //rtrim(str);
    //ltrim(str);
    //std::transform(str.begin(), str.end(), str.begin(), [](unsigned char ch) { return std::tolower(ch); });
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
            //std::sort(currScramble.begin(), currScramble.end());
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

    std::unordered_map< std::string, bool > dictionary;

    for (std::string currLine; std::getline(ifs, currLine); )
    {
        cleanup(currLine);

        for (size_t ii = 0; ii < currLine.length(); ++ii)
        {
            bool isEnd = ii == (currLine.length() - 1);
            auto currSub = currLine.substr(0, ii + 1);
            auto pos = dictionary.find(currSub);
            if (pos == dictionary.end())
            {
                dictionary[currSub] = isEnd;
            }
            else if (isEnd && !(*pos).second)
            {
                (*pos).second = true;
            }
        }
    }

    auto isWordFunc =
        [dictionary](const std::string& word)
    {
        auto pos = dictionary.find(word);
        if (pos == dictionary.end())
            return false;
        return (*pos).second;
    };

    auto isWordStartFunc =
        [dictionary](const std::string& word)
    {
        auto pos = dictionary.find(word);
        return pos != dictionary.end();
    };

    for (auto&& scramble : scrambles)
    {
        std::cout
            << "=============================================" << std::endl
            << "Searching for words in: " << scramble << std::endl
            << "=============================================" << std::endl;

        permuteFast(scramble, "", isWordStartFunc, isWordFunc);
    }
    return 0;
}
