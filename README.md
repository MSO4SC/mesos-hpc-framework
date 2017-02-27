# Mesos HPC Framework

[![Join the chat at https://gitter.im/MSO4SC/mesos-slurm-framework](https://badges.gitter.im/MSO4SC/mesos-slurm-framework.svg)](https://gitter.im/MSO4SC/mesos-slurm-framework?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

The HPC framework for Mesos runs tasks on different HPC machines. It is in heavy development (see issues) and for now should not be considered as estable.

### Main restrictions for now:

* Framework sending only dummy tasks. REST API to send tasks to the framwork to be developed.
* Only HPCs using Slurm are supported (only an Slurm executor developed).


## Build & Install

* Compile and install mesos:  
```
$ ./scripts/build-dependencies.sh  
```

* Compile the framework and executor:
```
$ ./scripts/build.sh  
```

* Create the executor credentials:
```
$ cd ./slurm_executor/etc  
$ cp config.json_sample config.json  
```

* Finally edit the file created to add the SSH credentials to the HPC machine.



## Run

To run the framework a mesos instance running somewhere is needed. The exact command arguments to run the hpc framework it is going to depend on this configuration. Some examples:

* Mesos with only one master not using authorization mechanism on 192.168.56.10 and listening on default port.
```
$ cd ./hpc_framework/Release  
$ DEFAULT_PRINCIPAL= ./hpc-framework --master=192.168.56.10:5050  
```

* Mesos running with zookeeper listening on default port and not using authorization mechanism.
```
$ cd ./hpc_framework/Release  
$ DEFAULT_PRINCIPAL= ./hpc-framework --master=zk://node1:2181/mesos  
```
