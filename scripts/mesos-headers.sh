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
unzip ./3rdparty/mesos-headers.zip
sudo mv mesos /usr/local/include/

unzip ./3rdparty/stout-headers.zip
sudo mv stout /usr/local/include/

unzip ./3rdparty/process-headers.zip
sudo mv glog /usr/local/include/

unzip ./3rdparty/glog-headers.zip
sudo mv glog /usr/local/include/

## generate  protobuf mesos headers files
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include /usr/local/include/mesos/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include /usr/local/include/mesos/agent/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include /usr/local/include/mesos/allocator/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include /usr/local/include/mesos/appc/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include /usr/local/include/mesos/authentication/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include /usr/local/include/mesos/authorizer/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include /usr/local/include/mesos/docker/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include /usr/local/include/mesos/executor/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include /usr/local/include/mesos/fetcher/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include /usr/local/include/mesos/maintenance/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include /usr/local/include/mesos/master/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include /usr/local/include/mesos/module/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include /usr/local/include/mesos/quota/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include /usr/local/include/mesos/scheduler/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include /usr/local/include/mesos/slave/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include /usr/local/include/mesos/uri/*.proto

sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include /usr/local/include/mesos/v1/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include /usr/local/include/mesos/v1/agent/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include /usr/local/include/mesos/v1/allocator/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include /usr/local/include/mesos/v1/executor/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include /usr/local/include/mesos/v1/maintenance/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include /usr/local/include/mesos/v1/master/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include /usr/local/include/mesos/v1/quota/*.proto
sudo protoc -I=/usr/local/include --cpp_out=/usr/local/include /usr/local/include/mesos/v1/scheduler/*.proto

