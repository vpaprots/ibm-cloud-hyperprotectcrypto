# Copyright IBM Corp. All Rights Reserved.
#
# SPDX-License-Identifier: Apache-2.0
#

build-protos:
	@echo "=== Building the proto bindings ==="
	protoc  protos/server.proto \
	        -Iprotos \
	        -Ivendor/github.com/gogo/protobuf/gogoproto \
	  --gogofast_out=plugins=grpc,\
Mgoogle/protobuf/descriptor.proto=github.com/gogo/protobuf/protoc-gen-gogo/descriptor,\
Mgoogle/api/annotations.proto=github.com/gogo/googleapis/google/api:./golang/grpc \
	  --grpc_out=cxx/grpc --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` \
      --cpp_out=cxx/grpc \
      --python_out=python/grpc \
      --grpc_python_out=python/grpc --plugin=protoc-gen-grpc_python=`which grpc_python_plugin`
	gofmt -w -s golang/ep11
	protoc  protos/status.proto \
	        -Iprotos \
	  --grpc_out=cxx/grpc --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` \
      --cpp_out=cxx/grpc \
      --python_out=python/grpc \
	  --grpc_python_out=python/grpc --plugin=protoc-gen-grpc_python=`which grpc_python_plugin`