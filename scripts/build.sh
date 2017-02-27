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

# move to current directory
cd "$(dirname "$0")"

# get project full path
ROOT=$(readlink -f ../.)

## slurm executor build
/opt/mesos-1.1.0/build/3rdparty/protobuf-2.6.1/src/protoc -I=$ROOT/slurm_executor/src --cpp_out=$ROOT/slurm_executor/src $ROOT/slurm_executor/src/jobsettings.proto
make -C $ROOT/slurm_executor/Release

## framework build
/opt/mesos-1.1.0/build/3rdparty/protobuf-2.6.1/src/protoc -I=$ROOT/hpc_framework/src --cpp_out=$ROOT/hpc_framework/src $ROOT/hpc_framework/src/jobsettings.proto
make -C $ROOT/hpc_framework/Release
ln -fs $ROOT/slurm_executor/Release/slurm-executor $ROOT/hpc_framework/Release/