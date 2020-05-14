# Telegraph
An embedded system can/uart framework.

 - /cpp contains the c++ grpc server/client as well as the c++ codegen code
 - /python contains a simple CLI python tool to interact with a device over uart
 - /js contains the javascript grpc server/client code
 - /gui contains the vuejs gui, which uses the javascript grpc server/client


# Building C++ Code

Install bazel and then run

```bash
bazel run //cpp:server
```

To launch the backend

# Building Javascript Code
Install yarn, npm, and node > 13 
(you also need npm unfortunately as a CLI tool uses npm for generating the protobuf files)

Go into the js directory
```bash
yarn generate
yarn link
```

Then go into the gui directory and run
```bash
yarn install
yarn link telegraph
```

Now you are ready to build! To run as a webserver do
```bash
yarn web:serve
```
To run as an electron app, do
```bash
yarn electron:serve
```
