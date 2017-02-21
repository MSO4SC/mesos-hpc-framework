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

# Update the packages.
sudo apt-get update

# Install a few utility tools.
sudo apt-get install -y tar wget git

# Install the latest OpenJDK.
sudo apt-get install -y openjdk-8-jdk

# Install autotools (Only necessary if building from git repository).
#sudo apt-get install -y autoconf libtool

# Install other Mesos dependencies.
sudo apt-get -y install build-essential python-dev libcurl4-nss-dev libsasl2-dev libsasl2-modules maven libapr1-dev libsvn-dev zlib1g-dev

# Download and extract mesos
wget -q http://www.apache.org/dist/mesos/1.1.0/mesos-1.1.0.tar.gz
tar -xvzf mesos-*
rm mesos-*.tar.gz
cd mesos-*

# Bootstrap (Only required if building from git repository).
#./bootstrap

# Configure and build.
mkdir build
cd build
../configure
make

# Run test suite.
#make check

# move mesos to /opt
cd ../../
sudo mv mesos-* /opt

# Install.
cd /opt/mesos-*
sudo -H make install
sudo ldconfig

# Mesos framework & executor additional development dependencies
sudo apt-get update
sudo apt-get install -y libboost-dev libboost-thread-dev libboost-random-dev libssh-dev
sudo ln -s /usr/lib/x86_64-linux-gnu/libboost_system.so.1.58.0 /usr/lib/x86_64-linux-gnu/libboost_system.so
sudo ln -s /usr/lib/x86_64-linux-gnu/libboost_filesystem.so.1.58.0 /usr/lib/x86_64-linux-gnu/libboost_filesystem.so