# Docker Environment for the PACE Challenge 2025

This repository contains a docker stack for the [PACE Challenge 2025](https://pacechallenge.org/2025/), consisting of four containers; one for each problem and track.
The evaluation of all solvers will take place within this environment to ensure fairness. To be precise, we provide one core and 16 GB of RAM for all solvers.

This repository contains a small demo solver and a small set of test instances for both problems.

## Getting started

First of all, install docker and  the docker compose plugin on your machine if not already present. We recommend to use [Docker Desktop](https://docs.docker.com/get-started/introduction/get-docker-desktop/) which contains everything you need and is easy to install. However, note that the commercial use of Docker Desktop for larger enterprises requires a paid subscription.

Afterwards, build the docker images using the following command.

```bash
docker compose build
```
Finally, you can start the evaluation process for all tracks by using the following command.

```bash
docker compose up
```

This starts the execution of four containers (one for each combination of problem and track) and evaluates a demo solver.
Each container terminates once the evaluation process has finished.

You can also start the execution of a single container by using the following command instead.
```bash
docker compose up [name]
```
where `[name]` is one of the following elements reflecting the problem and track: `ds-exact`, `ds-heuristic`, `hs-exact`, `hs-heuristic`.

If the process succeeded, find the `output` directory containing a `.csv` file for each track. This file contains the results of the evaluation process, including the status per instance (ok, exception, ...), the runtime, the solution size, and possibly an error message.

You are now ready to install your own solver.


## How to replace the demo solver
If you would like to replace the demo solver, you need to modify some things. First clone this repository.
Then:

 - If your solver is provided as an executable file, e.g., a python script, an executable jar or a static binary, simply replace our demo solver in the respective `solver` directory.
   Finally, you need to adjust the respective `SOLVER_CMD` field in the `docker-compose.yaml` to specify how to execute your solver.
   In our demo case, the solver is a python script which is executed py running `python3 ds_greedy.py`, hence the `SOLVER_CMD` is set to `"python3,/solver/ds_greedy.py"`. Observe that the comma serves as a delimeter.
 - If you want to build your solver from source, copy your source folder into the respective `solver` directory.
   Afterwards, you need to modify the respective `Dockerfile` depending on the installation process of your solver. In each `Dockerfile`, you will find a comment at the place you need to modify.
   We provide some examples.
     - If you need to execute a make file, add the line `RUN make` into the `Dockerfile`. It might be the case that you need to make the files executable, eg by running `chmod 777`.
     - If you need to compile a `.cpp` source file using `g++` add the line `RUN g++ -O2 -std=c++17 solver.cpp -o solver` into the `Dockerfile`.
     - If you need to compile a `.java` source file using `javac`,  add the line `RUN javac solver.java` into the `Dockerfile`.
   
   In case you need to install third-party dependencies, we recommend the use of different build stages in order to reduce the size of the image. We refer to [multi-stage-builds](https://docs.docker.com/build/building/multi-stage/) for further explanation.
   
   Finally, you need to adjust the `SOLVER_CMD` field in the `docker-compose.yaml` as in the first case.
   

## How to replace the instances
This project contains only three test instances per track in the `instances` directory. If you want to run the evaluation process on other instances, simply replace (or add) your instances into the `instances` folder.
For example, you can find the official set of public instances for PACE 2025 also on [Github](https://github.com/MarioGrobler/PACE2025-instances). Note that the folder structure in the instance repository matches the one required here.

      
## The environment variables in the Docker compose file
The docker compose file `docker-compose.yaml` specifies the configuration for each track. This includes resource limitations and the specification of time limits. In particular, we allow only one core and 16GB of RAM for each track. For heuristic solvers, we allow 5 minutes before sending a SIGTERM signal with a mercy time of 25 seconds (that is, the time before your solver is killed after sending SIGTERM), while we allow 30 minutes for exact solvers with a mercy time of 30 seconds.
This configuration reflects exactly the configuration we will use for the final evaluation of the submitted solvers. However, you can modify the resource limitations by editing the `cpus` and `memory` values for each track. Similarly, you can modify the mentioned time limits by editing `MAX_TIME` and `MERCY_TIME` values for each track.


## LICENSE
Copyright 2025 Mario Grobler

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.


### Acknowledgements
A big “Thank You” goes to Yannik (Putzzmunta) for assisting with the docker setup.