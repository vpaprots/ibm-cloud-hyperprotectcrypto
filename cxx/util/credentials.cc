#include <curl/curl.h>
#include "credentials.h"

size_t curlAppendToString(void *contents, size_t size, size_t nmemb, std::string *s){
    size_t newLength = size*nmemb;
    try{
        s->append((char*)contents, newLength);
    } catch(std::bad_alloc &e) {
        //handle memory problem
        return 0;
    }
    return newLength;
}

std::string fetchFromJson(std::string data, std::string label) {
    int labelBegin = data.find(label);
    if (labelBegin == std::string::npos) {
        std::cout << "no token found!" << std::endl;
        return "";
    }

    int labelStart = labelBegin + label.length();
    int labelEnd = data.find("\"", labelStart);
    if (labelEnd == std::string::npos) {
        std::cout << "no token end found!" << std::endl;
        return "";
    }

    return data.substr(labelStart, labelEnd - labelStart);
}

grpc::StatusCode fetchBearerToken(std::string endpoint, std::string apiKey, std::string &accessToken, time_t &expiry) {
    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl == NULL) {
        return grpc::UNAUTHENTICATED;
    }

    std::string payload = "grant_type=urn:ibm:params:oauth:grant-type:apikey&apikey=" + apiKey;
    std::string url = endpoint + "/identity/token";
    std::string data;
    curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
    headers = curl_slist_append(headers, "charsets: utf-8");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcrp/0.1");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlAppendToString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);

    res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        return grpc::UNAUTHENTICATED;
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup();

    accessToken = fetchFromJson(data, "\"access_token\":\"");
    if (accessToken.length() == 0) {
        return grpc::UNAUTHENTICATED;
    }
    accessToken = "Bearer " + accessToken;

    std::string expiresIn = fetchFromJson(data, "\"expires_in\":");
    if (expiresIn.length() == 0) {
        return grpc::UNAUTHENTICATED;
    }

    int expiresInInt;
    try {
        expiresInInt = std::stoi(expiresIn);
    } catch (std::invalid_argument) {
        return grpc::UNAUTHENTICATED;
    }
    expiry = time(0) + expiresInInt;

    return grpc::OK;
}

grpc::Status grep11::IAMPerRPCCredentials::GetMetadata(grpc::string_ref service_url, grpc::string_ref method_name,
      const grpc::AuthContext& channel_auth_context, std::multimap<grpc::string, grpc::string>* metadata) {
    if (this->endpoint.length() != 0) {
        // We have API key, need to convert it to Bearer token
        // unless expired
        if (difftime(this->expiration, time(0)) < 0) {
            grpc::StatusCode status = grpc::OK;
            this->updateLock.lock();
            // Check if another thread already updated
            if (difftime(this->expiration, time(0)) < 0) {
                status = fetchBearerToken(this->endpoint, this->apiKey, this->accessToken, this->expiration);
            }
            this->updateLock.unlock();
            if (status != grpc::OK) {
                return grpc::Status(status, "Failed to fetch Bearer token from IAM");
            }
        }
    }

    metadata->insert(std::make_pair("bluemix-instance", this->instance));
    metadata->insert(std::make_pair("authorization", this->accessToken));
    return grpc::Status::OK;
}