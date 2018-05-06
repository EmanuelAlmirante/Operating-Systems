# Operating Systems

This repository hosts the project of the course of Operating Systems.

**Note:** Some images have text in portuguese. Please use Google Translate if you need to understand something.

## Table of Contents

- [Objetives](https://github.com/EmanuelAlmirante/Operating-Systems/blob/master/README.md#objectives)
- [Part 1](https://github.com/EmanuelAlmirante/Operating-Systems/blob/master/README.md#part-1)
  - [KOS: Key value store of the OS course](https://github.com/EmanuelAlmirante/Operating-Systems/blob/master/README.md#kos-key-value-store-of-the-os-course)
  - [Structure and organization of the project](https://github.com/EmanuelAlmirante/Operating-Systems/blob/master/README.md#structure-and-organization-of-the-project)
  - [Design and implementation of the solution of the first part of the project](https://github.com/EmanuelAlmirante/Operating-Systems/blob/master/README.md#design-and-implementation-of-the-solution-of-the-first-part-of-the-project)
    - [Dispersion tables used to save the data of KOS](https://github.com/EmanuelAlmirante/Operating-Systems/blob/master/README.md#dispersion-tables-used-to-save-the-data-of-kos)
    - [Buffer to the communication between client and server tasks](https://github.com/EmanuelAlmirante/Operating-Systems/blob/master/README.md#buffer-to-the-communication-between-client-and-server-tasks)
    - [Synchronization between server tasks to access the data saved in KOS](https://github.com/EmanuelAlmirante/Operating-Systems/blob/master/README.md#synchronization-between-server-tasks-to-access-the-data-saved-in-kos)
  - [Compile the code](https://github.com/EmanuelAlmirante/Operating-Systems/blob/master/README.md#compile-the-code)
  - [Testing](https://github.com/EmanuelAlmirante/Operating-Systems/blob/master/README.md#testing)
- [Part 2](https://github.com/EmanuelAlmirante/Operating-Systems/blob/master/README.md#part-2)
  - [KOS: Key value store of the OS course](https://github.com/EmanuelAlmirante/Operating-Systems/blob/master/README.md#kos-key-value-store-of-the-os-course-1)
  - [Requisites of the second part of the project](https://github.com/EmanuelAlmirante/Operating-Systems/blob/master/README.md#requisites-of-the-second-part-of-the-project) 
    - [Concurrency](https://github.com/EmanuelAlmirante/Operating-Systems/blob/master/README.md#concurrency)
    - [Persistence of data in KOS](https://github.com/EmanuelAlmirante/Operating-Systems/blob/master/README.md#persistence-of-data-in-kos)
  - [Compile the code](https://github.com/EmanuelAlmirante/Operating-Systems/blob/master/README.md#compile-the-code-1)
  - [Testing](https://github.com/EmanuelAlmirante/Operating-Systems/blob/master/README.md#testing-1)
  
  
## Objectives

The objective of this project is to develop a system for the storage of data caleed KOS (_Key-value store of the Operating Systems course_).

Any help or recommendation is welcome, so feel free to change the code.

## Part 1

### KOS: Key value store of the OS course

The objective of this project is to develop a system for the storage of data caleed KOS (_Key-value store of the Operating Systems course_). KOS uses a logical model of data organization called _key-value_, where data is associated with the following pair: a "key" that allows to identify that data, and a "value". This type of data storage systems are used in many contexts, like, for example, in the registry of Windows to save configuration's information and metadata of the operating system.

The logical organization of KOS is illustrated in Figure 1 (see it in the PDF). Data saved in KOS is organized in partitions ("shards"), identified by an integer identifier (shardId). "Sharding" is a technique used to optimize the performance of the access primitives, because it allows to parallelize the access to data and subdivide them in partitions (shards) and assign partitions to dedicated servers.

KOS supports the following API:
  
  - char* get(int clientId, int shardId, char* key): returns the "Value" associated to the key ("Key") passed as entry parameter, case this exist in KOS, NULL otherwise.
  
  - char* put(int clientId, int shardId, char* key, char* value): inserts a pair "<Key, Value>" and returns NULL if the "Key" does not exist, or, otherwise, the previous value associated to that key.
  
  - char* remove(int clientId, int shardId, char* key): deletes the key identified by the string Key, returning the  value of the key, case this exists, and NULL otherwise.
  
  - KV_t* kos_getAllKeys(int clientId, int shardId, int* sizeKVarray): returns a pointer to a vector of pairs "<Key, Value>", with each pair being saved in a data structure of pairs "<Key, Value>", whose definitio is specified in Annex A (see it in the PDF). The size of the vector that is returned is written in the parameter sizeKVarray.
  
For simplicity purposes, it is assumed that both the key and the value are strings of a fixed size, defined by the constant KV_SIZE.

Internally, each partition of KOS is supported by a dispersion table (hashmap), that needs to be implemented. More indications about the implementation of this dispersion table is given further ahead.

KOS executes itself in an unique process composed by multiple tasks. There are server tasks and client tasks. Server tasks manage the state of KOS and serve requests generated by client tasks, as seen in Figure 2 (see it in the PDF).

Client tasks do not directly access the state of KOS. Instead, these taskss should interact with the server tasks using the specified functions in the file _kos_client.h_, which insert a request in an in-memory buffer shared with the server tasks. Client tasks put in the aforementioned buffer their requests that, in turn, server tasks execute; this same buffer should be used to return the respective answers to the client tasks.

### Structure and organization of the project

The project is divided in two parts:

  - The first part of the project has as objective the development of the base architecture of KOS. In particular, during the first phase it will be necessary:
    - conceive and implement a basic scheme of synchronization between client and server tasks. In this phase it is assumed that the number of client and server tasks is the same and is known beforehand, allowing solutions in which each client task is statically associated to a certain server task.
    - conceive and implement a basic scheme of synchronization between server tasks, where the access to KOS is protected by a single lock or semaphore.
    - implement the dispersion table used to save the data of KOS in RAM, this is, in this phase it is not necessary to develop mechanisms to persist the state of KOS in disk.
    
  - The second part of the project is focused in two componentes:
    - make the schemes of synchronization between tasks more eficient, pursuing the maximum level of parallelism possible, in the access to the buffer and in the access to the persisten data of KOS.
    - develop schemes to guarantee the persistent of data saved in KOS through the API of the UNIX filesystem.
    
### Design and implementation of the solution of the first part of the project

As mentioned before, in the first part of the project it will be necessary to implement the base architecture of the project, without yet including the mechanisms for persistence and implement the basic solutions of synchronization. Next it is given additional information about the data structures used to save the state of KOS in volatile memory, and about the mechanisms of synchronization between tasks.

#### Dispersion tables used to save the data of KOS

Figure 3 (see it in the PDF) shows the scheme of the dispersion table used to save in volatile memory each partition of data of KOS. Each table is implemented through a vector of connected lists. The vector has a number of fixed elements (specified through the constant HT_SIZE). A pair <key, value> that belongs to the partition associated to the dispersion table is inserted in the list in the position i of the vector, where i is determined through a dispersion function. This function converts each character of the key in an integer, and sums each of these values in an accumulating variable, called i. The position of the vector of lists to access is determined with the result of: i%HT_SIZE.

#### Buffer to the communication between client and server tasks

For the communication between client and server tasks only valid solutions will be considered, where the tasks communicate indirectly through a buffer shared in memory.

In this first phase it is assumed that that there are the same number of client and server tasks, and that this value is known beforehand, specified by the constant NUM_THREADS.

The in-memory buffer should be materialized through a vector of size NUM_THREADS, where each element of the vector is statically associated to a pair <client task i, server task i>. This means that the request of a certain client task are always processed by the same server task. Each element of the vector should contain enough information to allow synchronization between corresponding client and server tasks.

The diagram in Figure 4 (see it in the PDF) shows the solution aforementioned. To synchronize the activities of a client task and the corresponding server task, the solution to develop should maximize the efficiency of the synchronization between each pair of client/server task. The server tasks should block itselves if there are no requests to process, and the client tasks shouls block itselves waiting for the answer to the own requests; so, no task should stay in active wait.

#### Synchronization between server tasks to access the data saved in KOS

In this first phase of the project it is enough to develop simple solutions (for example, based in the use of a single lock/semaphore) to synchronize the access of the server tasks to the data saved in KOS. One of the objectives of the second part of the project is to improve the performance of the system by integrating sychronizations schemes that will allow to reach higher parallelism levels.

Before starting to process any request from the buffer, each server task should execute a call to the function _delay()_, defined by the files: _include/delay.h_ and _kos/delay.c_. This function injects a delay (to simulate, for example, the indeterminism of an operating system), suspending the execution of the task for a period of time (configurable in the file _delay.c_) and allowing to exercise the mechanisms of synchronization between the server tasks. 

### Compile the code

In the folder _/kos/_, open a terminal and execute the command **_make_**, to compile all the _.c_ files.

### Testing

In the foldes _/tests/_, open a terminal, execute the command **_make_**, to compile all the _.c_ files, and then execute the command **_sh doAllTests.sh_**.

## Part 2

### KOS: Key value store of the OS course

Now we assume that:
  
  - The number of clients is different, and tendentially higher, of the number of servers.
  
  - KOS is persistent, this is, the data stored in it survives the activities that created or accessed them.
  
The new functionalities are specified in a way to allow the reuse of what was already developed in the first part of the project: all conceptual characteristics are maintained and the general architecture of KOS (Figure 1 and 2 of the PDF of the first part), as well as the structure of data of each partitiion (Figure 3 of the PDF of the first part). The model of interaction client-server is also the same:

  _The client tasks should not access directly the state of KOS. Instead, this tasks should interact with the server tasks using the functions specified in the file_ kos_client.h_._
  
The new functionalities will affect the management of the communication between clients and servers, in the control of concurrency in the access to the data saved in KOS in memory and in the persistent storage of the data.

### Requisites of the second part of the project

#### Concurrency

It is considered in this part, in a more realistic way, that the number of clients is different from the number of servers (tendentially higher).

**Number of Clients != Number of Servers**

In the first part of the project there was a one-to-one correspondence between a client and it's server. It is now intended that the buffer of communication between clients and servers will transmit requests of M clients to N serves, being M not equal to N.

A client puts a request of service in the buffer, being attended by a server that is available in that moment. Established an association between a server and a client for the attendance of a request the interaction between both - allocation of buffer slots, model of synchronization, transference of data through the buffer - it should be adjusted (relatively to what was developed in the first part) to allow a dynamic connection between the client and the server of the client's request.

In the initialization it is assumed that the servers are up first than the clients and wait for the client's requests.

**Parallelism in the access to KOS**

Increasing the number of clients, the load in the servers, measured by the number of requests, tends to increase. That is why it is important to minimize containment between servers in the access to data of KOS that is in memory.

Keeping the model of data of KOS and the operations in it involved - _get_, _put_, _remove_, _getAllKeys_ - there are multiple approaches to increase concurrency. Some options are indicated below:

  - Allow parallel accesses to different partitions;
  - Allow reading accesses (_get_, _getAllKeys_) parallel in one partition;
  - Allow parallel accesses to distinct lists in a partition;
  - Allow search sequences (scanning of a list before reaching  the local of the _put_ or _remove_)) and reading accesses parallel in a partition.
  
It should be noted that the chosen option has implications in the management of concurrency of accesses to the filesystem to propagate the changes to KOS in memory.

#### Persistence of data in KOS

It is intended that the data of KOS is stored in a persistent way in the filesystem of UNIX.

Each partition should be stored in a file called _fshardId_. The file is updated as the partition in memory is changed. The data flow of KOS between the memory and the filesystem obeys the following rules:

  - The initialization of each partition in memory is done from the corresponding file in the phase of initialization of the system.
    - If that file does not exist it is assumed that the partition has no data, and it should be initialized empty in memory and the corresponding file created and initialized.
  - The reading operations (_get_, _getAllsKeys_) do not change the partitions in memory, so there is no change in the file-partition.
  - The writing is atomic, this is, the operation of writing due from the execution of a request _put_ or _remove_ is performed in the partition in memory and it is immediately propagated to the file-partition.
    - An operatin that implies a write (_put_, _remove_) is only considered concluded after the writing of the information in memory and in the file. It is considered that the write in the file was performed after the return of the call to the function of the API of the UNIX filesystem that performs the writing.
    
Since the pairs _Key-Value_ continue to be manipulated by the servers in the partitions in memory, the data to store in the file-partition may follow a structure more simple and compact:

  _Number of pairs_ Key-Value _stored inthe file_ - int NPKV - _followed by the sequence of the pairs_ Key-Value _stored with in no particular order (option P0)._
  
This solution, although very simple, forces to search the pairs <key, value> that are intended to be accessed - to delete (_remove_) or to update (_put_) - in a sequential scanning of the file. After this is implemented and the persistence of KOS is successfuly tested, more efficient implementation alternatives to allow the sequential scanning of the files and keep them compact should be investigated.

The following optimizations are to be considered:

  - P1; storage in the primary memory of the position (offset)of the register <key, value> into file, avoiding scanning.
  - P2; periodic compression of the file, eliminating empty records as a result of _removes_.
  - P3; reoccupation of empty <key, value> slots in new inserts, optimizing the addresses of these positions by storing their offset in the file that is in the list or in the bitmap residing in memory.
  
### Compile the code

In the folder _/kos/_, open a terminal and execute the command **_make_**, to compile all the _.c_ files.

### Testing

In the foldes _/tests/_, open a terminal, execute the command **_make_**, to compile all the _.c_ files, and then execute the command **_sh doAllTests.sh_**.
