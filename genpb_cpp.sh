#!/bin/bash

protoc -I=. --cpp_out=server/ gravioli.proto
