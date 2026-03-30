#include <iostream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string networkRequest(std::string url) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return readBuffer;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <IP> <JAIL>" << std::endl;
        return 1;
    }

    std::string ip = argv[1];
    std::string jail = argv[2];

    std::string token = "здесь_вставить_токен_бота";
    std::string chat_id = "здесь_chat_id_тг_акка";

    std::string geoUrl = "http://ip-api.com/json/" + ip + "?fields=status,country,city,isp,proxy";
    std::string geoData = networkRequest(geoUrl);
    auto g = json::parse(geoData);

    std::string proxyStatusR = g.value("proxy", false) ? "Да" : "Нет";

    std::string message = "🚫 *Заблокирован IP:* `" + ip + "`\n\n"
                         "*Сервис/IP:* " + jail + "\n"
                         "*Локация:* " + g.value("country", "Неизвестно") + ", " + g.value("city", "Неизвестно") + "\n"
                         "*Провайдер:* " + g.value("isp", "Неизвестно") + "\n"
                         "*VPN/Proxy:* " + proxyStatusR;

    std::string tgUrl = "https://api.telegram.org/bot" + token + "/sendMessage";
    std::string postData = "chat_id=" + chat_id + "&text=" + message + "&parse_mode=Markdown";

    CURL* curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, tgUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }

    std::cout << "Report sent for IP: " << ip << std::endl;
    return 0;
}
