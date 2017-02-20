#!/bin/bash

# Copyright 2017 MSO4SC - javier.carnero@atos.net
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

## framework build
protoc -I=./hpc_framework/src --cpp_out=./hpc_framework/src ./hpc_framework/src/jobsettings.proto
make -C ./hpc_framework/Debug

## slurm executor build
protoc -I=./slurm_executor/src --cpp_out=./slurm_executor/src ./slurm_executor/src/jobsettings.proto
make -C ./slurm_executor/Debug