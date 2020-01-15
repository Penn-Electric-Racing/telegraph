#!/usr/bin/env bash
protoc --proto_path=../proto --python_out=. ../proto/common.proto ../proto/stream.proto
