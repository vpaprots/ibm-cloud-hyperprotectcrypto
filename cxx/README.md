# Prerequisites

See [grpc/Building](https://github.com/grpc/grpc/blob/master/BUILDING.md)

# Running

Edit `examples/main.cc`:
```
std::string instance = "<HPCS-instance>";
std::string endpoint = "https://iam.test.cloud.ibm.com";
std::string apiKey = "IAM API key";
std::string url = "address:port";
```

Then run:
```
$ make -j
=== RUN   testGenerateRandom
--- PASS: testGenerateRandom
=== RUN   testECDSASignature
Generated ECDSA key pair
Data signed
Verified
--- PASS: testECDSASignature
```