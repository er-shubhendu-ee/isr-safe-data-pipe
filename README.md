# isr-safe-data-pipe

A lightweight, ISR-safe, and thread-safe snapshot transport service designed for resource-constrained embedded C applications.

The library uses static memory allocation and deterministic ownership transfer to transport snapshots in a single direction between independent execution contexts.

Typical examples include:

* RTOS Task → ISR
* ISR → RTOS Task
* Supervisor → Control
* Control → Supervisor
* Service → Service

The service follows a **round-robin write** and **latest-snapshot read** policy while remaining completely non-blocking.

---

## Features

### Memory Model

* Static memory allocation only
* No dynamic allocation
* Fixed memory footprint
* Deterministic runtime behavior

### Concurrency

* Non-blocking operation
* ISR-safe ownership transitions
* RTOS-safe ownership transitions
* No locks required beyond short ownership-protection critical sections

### Data Model

* Fixed-width payload
* Fixed channel count
* Snapshot-oriented communication
* Latest state is preferred over historical state

### Access Policies

#### Write Policy

* Round-robin channel allocation
* First available channel wins
* Atomic ownership acquisition

#### Read Policy

* Returns the latest available snapshot
* If the latest snapshot is temporarily unavailable, the next latest available snapshot is selected

---

## Communication Model

A Service Pipe is a unidirectional snapshot transport.

```text
Producer ----> Pipe ----> Consumer
```

A single pipe instance represents a single direction of data flow.

The service does not provide duplex communication within a single pipe instance.

If two functional blocks must exchange information in both directions, two independent pipes shall be used.

```text
+----------+                 +----------+
| Block A  | ---- Pipe 1 --->| Block B  |
|          |<--- Pipe 2 -----|          |
+----------+                 +----------+
```

Examples:

```text
RTOS Task ----> ISR
RTOS Task <---- ISR
```

```text
Supervisor ----> Control
Supervisor <---- Control
```

Each direction should be assigned its own pipe instance.

---

## Design Philosophy

A PIPE is a transport medium only.

The design intentionally follows the behavior of a physical pipe.

A physical pipe is responsible for transporting fluid. It is not responsible for:

* Whether fluid exists
* Whether fluid is fresh
* Whether a pump has started
* Whether a consumer arrives too early
* Whether a consumer arrives too late

Similarly, Service Pipe is responsible only for transporting snapshots between producers and consumers.

The service does not track:

* Data validity
* Data freshness
* Data ownership
* Data consumption state

These concerns belong to the application layer.

---

## Intended Use

Service Pipe is intended to provide a lightweight snapshot transport mechanism
between independent execution contexts in embedded systems.

Examples include:

* RTOS Task → ISR
* ISR → RTOS Task
* Supervisor → Control
* Control → Supervisor

The service is intended for transporting state snapshots and control information.

The service is not intended to provide:

* Event delivery guarantees
* Message delivery guarantees
* Ordered message delivery
* Data persistence
* Acknowledgement mechanisms

---

## What Service Pipe Is Not

Service Pipe is not:

* FIFO
* Queue
* Mailbox
* Publish/Subscribe system
* Event transport mechanism

If ordered delivery, consumption tracking, guaranteed delivery, or event delivery semantics are required, a queue-based mechanism should be used instead.

---

## Read Semantics

A READ operation retrieves the latest available snapshot currently stored in the pipe.

The service does not determine whether a snapshot is meaningful.

Immediately after initialization and before the first WRITE operation, a READ may return the default contents of the pipe storage.

This behavior is intentional.

Applications are expected to ensure that producers begin publishing meaningful snapshots before consumers depend on them.

---

## Write Semantics

A WRITE operation publishes a complete snapshot into the pipe.

Channel allocation follows a round-robin policy to reduce contention between concurrent readers and writers.

The most recently written snapshot becomes the preferred candidate for subsequent reads.

---

## Typical Usage

### Initialization

```c
service_pipe_init();
```

### Allocate Pipe

```c
service_pipe_Handle_t hPipe;

hPipe = service_pipe_get_handle(PIPE_ID);
```

### Publish Snapshot

```c
service_pipe_Message_t message;

service_pipe_access(
    hPipe,
    service_pipe_OP_TYPE_WRITE,
    &message);
```

### Consume Snapshot

```c
service_pipe_Message_t message;

service_pipe_access(
    hPipe,
    service_pipe_OP_TYPE_READ,
    &message);
```

---

## Example Application Architecture

### Single Direction

```text
+-----------+      WRITE       +--------------+       READ       +-----------+
| Producer  | ---------------> | Service Pipe | --------------> | Consumer  |
+-----------+                  +--------------+                 +-----------+
```

### Duplex Communication

```text
+-----------+                    +-----------+
|  Block A  | ---- Pipe A -----> |  Block B  |
|           | <--- Pipe B ------ |           |
+-----------+                    +-----------+
```

---

## Building

### Standalone Build

```bash
cmake -B build
cmake --build build
```

### Build with Tests

```bash
cmake -B build -DSERVICE_PIPE_BUILD_TESTS=ON
cmake --build build
```

### Use as a Submodule

```cmake
add_subdirectory(service_pipe)

target_link_libraries(my_target
    PRIVATE
        service_pipe)
```

When used as a submodule:

* Standalone compatibility layer is disabled
* Test executable is disabled by default

---

## Testing

The project includes a standalone test suite covering:

* Initialization
* Handle allocation
* Handle exhaustion
* Single write / read
* Multiple write / read
* Invalid arguments
* Invalid operations
* Pipe isolation
* Read-before-write behavior

Run:

```bash
./service_pipe_test
```

---

## Disclaimer

This software is provided for educational, experimental, research, and reference purposes.

This software is not intended, certified, or guaranteed to be suitable for any specific purpose.

The author makes no representation or warranty, express or implied, regarding:

* Correctness
* Completeness
* Reliability
* Suitability for a particular application
* Fitness for commercial use
* Fitness for safety-critical systems
* Fitness for mission-critical systems

The software is provided **"AS IS"**, without warranty of any kind.

Use of this software is entirely at the user's own risk.

The user is solely responsible for:

* Verification
* Validation
* Testing
* Integration
* Deployment
* Regulatory compliance
* Safety analysis

The author does not guarantee that the software:

* Is free from defects
* Is free from design errors
* Meets any performance requirement
* Meets any safety requirement
* Meets any legal or regulatory requirement

Under no circumstances shall the author be liable for any claim, damage, loss, injury, liability, cost, or expense arising from the use, misuse, modification, distribution, integration, or inability to use this software.

This limitation includes, but is not limited to:

* Direct damages
* Indirect damages
* Consequential damages
* Incidental damages
* Special damages
* Economic losses
* Loss of business
* Loss of profits
* Loss of revenue
* Loss of data
* Loss of production
* Equipment damage
* Property damage
* Personal injury

By using this software, the user acknowledges and accepts these terms.

## Licensing

This project is licensed under the **Creative Commons Attribution-NonCommercial 4.0 International (CC BY-NC 4.0)** license.

You are free to:

* Share
* Copy
* Redistribute
* Adapt
* Modify

for non-commercial purposes, provided appropriate attribution is given.
