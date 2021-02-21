#include <grpcpp/grpcpp.h>
#include "grpc/server.grpc.pb.h"
#include "grpc/server.pb.h"
#include "util/typemaps.h"
#include <openssl/sha.h>

#define  ASN_EC_P256        "\x06\x08\x2a\x86\x48\xce\x3d\x03\x01\x07"
#define  ASN_EC_P256_BYTES  10

std::string sha256(std::string data) {
    unsigned char buffer[SHA256_DIGEST_LENGTH];
    SHA256_CTX ctx;
    int rc = SHA256_Init(&ctx);
    if (rc != 1) {
        return "";
    }
    rc = SHA256_Update(&ctx, data.c_str(), data.length());
    if (rc != 1) {
        return "";
    }
    rc = SHA256_Final(buffer, &ctx);
    if (rc != 1) {
        return "";
    }
    return std::string((char*)buffer, SHA256_DIGEST_LENGTH);
}

int testECDSASignature(std::shared_ptr<grep11::Crypto::Stub> client){
    grpc::ClientContext context1;
    grep11::GenerateKeyPairRequest generateECKeypairRequest;
    grep11::GenerateKeyPairResponse generateKeyPairResponse;

    (*generateECKeypairRequest.mutable_pubkeytemplate()) = grep11::ep11Attributes{
		{CKA_EC_PARAMS, ASN_EC_P256, ASN_EC_P256_BYTES},
		{CKA_VERIFY, true},
    };
    (*generateECKeypairRequest.mutable_privkeytemplate()) = grep11::ep11Attributes{
		{CKA_SIGN, true},
		{CKA_EXTRACTABLE, false},
    };
    (*generateECKeypairRequest.mutable_mech()) = grep11::ep11Mechanism(CKM_EC_KEY_PAIR_GEN);

    grpc::Status status = client->GenerateKeyPair(&context1, generateECKeypairRequest, &generateKeyPairResponse);
    if (!status.ok() ) {
        std::cout << "Error in GeneratedKey "<< status.error_message() << std::endl; 
        return 1;
    }
    std::cout << "Generated ECDSA key pair" << std::endl; 

	// Sign data
    grpc::ClientContext context2;
    grep11::SignInitRequest signInitRequest;
    grep11::SignInitResponse signInitResponse;

    (*signInitRequest.mutable_mech()) = grep11::ep11Mechanism(CKM_ECDSA);
    signInitRequest.set_privkey(generateKeyPairResponse.privkey());

    status = client->SignInit(&context2, signInitRequest, &signInitResponse);
	if (!status.ok() ) {
        std::cout << "Error in SignInit "<< status.error_message() << std::endl; 
        return 1;
    }

    grpc::ClientContext context3;
    grep11::SignRequest signRequest;
    grep11::SignResponse signResponse;

	std::string signData = sha256("This data needs to be signed");
    signRequest.set_state(signInitResponse.state());
    signRequest.set_data(signData);
 
    status = client->Sign(&context3, signRequest, &signResponse);
    if (!status.ok() ) {
        std::cout << "Error in Sign "<< status.error_message() << std::endl; 
        return 1;
    }
	std::cout << "Data signed" << std::endl;

    grpc::ClientContext context4;
    grep11::VerifyInitRequest verifyInitRequest;
    grep11::VerifyInitResponse verifyInitResponse;

    (*verifyInitRequest.mutable_mech()) = grep11::ep11Mechanism(CKM_ECDSA);
    verifyInitRequest.set_pubkey(generateKeyPairResponse.pubkey());

    status = client->VerifyInit(&context4, verifyInitRequest, &verifyInitResponse);
    if (!status.ok() ) {
        std::cout << "Error in VerifyInit "<< status.error_message() << std::endl; 
        return 1;
    }

    grpc::ClientContext context5;
    grep11::VerifyRequest verifyRequest;
    grep11::VerifyResponse verifyResponse;

    verifyRequest.set_data(signData);
    verifyRequest.set_state(verifyInitResponse.state());
    verifyRequest.set_signature(signResponse.signature());
	
    status = client->Verify(&context5, verifyRequest, &verifyResponse);
    if (!status.ok() ) {
        grep11::Grep11Error ep11Error;
        grep11::convertError(status, &ep11Error);
        if (ep11Error.code() == CKR_SIGNATURE_INVALID) {
            std::cout << "Invalid Signature in Verify "<< status.error_message() << std::endl; 
        } else {
            std::cout << "Error in Verify "<< status.error_message() << std::endl; 
        }
        return 1;
    }
	std::cout << "Verified" << std::endl;

    grpc::ClientContext context6;
    std::string corruptData = signData;
    corruptData.at(2) = '6';
    verifyRequest.set_data(corruptData);
    verifyRequest.set_state(verifyInitResponse.state());
    verifyRequest.set_signature(signResponse.signature());
	
    status = client->Verify(&context6, verifyRequest, &verifyResponse);
    grep11::Grep11Error ep11Error;
    grep11::convertError(status, &ep11Error);
    if (ep11Error.code() != CKR_SIGNATURE_INVALID ) {
        std::cout << "Expected Error in Verify: "<< ep11Error.detail() << std::endl; 
        return 1;
    }

    return 0;
}