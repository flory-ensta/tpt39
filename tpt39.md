# OpenCL

[Presentation](https://gitpitch.com/amusant/tpt39/tmpdev?grs=github&t=beige&p#)
[Computer Architecture](https://gitpitch.com/amusant/tpt39/tmpdev?grs=github&t=beige&p=archi#/)
[OpenCL](https://gitpitch.com/amusant/tpt39/tmpdev?grs=github&t=beige&p=ocl_syntax#/)
[GPU](https://gitpitch.com/amusant/tpt39/tmpdev?grs=github&t=beige&p=gpu_archi#/)
[Performance](https://gitpitch.com/amusant/tpt39/tmpdev?grs=github&t=beige&p=compiler#/)
[FPGA](https://gitpitch.com/amusant/tpt39/tmpdev?grs=github&t=beige&p=fpga_archi#/)
[Project](https://gitpitch.com/amusant/tpt39/tmpdev?grs=github&t=beige&p=project#/)

## Parallelism

* Task Parallelism
  * parallelize tasks that are independant
* Data Parallelism
  * ex: multiplication of each terms of vector (especially GPU)
* Pipeline
  * divide instructions in fetching, decoding, execution, write back, ... in a CPU
  * disadvantage: beginning, if a phase takes longer than the rest, latency (you have to wait for the firsts results) -> no quick reaction
  ![pipeline](https://upload.wikimedia.org/wikipedia/commons/b/bb/Pipeline_cha%C3%AEne_de_traitement.png)

* MPI (NUMA and clusters)
* OpenMP (CPU)
* OpenCL (heterogeneous)

### Amdahl's Law

speedup = 1/(S + P/N)

* S: fraction of the app that is serial
* S: fraction of the app that is parallelizable
* N: number of cores (or improvement factor)

## Computer Architecture

### Main Memory (DDR)

It's often the time consuming part (bottleneck of performance).
DRAM (capacitor), can't keep the charge so every 64ms we need to refresh it.
When we read we destroy the charge, so we need to store the data and write it back. We read row by row. Reading a column is very inefficient. We need a controller to reorganize the traffic (favorize traffic using the same row)

-> Much more comples than a static RAM.

* read
* send
* rewrite
* sometimes: refresh

It introduces a **lot** latency when fetching data (~800 cycles).
DRAM explains many architectures decisions.

### Cache

Static RAM. ~500MHz, data is available in 1 cycle.
~ 512 kB made of 64 B lines.

* Hit: data found
* Miss: data is not in the cache
* Evict: a clean cache line is replaced due to a new allocation (2 threads competiting for one address): we often use N-way cache to reduce it

#### Cache coherence

When a memory is used by multiple masters (CPUs or GPUs). The cache is often out of date. Instead of flashing cahes to the memory and see we have coherent lines in caches that keep each other updated.

## MMU (memory managment unit)

Helps resolve fragmentation problems in DRAM. It splits continuuous data amounts on small blocks that are scattered. Thus we don't require any big continuuous free space. When translating the virtual address into a phisycal one usin the Page Table, the MMU checks the writings rights of the user.

Transtlationnal Look-aside Buffer (TLB) is a cache for the MMU to find quickly the translation of the virtual address.

## Processors

Single/Multiple Intructions/Data

* SISD: CPUhttps://github.com/amusant/tpt39.git
* SIMD: GPUhttps://github.com/amusant/tpt39.git
* MISD: nothinghttps://github.com/amusant/tpt39.git
* MIMD: Multicore
  
Branch prediction: guess branch taken in if...else statements

### out of Order Processors

Instructions are buffered and only executed when their data are available. We then require a Reorder buffer so that the results are sent in the right order.

### Multiprocessor UMA vs NUMA

Uma: processors share the same memory

Threads: give the computing power when a program doesnt require it.
**Simultaenous multi-threading**: optimize the most.

## Odroid-XU4

Max flops in theory: 13.6 GF/s

## OpenCL usage

We need at least a C/C++ programm (the host) on a CPU (the master), and then Kernels computed by the GPU.
We need to allocate the memory space and then we can send the Command Queue IN or OUT of order.

IP : 137.194.217.69

ls
source init.sh
mgd

sur odroid
source init_odroid.sh
mgddaemon

### Vector addition

Launch N threads through a NDRange Kernel.
get_global_id() 
create i/o buffers clCreateBuffer() then clEnqueueWriteBuffer() / clEnqueueRead Buffer()
-> make it blocking so that we easure only the computing time
clSetKernelArg() so that we can use arguments
clEnqueueNDRangeKernel()
-> we can map buffers instead of writing them since the GPU and the CPU use the same DDR

#### Results

##### Sans maping

CPU took 19.02566551 seconds to create random floats.
CPU took 0.52553408 seconds to compute.
GPU took 1.25416294 seconds to initialize args.
GPU took 0.28872370 seconds to compute.
GPU took 0.35228365 seconds to write back.

##### Avec maping

CPU took 18.60632863 seconds to create random floats.
CPU took 0.53684544 seconds to compute.
GPU took 0.00001746 seconds to initialize args.
GPU took 0.29601545 seconds to compute.

## GPU Architecture

CPU

* instruction fetching
* decoding
* execution (ALU, FPU)
* registers (stack pointer, program counter)

GPU : a lot of data parallelism, still memory stalls, latency problems -> same solution.
Main architecture: MIMD. But we don't multiple instructions so we only fetch and decode instructions once. We still plan to use multi threading.

Nowadays we have a single context memory configuSansrable for each thread. Instructions fetching is a single block and schedule to optimize the ALU utilization.

We add a Cache for latency and a Scratchpad memory to communicate between threads especially.

And then we duplicate it in Shader Cores.

We use an ARM GPU: the MALI T628. ARM doesnt create anything, just sells a design, our SoC are made by Samsung.

MALI T628: 256 threads simultaneous. Threads are limited by the number of registers.
We have computation blocks that stay in idle during running since the application doesn't use every operations supported by our GPU

## Performance: Roofline

### Arithmetic Intensity

I = Flops/Bytes

Vector add: 0.5 -> not intense
Matrix multiplication: 1 -> not much more

-> use work group to pack threads sharing similar data to improve the intensity

### Peak Bandwidth

B = Bytes/S

### Peak performance

pi in Flops/S

### Perforammance

#### Usefull commands

memcpy
copyMakeBorder

## What we know

* Parallelism
  * task
  * data
  * pipeline
* Amdahl's law
* memory stalls and latency
* hide latency through caching
* virtual memory
* uniprocessor -> multicores with simultaneous multi*threading
* Mali T628
* GPU architectureinto SIMD
* SoC architecture
* launch opencl
  * Platfrom-> Device-> Context-> Command Queue -> compile kernel
  * Create Buffers-> Copy Buffers from CPU to GPU
  * Create Buffers-> Map Buffers
  * Launch Kernel-> readback result buffers

## FPGA

Field Programmable Gate Array
![fpga](https://perso.telecom-paristech.fr/chaudhur/tpt39/FPGAsArch.svg)

### What do we need for a system (hardware)

* Logic gates
* Finite state machine
* Memory (Register, RAM, DRAM)
Classic hardware:
ASIC: Application Specialized Integrated Circuit

SO FPGA are actually slower than asic. So we pute not programmable hardware in fpga to have some common computations go fast.

### Design

We can't just make a multi million gate design. So we use hardware description language like VHDL. The compiler automates everything, routing, position, latency,..

### Applications

* Prototyping, ASIC Emulation
  * FPGAs can emulate all hardware
* Low/Medium volume hardware
  * Telecom Equipment
* Low Latency Real-Time Systems
  * Drones, Financial Feeds
* Military
* Accelerators

We can use it in a system with a PCI-Xpress, embedded or in Cloud.
We can use the full hardware for every programm.

### Usage

Each loopnest is transformed to a hardware pipeline.
Decide how many computation units to create, and the number of threads.

## CUDA

Blocks (work groups) in a grid (NDRangeKernel) and instead of work_item we have threads.
