#include <iostream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <sstream>
#include <ctime>
#include <cstring>

using json = nlohmann::json;

// function to URL-encode a string using libcurl
std::string urlEncode(const std::string& value) {
    CURL* curl = curl_easy_init();
    if (!curl) return value;

    char* encoded = curl_easy_escape(curl, value.c_str(), value.length());
    std::string result(encoded);
    curl_free(encoded);
    curl_easy_cleanup(curl);
    return result;
}

// function to write data received from curl into a string
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// network request function using libcurl to fetch data from a URL
std::string networkRequest(const std::string& url) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return readBuffer;
}

// getCurrentDateTime
std::string formatBanTime(const std::string& banTime) {
    if (banTime.empty() || banTime == "unknown" || banTime == "неизвестно") {
        return "неизвестно";
    }

    // parse ISO 8601 format (e.g., "2024-06-01T12:34:56") and convert to "DD.MM.YYYY в HH:MM:SS"
    struct tm tm = {};
    if (strptime(banTime.c_str(), "%Y-%m-%dT%H:%M:%S", &tm)) {
        char buffer[100];
        strftime(buffer, sizeof(buffer), "%d.%m.%Y в %H:%M:%S", &tm);
        return std::string(buffer);
    }

    return banTime;
}

// formats current date and time as "DD.MM.YYYY HH:MM"
std::string getCurrentDateTime() {
    time_t now = time(nullptr);
    struct tm* t = localtime(&now);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%d.%m.%Y %H:%M", t);
    return std::string(buffer);
}

int main(int argc, char* argv[]) {


    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <IP> <JAIL> [USERNAMES] [ATTEMPTS_COUNT] [BAN_TIME]" << std::endl;
        return 1;
    }

    std::string ip = argv[1];
    std::string jail = argv[2];

    // parameters with default values
    std::string usernames = "не определены";
    std::string attemptsCount = "неизвестно";
    std::string banTime = "неизвестно";

    // if additional parameters are provided, override the defaults
    if (argc >= 4) {
        usernames = argv[3];
    }
    if (argc >= 5) {
        attemptsCount = argv[4];
    }
    if (argc >= 6) {
        banTime = argv[5];
    }

    // Secret token and chat ID for Telegram bot
    std::string token = "YOUR_TELEGRAM_BOT_TOKEN";
    std::string chat_id = "YOUR_CHAT_ID";

    //get geolocation data for the IP
    std::string geoUrl = "http://ip-api.com/json/" + ip + "?fields=status,country,city,isp,proxy,lat,lon";
    std::string geoData = networkRequest(geoUrl);

    json g;
    try {
        g = json::parse(geoData);
    } catch (const std::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        g["status"] = "fail";
    }

    std::string proxyStatusR = g.value("proxy", false) ? "✅ Да" : "❌ Нет";
    std::string country = g.value("country", "Неизвестно");
    std::string city = g.value("city", "Неизвестно");
    std::string isp = g.value("isp", "Неизвестно");
    float lat = g.value("lat", 0.0f);
    float lon = g.value("lon", 0.0f);

    // format ban time and get current date/time
    std::string formattedBanTime = formatBanTime(banTime);
    std::string currentDateTime = getCurrentDateTime();

    // 2. Create the message to send to Telegram
    std::stringstream message;

    // header with current date/time
    message << " *Блокировка новой атаки:* \n\n";


    // IP information
    message << " ▫️*IP:* `" << ip << "`\n";
    message << " ▫️*Jail:* `" << jail << "`\n";
    message << " ▫️*Локация:* " << country;
    if (!city.empty() && city != "Неизвестно") {
        message << ", " << city;
    }
    message << "\n";
    message << " ▫️*Провайдер:* " << isp << "\n";
    message << " ▫️*VPN/Proxy:* " << proxyStatusR << "\n\n";



    // information about attempts and logins
    message << "🔑 *Попыток входа:* " << attemptsCount << "\n";

    // Logins
    if (usernames != "не определены" && usernames != "неизвестно" && !usernames.empty()) {
        message << "▫️ *Логины:* `" << usernames << "`\n\n";
    }

    message << "▫️ *Время бана:* " << formattedBanTime << "\n\n";




    // link to map if coordinates are available
    if (lat != 0.0 && lon != 0.0) {
        message << " [Посмотреть на карте](https://www.google.com/maps?q=" << lat << "," << lon << ")\n\n";
    }

    // status of the ban
    message << "▪️ *Статус:* Забанен.\n";


    // 3. Send the message to Telegram
    std::string tgUrl = "https://api.telegram.org/bot" + token + "/sendMessage";
    std::string postData = "chat_id=" + chat_id + "&text=" + urlEncode(message.str()) + "&parse_mode=Markdown";

    CURL* curl = curl_easy_init();
    bool success = false;
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, tgUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

        CURLcode res = curl_easy_perform(curl);
        success = (res == CURLE_OK);
        curl_easy_cleanup(curl);
    }

    if (success) {
        std::cout << "✅ Report sent for IP: " << ip << std::endl;
        std::cout << "   Logins: " << usernames << std::endl;
        std::cout << "   Attempts: " << attemptsCount << std::endl;
    } else {
        std::cerr << "❌ Failed to send report for IP: " << ip << std::endl;
        return 1;
    }

    return 0;
}
