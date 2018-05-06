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
 
**UNDER CONSTRUCTION**
