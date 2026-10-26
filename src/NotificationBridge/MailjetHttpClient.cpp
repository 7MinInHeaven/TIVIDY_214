#include "MailjetHttpClient.h"
#include <curl/curl.h>
#include <utility>


namespace {
size_t writeCb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* out = static_cast<std::string*>(userdata);
    out->append(ptr, size * nmemb);
    return size * nmemb;
}
}

HttpResponse MailjetHttpClient::postJson(const string& url, const string& json){
    HttpResponse res; 

    CURL* curl = curl_easy_init(); 

    if(!curl) {
        res.error = "curl failed";
        return res;
    }
}

curl_slist* headers = curl_slist_append(nullptr, "Contnet-Type: application/json");

const std::string userpwd = apiKey + ":" + apiSecret;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_USERPWD, userpwd.c_str()); // Basic Auth
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res.body);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    CURLcode rc = curl_easy_perform(curl);
    if (rc != CURLE_OK) {
        res.error = curl_easy_strerror(rc);
    } else {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res.status);
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return res;
}