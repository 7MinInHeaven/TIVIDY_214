/**
 * @file MailjetHttpClient.h
 * @brief HTTP client wrapper for interacting with the Mailjet API.
 */
#ifndef MAILJETHTTPCLIENT_H
#define MAILJETHTTPCLIENT_H

#include <string>

/**
 * @struct HttpResponse
 * @brief A simple DTO to hold the results of an HTTP request.
 */
struct HttpResponse { 
    long status = 0;     /**< HTTP status code (e.g., 200, 401) */
    std::string body;    /**< The raw JSON body from the response */
    std::string error;   /**< Contains error messages if the request failed */
}; 

/**
 * @class MailjetHttpClient
 * @brief The Adaptee in the Adapter Pattern for Mailjet.
 * 
 * Provides a thin wrapper around libcurl to handle Basic Authentication and POST requests.
 * Used internally by the MailjetEmailAdapter.
 */
class MailjetHttpClient { 
private: 
    std::string apiKey_; 
    std::string apiSecret_;

public: 
    MailjetHttpClient(std::string apiKey, std::string apiSecret);

    /**
     * @brief Executes a POST request with a JSON payload.
     * @param url The Mailjet endpoint.
     * @param json The serialized JSON string.
     * @return HttpResponse containing the status and body.
     */
    HttpResponse postJson(const std::string& url, const std::string& json);
};

#endif