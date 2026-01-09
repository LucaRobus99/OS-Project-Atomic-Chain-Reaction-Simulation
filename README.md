# ☢️ Atomic Chain Reaction Simulation



## 📖 Project Overview

This software is a multi-process simulation of a nuclear chain reaction. It was developed to demonstrate the practical application of advanced **Unix/Linux system programming** concepts.

The simulation models the interaction between atoms, activators, and power supplies, where each entity is an independent process. The system manages complex synchronization and communication through **System V IPC (Inter-Process Communication)** mechanisms to simulate energy production, meltdowns, and system failures.

---

## 🛠 Technical Stack & IPC Mechanisms

* **Language:** C (C11 standard)
* **Platform:** Linux (Tested on Ubuntu, Debian, and WSL)
* **Build Tool:** GNU Make
* **IPC Tools:**
* **Shared Memory:** Used for real-time status updates and global system state.
* **Semaphores:** Implemented to prevent race conditions during atom interactions.
* **Message Queues:** Used for asynchronous communication between the Master and child processes.
* **POSIX Signals:** Robust handling of system-wide events (termination, interrupts, state changes).



---

## 🏗 System Architecture

### 1. Master Process (The Controller)

The orchestrator of the entire simulation. Its responsibilities include:

* Memory allocation and IPC resource initialization.
* Parsing configuration files to set simulation physics.
* Spawning the process hierarchy and monitoring life cycles.
* Reporting final statistics and ensuring a **leak-free cleanup** of resources.

### 2. Atom Processes

Represent the core logic of the reaction. They utilize shared memory to sense nearby entities and synchronize their state changes via semaphores to simulate fission and energy release.

### 3. Activator Process

Acts as the external stimulus for the reaction, managing the flow of activation energy through message queues and handling signal-based interruptions.

### 4. Power Supply (*Alimentazione*)

Monitors the simulated grid, reacting to energy spikes or shortages, and triggering "Blackout" or "Overload" states based on the Master's configuration.

---

## 📂 Project Structure

```text
.
├── master.c                # Simulation entry point and orchestrator
├── Makefile                # Automated build system
├── *.conf                  # Scenario-based configuration files
├── includes/               # Modular header files
│   ├── atomo.h
│   ├── attivatore.h
│   └── ...
├── src/                    # Core logic implementations
│   ├── atomo.c
│   ├── attivatore.c
│   └── alimentazione.c
├── utils/                  # IPC wrappers and utility functions
│   ├── message_queue.c     # System V Message Queue abstraction
│   ├── semaphore.c         # Semaphore P/V operation wrappers
│   └── shared_memory.h     # Shared structure definitions
└── Relazione.pdf           # Comprehensive technical report (Italian)

```

---

## 🚀 Getting Started

### Prerequisites

A Linux environment with `gcc` and `make` installed.

### Compilation

Build the entire project using the provided Makefile:

```bash
make

```

### Execution

Run the simulation by passing a specific configuration scenario:

```bash
./master <scenario>.conf

```

Available scenarios include:

* `Blackout.conf`: Simulates power grid failure.
* `Explode.conf`: Simulates uncontrolled chain reaction.
* `Meltdown.conf`: Simulates critical core temperature.
* `Timeout.conf`: Simulates a standard timed run.

---

## 🎓 Learning Outcomes

This project covers the fundamental pillars of **Concurrent Computing**:

* **Process Lifecycle:** Advanced use of `fork()`, `exec()`, and `waitpid()`.
* **Concurrency Control:** Mastering Mutex/Semaphore logic to avoid deadlocks.
* **Asynchronous Programming:** Designing robust signal handlers for graceful shutdowns.
* **Resource Management:** Designing systems that clean up kernel-level IPC structures even after fatal errors.

---

Perfetto! Ecco la sezione **Contributors** aggiornata con il link diretto al profilo GitHub di Davide Robustelli. Puoi inserirla nel tuo file `README.md` o nel file `CONTRIBUTING.md`.

---

## 👥 Contributors

* **Davide Robustelli** — [@xDavikx](https://github.com/xDavikx)
* **Luca Robustelli** - [@LucRobus99](https://github.com/LucaRobus99)
---


## 📜 License

This project was developed for academic purposes as part of the Operating Systems course at the University of Turin. All rights reserved by the authors.
