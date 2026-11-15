# file-transferring

## Requirements
* Linux machine
* g++ with C++ 20
* CMake 3.20
* Python 3.10

## How to setup
*Note: This setup for now runs server and client on the same machine. To run in different machine, edit `myTCPConfig::SERVER_IP` in `src/myTCPConfig.h` with the server's IP to run on. Each Python script has hardcoded IPs at the moment at the start of source files.*
*Note: All commands are assumed to run on root folder.*
1. Build the project with CMake on Linux
   * `cmake -B generated/build`
   * `cmake --build generated/build`
2. Run python script `scripts/gen_test_files.py` to generate files for testing
3. Run `generated/build/tcp_server` on your machine
4. Run client (through one of the methods in **Client tests** section) on the same machine

### Client tests
* `generated/build/tcp_client`: Intended client for regular users. Currently not compatible with `tcp_server`
* `scripts/tcp_test_file.py <filenames separated by space>`: Temporary replacement of `tcp_client` for testing
* `scripts/tcp_client_tests.py <tests separated by space>`: Compatible with `tcp_server`. Arguments: `exact_1024`, `1025_rejected`, `zero_length`, `die_mid_body`, `rst_on_read`, `kill_mid_transfer`, `slow_body`, `many_clients`, `split_header`

## How it works
Every request starts with a 4-byte big-endian frame that informs the receiver how many bytes the next chunk should be expected.

Client sends bytes of string representing file name (up to 1024 bytes, as defined in `myTCPConfig::BUFFER_SIZE`).
* If the client sends 0 bytes or 1025 bytes or more of filename, the connection will be closed.

The server, after receiving the file name, sends a `Protocol` response frame of 16 bytes.
* The first byte is 1 (true) if the filename matches one in the server's whitelist, 0 (false) if the filename is mismatched.
* Bytes at 1-7 are padded due to Protocol struct layout and do not have any meaning.
* Bytes at 8-15 indicate the size of the file to be sent. These bytes are little-endian, as the server is currently expected to be run on little-endian machines.
The server then continuously sends bytes representing content of the file.
* The server may send multiple chunks of up to 1024 bytes until it sends exactly all bytes of the file. This step will be skipped but the connection stays open if:
   * The file does not have size (or has 0 bytes)
   * The client sends mismatched filename

## Future updates
* CMake project `tcp_client` to be reworked to be compatible with `tcp_server`
* Proper logging and debugging system
* Implementing shutdown logic when signals are emitted
* Testing client and server communications through different machines
* Implementing TLS so that the bytes are encrypted before being sent

## License
MIT