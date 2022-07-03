#include "pch.h"

#include "Helpers.h"

const std::vector<BYTE> Helpers::publicKey = Helpers::base64Decode(PUBLIC_KEY);
const std::vector<std::string> Helpers::allowedWildcardDomains { ALLOWED_WILDCARD_DOMAINS };
const std::vector<std::string> Helpers::allowedDomains { ALLOWED_DOMAINS };
const std::vector<std::string> Helpers::allowedSchemes { ALLOWED_SCHEMES };
const std::vector<std::string> Helpers::allowedEmbeddedSchemes { ALLOWED_EMBEDDED_SCHEMES };

// https://stackoverflow.com/questions/48212992/how-to-find-out-if-there-is-any-non-ascii-character-in-a-string-with-a-file-path
bool Helpers::isASCII(const std::string& s)
{
    return !std::any_of(s.begin(), s.end(), [](char c) {
        return static_cast<unsigned char>(c) > 127;
    });
}

// https://stackoverflow.com/questions/313970/how-to-convert-an-instance-of-stdstring-to-lower-case
std::string Helpers::toLower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    return s;
}

// https://stackoverflow.com/a/44562527
std::vector<BYTE> Helpers::base64Decode(const std::string_view data)
{
    // table from '+' to 'z'
    const uint8_t lookup[] = {
        62,  255, 62,  255, 63,  52,  53, 54, 55, 56, 57, 58, 59, 60, 61, 255,
        255, 0,   255, 255, 255, 255, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
        10,  11,  12,  13,  14,  15,  16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
        255, 255, 255, 255, 63,  255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
        36,  37,  38,  39,  40,  41,  42, 43, 44, 45, 46, 47, 48, 49, 50, 51
    };

    static_assert(sizeof(lookup) == 'z' - '+' + 1);

    std::vector<unsigned char> out;
    int val = 0, valb = -8;
    for (uint8_t c : data)
    {
        if (c < '+' || c > 'z')
        {
            break;
        }

        c -= '+';
        if (lookup[c] >= 64)
        {
            break;
        }

        val = (val << 6) + lookup[c];
        valb += 6;

        if (valb >= 0)
        {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }

    return out;
}

// https://stackoverflow.com/a/28269049
std::map<std::string, std::string> Helpers::parseQueryString(std::string query)
{
    std::istringstream stream(query);
    std::map<std::string, std::string> parsed;
    std::string pair, key, value;

    while (std::getline(stream, pair, '&')) // split each term
    {
        std::istringstream term(pair);

        if (std::getline(std::getline(term, key, '='), value))
        {
            parsed[key] = value;
        }
    }
    
    return parsed;
}

std::string Helpers::joinQueryString(std::map<std::string, std::string> query)
{
    std::stringstream stream;
    stream << "?";

    for (auto const& pair : query)
    {
        stream << pair.first << "=" << pair.second << "&";
    }

    std::string result = stream.str();
    result.pop_back(); // remove ending ampersand

    return result;
}