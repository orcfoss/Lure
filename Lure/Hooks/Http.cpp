#include "pch.h"

#include "Helpers.h"
#include "Hooks/Http.h"

#define CHECK(condition, code) \
    if(!error) {               \
        if ((condition)) {     \
            error = code;      \
        }                      \
    }

Http__httpGetPostWinInet_t Http__httpGetPostWinInet = (Http__httpGetPostWinInet_t)ADDRESS_HTTP__HTTPGETPOSTWININET;
Http__trustCheck_t Http__trustCheck = (Http__trustCheck_t)ADDRESS_HTTP__TRUSTCHECK;

size_t write(char* contents, size_t size, size_t memory, void* pointer)
{
    ((std::string*)pointer)->append((char*)contents, size * memory);
    return size * memory;
}

void __fastcall Http__httpGetPostWinInet_hook(Http* _this, void*, bool isPost, int a3, bool compressData, LPCSTR additionalHeaders, int a6)
{
    CURLU* curl = curl_url();
    CURLUcode result = curl_url_set(curl, CURLUPART_URL, _this->url.c_str(), 0);

    Http _changed = *_this;

    if (result == CURLE_OK)
    {
        char* path;
        char* host;
        char* query;

        curl_url_get(curl, CURLUPART_PATH, &path, 0);
        curl_url_get(curl, CURLUPART_HOST, &host, 0);
        curl_url_get(curl, CURLUPART_QUERY, &query, 0);
        curl_url_cleanup(curl);

        if (path != NULL && host != NULL && query != NULL)
        {
            std::string _path = Helpers::toLower(std::string(path));
            std::string _host = std::string(host);
            std::string _query = std::string(query);

            if (_host == "roblox.com" || _host == "www.roblox.com")
            {
                if (_path == "/asset" || _path == "/asset/" || _path == "/asset/default.ashx")
                {
                    _changed.url = "https://assetdelivery.roblox.com/v1/asset/?" + _query;
                    _this = &_changed;
                }
                else if (_path == "/thumbs/asset.ashx" || _path == "/thumbs/avatar.ashx")
                {
                    /*
                        Both Roblox endpoints require thumbnailFormatId to be set. We will make the default value for it as 0.

                        Asset.ashx -> requires overrideModeration (default false) -> /asset/request-thumbnail-fix
                        Avatar.ashx -> requires dummy (default false) -> /avatar/request-thumbnail-fix

                        1. Parse query
                        2. Construct a brand new blank query with thumbnailFormatId as 0 and dummy/overrideModeration as false (if Avatar.ashx or Asset.ashx)
                        3. Merge the old query with priority over the old query so that if they declared any of the special variables, ours gets overwritten
                        4. Rename id (if found) to assetId or userId (specific to the endpoint)
                        5. Append to the Roblox url (specific to the endpoint)
                        6. Fetch Roblox API
                        7. Parse JSON
                        8. Set the URL as the given url
                    */

                    std::string api = "https://www.roblox.com/" + std::string(_path == "/thumbs/asset.ashx" ? "asset" : "avatar") + "/request-thumbnail-fix";

                    std::map<std::string, std::string> source = Helpers::parseQueryString(query);
                    std::map<std::string, std::string> fixed = {
                        { _path == "/thumbs/asset.ashx" ? "overrideModeration" : "dummy", "false" },
                        { "thumbnailFormatId", "0" }
                    };
                    
                    // the modern Roblox API doesn't care about parameter capitalization because of asp.net quirks
                    // thus, we are able to do this :-)
                    for (auto& pair : source)
                    {
                        fixed[Helpers::toLower(pair.first)] = pair.second;
                    }

                    if (fixed.find("id") != fixed.end())
                    {
                        auto handler = fixed.extract("id");
                        handler.key() = _path == "/thumbs/asset.ashx" ? "assetId" : "userId";
                        
                        fixed.insert(std::move(handler));
                    }

                    api += Helpers::joinQueryString(fixed);
                    
                    // get the api response
                    CURL* curl = curl_easy_init();
                    CURLcode result;
                    long response = 0;
                    std::string data;

                    if (!curl)
                    {
                        throw std::runtime_error("Failed to initialize cURL");
                    }

                    curl_easy_setopt(curl, CURLOPT_URL, api);
                    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write);
                    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);

                    result = curl_easy_perform(curl);
                    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response);

                    curl_easy_cleanup(curl);

                    if (result != CURLE_OK)
                    {
                        throw std::runtime_error("Unexpected error occurred when fetching Roblox API: 0x0");
                    }

                    if (response != 200)
                    {
                        throw std::runtime_error("Unexpected error occurred when fetching Roblox API: Response code was " + std::to_string(response));
                    }

                    rapidjson::Document document;
                    document.Parse(data.c_str());

                    int error = 0;

                    CHECK(document.HasParseError(), 0x01);
                    CHECK(!document.HasMember("d"), 0x02);
                    CHECK(!document["d"].IsObject(), 0x03);
                    CHECK(!document["d"].HasMember("url"), 0x04);
                    CHECK(!document["d"]["url"].IsString(), 0x05);

                    if (error != 0)
                    {
                        throw std::runtime_error("Unexpected error occurred when fetching Roblox API: 0x0" + std::to_string(error));
                    }

                    _changed.url = document["d"]["url"].GetString();
                    _this = &_changed;
                }
            }
        }
    }

    Http__httpGetPostWinInet(_this, isPost, a3, compressData, additionalHeaders, a6);
}

bool __fastcall Http__trustCheck_hook(const char* url)
{
    if (strlen(url) == 7 && !Helpers::isASCII(url))
    {
        // if client opens ie window it passes a char** as opposed to a char*
        url = *(char**)url;
    }

    std::string _url = std::string(url);

    // checking for embedded schemes must come BEFORE checking if it's valid
    // cURL does not treat embedded resources as URLs

    if (_url == "about:blank")
    {
        return true;
    }

    for (std::string allowedEmbeddedScheme : Helpers::allowedEmbeddedSchemes)
    {
        if (_url.rfind(allowedEmbeddedScheme + ":", 0) == 0)
        {
            return true;
        }
    }

    CURLU* curl = curl_url();
    CURLUcode result = curl_url_set(curl, CURLUPART_URL, url, 0);

    if (result != CURLE_OK)
    {
        return false;
    }

    char* scheme;
    char* host;

    curl_url_get(curl, CURLUPART_SCHEME, &scheme, 0);
    curl_url_get(curl, CURLUPART_HOST, &host, 0);
    curl_url_cleanup(curl);

    if (std::find(Helpers::allowedSchemes.begin(), Helpers::allowedSchemes.end(), std::string(scheme)) != Helpers::allowedSchemes.end())
    {
        std::string _host = std::string(host);
        
        // this is crude and inefficient but I don't care
        for (std::string wildcard : Helpers::allowedWildcardDomains)
        {
            if (_host.size() >= wildcard.size() && 0 == wildcard.compare(_host.size() - wildcard.size(), wildcard.size(), wildcard))
            {
                return true;
            }
        }

        return std::find(Helpers::allowedDomains.begin(), Helpers::allowedDomains.end(), _host) != Helpers::allowedDomains.end();
    }

    return false;
}
