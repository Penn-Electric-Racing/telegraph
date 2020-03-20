#!/usr/bin/env bash
protoc --proto_path=.. --python_out=. ../common.proto ../stream.proto
