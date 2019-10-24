# Telegraph
An embedded system can/uart framework.

cpp contains the c++ grpc server/client as well as the c++ codegen code
proto contains all the grpc api definition files
js contains the javascript grpc server/client code
gui contains the vuejs gui, which uses the javascript grpc server/client


# Building C++ Code

Install bazel and then run

bazel build //cpp:generate

This will build the codegen tool (cpp/main/generate.cpp)

To run the codegen tool do "bazel run //cpp:generate -- generate-arguments"

To run the C++ test in cpp/main/test.cpp do "bazel run //cpp:test"
