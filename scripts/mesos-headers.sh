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

## installs mesos headers
unzip ./scripts/mesos-headers.zip
sudo mv mesos /usr/local/include/

sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include/mesos /usr/local/include/mesos/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include/mesos/agent /usr/local/include/mesos/agent/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include/mesos/allocator /usr/local/include/mesos/allocator/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include/mesos/appc /usr/local/include/mesos/appc/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include/mesos/authentication /usr/local/include/mesos/authentication/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include/mesos/authorizer /usr/local/include/mesos/authorizer/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include/mesos/docker /usr/local/include/mesos/docker/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include/mesos/executor /usr/local/include/mesos/executor/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include/mesos/fetcher /usr/local/include/mesos/fetcher/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include/mesos/maintenance /usr/local/include/mesos/maintenance/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include/mesos/master /usr/local/include/mesos/master/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include/mesos/module /usr/local/include/mesos/module/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include/mesos/quota /usr/local/include/mesos/quota/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include/mesos/scheduler /usr/local/include/mesos/scheduler/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include/mesos/slave /usr/local/include/mesos/slave/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include/mesos/uri /usr/local/include/mesos/uri/*.proto

sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include/mesos/v1 /usr/local/include/mesos/v1/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include/mesos/v1/agent /usr/local/include/mesos/v1/agent/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include/mesos/v1/allocator /usr/local/include/mesos/v1/allocator/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include/mesos/v1/executor /usr/local/include/mesos/v1/executor/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include/mesos/v1/maintenance /usr/local/include/mesos/v1/maintenance/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include/mesos/v1/master /usr/local/include/mesos/v1/master/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include/mesos/v1/quota /usr/local/include/mesos/v1/quota/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include/mesos/v1/scheduler /usr/local/include/mesos/v1/scheduler/*.proto

