# Communication Protocol
---
## Description

This project is a simple implementation of a custom communication protocol in C. It defines a specific frame structure for messages, calculates a checksum for integrity, and includes a state machine parser to validate and interpret incoming byte streams.

The `main.c` file includes a series of tests to validate the protocol's functionality:
1.  **Test 1:** A normal transmission and reception test where a message is framed, "sent" (written to `channel.bin`), "received" (read from `channel.bin`), and verified.
2.  **Test 2:** A simulated corruption test where a frame's data is intentionally altered before being written to `corrupted.bin`. The receiver then correctly identifies the message as corrupt by validating the checksum.
3.  **Test 3:** A state machine test that feeds the parser an invalid byte stream to ensure it correctly identifies the corrupted frame.

---
## Features

* **Frame Structure:** Defines a `protocol_frame_t` with the following structure:
    * `START_BYTE` (0x55)
    * `message_id` (1 byte)
    * `length` (1 byte, max payload 256)
    * `payload` (up to 256 bytes)
    * `checksum` (1 byte)
* **Checksum:** Uses a simple XOR checksum (`calculate_checksum`) for all data bytes (ID, length, and payload) to ensure data integrity.
* **State Machine Parser:** Implements a simple state machine (`parse_byte`) to process bytes one at a time and validate the frame structure and checksum.

---
## Building

This project uses CMake. To build the executable:

1.  Clone the repository.
2.  Go to  build directory:
    ```sh
    cd build
    ```
3.  Run CMake to configure the project:
    ```sh
    cmake ..
    ```
4.  Run Make to compile the source code:
    ```sh
    make
    ```
This will create an executable named `com_protocol` in the `build` directory.

---
## Running Tests

After building, you can run the built-in tests directly from the root directory:

```sh
./build/com_protocol
