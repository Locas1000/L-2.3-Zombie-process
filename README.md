
# **Homework 2: Zombie Process Handler**

This project is an exploration of the process lifecycle in Linux, specifically focusing on the creation, detection, and management of zombie processes. It includes several command-line tools and a static library to demonstrate robust zombie prevention techniques.

## **Learning Objectives**

* Understand the origin and implications of zombie processes in a Unix-like operating system.
* Implement tools to scan the `/proc` filesystem and report on process states.
* Master signal handling, particularly `SIGCHLD`, for asynchronous process reaping.
* Develop a long-running daemon that correctly manages child processes without creating zombies.
* Encapsulate process management logic into a reusable static library.

-----

## **Features**

* **`zombie_creator`**: A utility to intentionally create a specified number of zombie processes for testing purposes.
* **`zombie_detector`**: A tool that scans the `/proc` filesystem, identifies all active zombie processes, and generates a detailed report, including parent process analysis.
* **`zombie_reaper`**: A program demonstrating three distinct strategies for reaping child processes to prevent zombies:
    1.  Explicit, blocking calls to `wait()`.
    2.  An asynchronous `SIGCHLD` signal handler.
    3.  Ignoring `SIGCHLD` to have the kernel perform automatic reaping.
* **`process_daemon`**: A robust background daemon that periodically spawns worker processes and uses a `SIGCHLD` handler to ensure no zombies are ever created. It also handles `SIGTERM` for a graceful shutdown.
* **`libzombie.a`**: A static library that provides a simple API (`zombie_init()`, `zombie_safe_fork()`) to easily add zombie prevention to any application.

-----

## **Directory Structure**

```
.
├── Makefile
├── README.md
├── DEMO.md
├── src
│   ├── process_daemon.c
│   ├── zombie.c
│   ├── zombie.h
│   ├── zombie_creator.c
│   ├── zombie_detector.c
│   └── zombie_reaper.c
├── tests
│   ├── test_creator.sh
│   ├── test_daemon.sh
│   ├── test_detector.sh
│   └── test_reaper.sh
```

-----

## **Compilation**

A `Makefile` is provided to handle all compilation. The following commands are available:

* **Build all programs and the library:**

  ```bash
  make
  ```

* **Clean up all compiled files:**

  ```bash
  make clean
  ```

-----

## **Usage**

All executables are created in the project's root directory.

* **Create 5 zombies:**

  ```bash
  ./zombie_creator 5
  ```

* **Detect all zombies on the system:**

  ```bash
  ./zombie_detector
  ```

* **Test reaping strategy 2 (SIGCHLD handler):**

  ```bash
  ./zombie_reaper 2
  ```

* **Start the background daemon:**

  ```bash
  ./process_daemon
  ```

* **Stop the background daemon:**

  ```bash
  killall process_daemon
  ```

-----

## **Testing**

The project includes a set of scripts in the `/tests` directory to automate testing. You can run the entire test suite with a single command.

* **Run all tests:**
  This command will build the project and then execute all test scripts in sequence.
  ```bash
  make run-tests
  ```