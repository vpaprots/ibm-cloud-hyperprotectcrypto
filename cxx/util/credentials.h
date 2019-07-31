#ifndef GREP11_UTIL_CREDENTIALS_H
#define GREP11_UTIL_CREDENTIALS_H

#include <string>
#include <time.h>
#include <grpcpp/grpcpp.h>

namespace grep11 {
class IAMPerRPCCredentials : public grpc::MetadataCredentialsPlugin {
public:
    IAMPerRPCCredentials(std::string instance, std::string accessToken)
        : instance(instance), accessToken(accessToken), 
          expiration(0), endpoint(""), apiKey(""), updateLock() {};
    IAMPerRPCCredentials(std::string instance, std::string endpoint, std::string apiKey)
        : instance(instance), endpoint(endpoint), apiKey(apiKey), updateLock(),
          accessToken(""), expiration(0) {};
    grpc::Status GetMetadata(grpc::string_ref service_url, grpc::string_ref method_name,
      const grpc::AuthContext& channel_auth_context, std::multimap<grpc::string, grpc::string>* metadata) override;
    
    virtual ~IAMPerRPCCredentials(){};
private:
	time_t expiration;
	std::mutex updateLock;
	std::string instance;    // Always Required - IBM Cloud HPCS instance ID
	std::string accessToken; // Required if APIKey nor Endpoint are specified - IBM Cloud IAM access token
	std::string apiKey;      // Required if AccessToken is not specified - IBM Cloud API key
	std::string endpoint;    // Required if AccessToken is not specified - IBM Cloud IAM endpoint
};

};

#endif //GREP11_UTIL_CREDENTIALS_H
