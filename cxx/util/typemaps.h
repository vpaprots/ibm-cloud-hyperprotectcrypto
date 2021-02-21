#ifndef GREP11_UTIL_TYPEMAPS_H
#define GREP11_UTIL_TYPEMAPS_H

#include <string>
#include <google/protobuf/map.h>
#include "grep11.h"
#include "grpc/status.pb.h"

namespace grep11 {

class ep11Attributes : public google::protobuf::Map<::google::protobuf::uint64, ::std::string> {
public:
    class ep11Attribute : public google::protobuf::MapPair<::google::protobuf::uint64, ::std::string> {
    public:
        ep11Attribute(CK_ATTRIBUTE_TYPE type, bool val) 
        : google::protobuf::MapPair<::google::protobuf::uint64, ::std::string>(type, std::string(val?"\1":"\0", 1)){};
        // ep11Attribute(CK_ATTRIBUTE_TYPE type, int val) 
        //   : google::protobuf::MapPair<::google::protobuf::uint64, ::std::string>(type, std::string(val, 1)){};
        ep11Attribute(CK_ATTRIBUTE_TYPE type, const char* val, int len) 
        : google::protobuf::MapPair<::google::protobuf::uint64, ::std::string>(type, std::string(val, len)){};
    };

    ep11Attributes(CK_ATTRIBUTE_PTR tmpl, CK_ULONG tmplCount) 
      : google::protobuf::Map<::google::protobuf::uint64, ::std::string>() {
        for (int i = 0; i < tmplCount; i++) {
            auto attr = tmpl[i];
            (*this)[(::google::protobuf::uint64)attr.type] = std::string((const char*)attr.pValue, attr.ulValueLen);
        }
    }
    ep11Attributes(std::initializer_list<ep11Attribute> attrs) 
      : google::protobuf::Map<::google::protobuf::uint64, ::std::string>(){
          this->insert(attrs.begin(), attrs.end());
      }
    
};

class ep11Mechanism : public grep11::Mechanism {
    public:
    ep11Mechanism(::google::protobuf::uint64 mech){
        this->set_mechanism(mech);
    }
    ep11Mechanism(::google::protobuf::uint64 mech, std::string parm){
        this->set_mechanism(mech);
        this->set_allocated_parameter(&parm);
    }
};

void convertError(const grpc::Status& status, grep11::Grep11Error* ep11Error) {
    google::rpc::Status to;

    if (!to.ParseFromString(status.error_details())) {
        ep11Error->set_code(CKR_GENERAL_ERROR);
        ep11Error->set_detail("Server returned error: " + status.error_message());
        return;
    }

    if (to.details_size() != 1) {
        ep11Error->set_code(CKR_GENERAL_ERROR);
        ep11Error->set_detail("Expected only one error: " + status.error_message());
    }

    if (!to.details()[0].UnpackTo(ep11Error)) {
        ep11Error->set_code(CKR_GENERAL_ERROR);
        ep11Error->set_detail("Detail is the wrong type: " + status.error_message());
    }
};

};

#endif