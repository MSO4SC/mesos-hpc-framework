#!/bin/bash

protoc -I=./slurm_framework/src --cpp_out=./slurm_framework/src ./slurm_framework/src/jobsettings.proto
protoc -I=./slurm_executor/src --cpp_out=./slurm_executor/src ./slurm_executor/src/jobsettings.proto
