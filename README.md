# isr-safe-data-pipe

A lightweight, ISR-safe, and thread-safe data pipe service designed for resource-constrained embedded C applications. It utilizes static memory allocation and features a deterministic "round-robin write" and "latest-snapshot read" policy.

## Features

* **Static Memory Only**: No dynamic allocation; safe for critical systems.
* **Non-Blocking**: ISR-safe and RTOS-safe atomic ownership transitions.
* **Deterministic**: Fixed channel counts and fixed-width payloads.
* **Policies**:
  * Write: Round-robin channel allocation.
  * Read: Always returns the latest available snapshot.

## Getting Started

1. **Initialization**: Call `service_pipe_init()` before any other operations.
2. **Handle Acquisition**: Obtain a handle using `service_pipe_get_handle(id)`.
3. **Data Access**: Use `service_pipe_access()` with `service_pipe_OP_TYPE_WRITE` or `service_pipe_OP_TYPE_READ`.

## Licensing

This project is licensed under the **Creative Commons Attribution-NonCommercial 4.0 International (CC BY-NC 4.0)** license. This means you are free to share and adapt the material for non-commercial purposes only, provided you give appropriate credit.
