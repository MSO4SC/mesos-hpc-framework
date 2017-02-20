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

## mesos development
sudo apt-get update -qq
sudo apt-get install libboost-dev libboost-thread-dev libboost-random-dev libssh-dev protobuf-compiler -y -qq
sudo ln -s /usr/lib/x86_64-linux-gnu/libboost_system.so.1.58.0 /usr/lib/x86_64-linux-gnu/libboost_system.so
sudo ln -s /usr/lib/x86_64-linux-gnu/libboost_filesystem.so.1.58.0 /usr/lib/x86_64-linux-gnu/libboost_filesystem.so