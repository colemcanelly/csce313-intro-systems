[![Open in Visual Studio Code](https://classroom.github.com/assets/open-in-vscode-c66648af7eb3fe8bc4f294546bfd86ef473780cde1dea487d3c4ff354943c9ae.svg)](https://classroom.github.com/online_ide?assignment_repo_id=8550489&assignment_repo_type=AssignmentRepo)
# <p align="center">PA1: Client-Server IPC using Named Pipes<p>

### Introduction:

In this assignment, you will write a client program that connects to a given server. 
The server hosts several electrocardiogram (ECG) data points of 15 patients suffering from various cardiac diseases. The client has to communicate with the server such that it can fulfill two main objectives:

1. Obtain individual data points from the server.
2. Obtain a whole raw file of any size in one or more segments from the server.

The client has to send properly-formatted messages to the server using a communication protocol that the server defines to implement this transfer functionality.  

## Tasks

- [ ] [Run server as a child of the client](#run-server-as-a-child-of-the-client)
- [ ] [Request data points](#requesting-data-points)
  - [ ] [Single data point](#single-data-point)
  - [ ] [1000 data points](#1000-data-points)
- [ ] [Request files (with and without differing buffer capacity)](#requesting-files)
  - [ ] [CSV file](#csv-file)
  - [ ] [binary file (of differing sizes)](#binary)
  - [ ] [Experiment](#experiment)
- [ ] [Request a new channel](#requesting-a-new-channel)
- [ ] [Close channels](#closing-channels)
- [ ] [Report](#report)

See the PA1 module on Canvas for further details and assistance.

<!------------------------------------------------------------------------------------------------------------->

## Flags

- `-p <patient no>` : `[1, 15]` : request a specific patient (defaults to 1000 points)
- `-t <time>` : `[0.00, 15.996]` : request a specific time record
- `-e <ecg no>` : `[1, 2]` : request a specific ecg record
- `-f <file name>` : request the given file
- `-m <size>` : change buffer size to the given amount (bytes)
- `-c` : if present, a new channel is created

<!------------------------------------------------------------------------------------------------------------->

## Run server as a child process
###### ***15 points***

Run the server process as a child of the client process using `fork()` and `exec()` so you don't need 2 terminals to execute the program. Additionally, make sure that the server does not continue to run after the client terminates; to do this, send a `QUIT_MSG` to the server for each open channel and call the `wait()` function.

### Connecting to the server:

To connect to the server, the client has to create a `FIFORequestChannel` instance with the same name as the server, but with `CLIENT_SIDE` as the second argument:

```cpp
FIFORequestChannel chan ("control", FIFORequestChannel::CLIENT_SIDE);
```

This effectively sets up a communication channel over an OS-provided IPC mechanism called a `named pipe`, created by calling the `mkfifo()` system call, allowing processes to `read` or `write` information from the pipe. For this programming assignment we can read and write data to the channel with the `cread()` and `cwrite()` methods. After creating the channel, the server enters an infinite loop that processes client requests until termination.

> Note: Multiple channels can be created and used to expedite transfers.

<!------------------------------------------------------------------------------------------------------------->

## Requesting data points
###### ***15 points***

### Request Format:

The server hosts the `./BIMDC/` directory which contains 15 files, one for each patient. The files hold ECG records for a duration of 1 minute, with a data point every 4ms, resulting in 15000 data points per file. Any given row has 2 ECG records; `ecg1` and `ecg2`. The request format is given in `common.h` as a `datamsg`. The client constructs a `datamsg` object, and then sends this object across the channel through a buffer. A `datamsg` has the following fields:

- ***PatientID***: Which patient to pull data for `[1, 15]`
- ***Time***: What time point to send `[0.00, 15.996]`
- ***ECG***: Which ECG record to send `[1, 2]`

The following will run the client to request `ecg2` for patient 10 at time 59.004:

```shell
$ ./client -p 10 -t 59.004 -e 2
```

When sending the request, the client would create the following `datamsg` object:

```cpp
datamsg dmsg(10, 59.004, 2); // [replace with variables holding these values]
```

### Single data point:
Begin by requesting 1 data point from the server and displaying it to `stdout` by running the client using the following bash command:

```shell
$ ./client -p <patient no> -t <time in seconds> -e <ecg no>
```

> You should use the Linux function `getopt()` to collect the arguments.

### 1000 data points:
After requesting 1 data point, request the first 1000 data points for a patient (`ecg1` and `ecg2`), collect the reponses, and put them in a file called `x1.csv`. Compare the file against the corresponding data points in the original file and check that they match. To collect the larger data sample you can use the following bash command line:

```shell
$ ./client -p <patient no>
```  

<!------------------------------------------------------------------------------------------------------------->

## Requesting files
###### ***Total: 35 points***

### Making requests:

To avoid depleting available memory, we limit the amount of data sent in each transfer through the file buffer. The `buffercapacity` variable in `client.cpp` and `server.cpp` allows us to change the size of the buffer; the default value is `MAX_MESSAGE` (256Bytes) defined in `common.h`. This value can be changed by simply adding the command line argument `-m` as follows:

```shell
$ ./client -m <size> # number of bytes e.g. 5000
```

This adjustment must be done for both the client and server for the change to be effective. This allows us to request certain *chunks* of a file by giving a start and end byte `[start, end]`; given our buffer was 5000 Bytes, we could request the first *chunk* of the file like `[0, 5000]`. You can perform this repeatedly to transfer the whole file in several smaller chunks. To request a file, the client will need to package the following information in a message:

- ***Starting offset***: The data type is `__int64_t` to allow for ranges of data that 32 bits might not allow for.
- ***N_bytes***: Number of bytes zdto transfer after the starting offset. The data type is `int`, as if you have large files they will be broken into chunks.
- ***Name***: Null-terminate string, relative to the directory `./BIMDC/`

If you wanted to retrieve 30 bytes from a file named "practice.txt" at an offset of 100 you would construct the following `filemsg` object, and then place it in a buffer as shown below.

```cpp
filemsg msg(100, 30);                             // (offset, length)
string fname = "practice.txt"                     // Name of requested file

int len = sizeof(filemsg) + (fname.size() + 1);   // Get the total size of the package
char* buffer = new char[len];                     // Create a buffer
memcpy(buffer, &msg, sizeof(filemsg));            // Put the filemsg into the buffer
strcpy(buffer + sizeof(filemsg), fname.c_str());  // Put the file name in the buffer
channel.cwrite(buffer, len);                      // write buffer to the server
```

These can also be changed by using the object accessors `msg.offset` and `msg.length`. When sending a message to the server, we will send a buffer containing **a) the constructed `filemsg` object**, followed by **b) a null-terminated string** which holds the file name we are requesting. There is not a fixed-length field for the file name, as if we were to use a variable you would have to know the size upon compile-time, which is impossible. The format can just be thought of a header (the `filemsg` object), followed by a variable-length payload, which is the file name. Upon recieving this request, the server will respond with the appropriate chunk of the requested file.

Be sure to take account for the fact that you are receiving portions of the file in repsonse to each request, so you should plan accordingly so that your file data ends up in the right place.

In a situation where you request a 400 Byte file, and the buffer size is 256 Bytes, we would need to know the file size ahead of time, so that we can adjust the length of the requests accordingly. To achieve this, the client should first send a request for the size of a given file by setting the offset and length both to 0, the server responds with the length in a `__int64_t` variable.

```cpp
// gets file size:
filemsg msg_size(0, 0);

// gets first chunk:
filemsg msg1(0, 256);   // Start = 0, length 256

// gets second chunk:
filemsg msg2(256, 144); // Start 256, length 144
```

### CSV file:
###### ***20 points***

Request a file from the server side using the following bash command:

```shell
$  ./client -f <file name>
```

> The file does not need to be one of the `*.csv` files currently in `./BIMDC/`, any file in the directory can be requested by the client.

The steps for requesting a file are as follows:

1. Send a request for the file length
2. Send corresponding requests for the file contents
3. Put the received file under the `./received/` directory with the same name as the original file
4. Compare the file against the original using the linux command `diff` and demonstrate that they are exactly the same
5. Measure the time for the transfer for different file sizes (using `truncate -s <s> test.bin` to create a `<s>` bytes empty file) and put the results in the report as a chart.

### Binary:
###### ***10 points***

Make sure to treat the file as a binary, as we will use the same program to transfer many types of files (music, ppt, pdf, etc.). Putting the data into an `STD::string` will not work because C++ strings are `NULL` terminated. To demonstrate that your file transfer is capable of handling binary files, make a large empty file under the `./BIMDC/` (as seen in #5), transfer that file, and compare them (#4) to make sure they are identical.

### Experiment:
###### ***5 points***

Experiment with transferring large files (~100MB), and document the required time. What is the main bottleneck? Can you change the transfer time by varying the bottleneck?

<!------------------------------------------------------------------------------------------------------------->

## Requesting a New Channel
###### ***15 points***

The flag to create a new channel, `-c`, can be used in tandem with nay other command, all communication for that execution will occur over the new channel. This feature will be used more thoroughly in future assignments dealing with multi-threaded clients. 

Ask the server to create a new channel for the client to access by sending a `NEWCHANNEL_MSG` request, the server will create the channel and return the channel name, allowing you to join that channel on the client side; this process can be seen in the server's `process_new_channel()` function. After the channel is created, you will have to process any other requests the user has passed in through the CLI arguments.

<!------------------------------------------------------------------------------------------------------------->

## Closing channels
###### ***5 points***

Ensure that there are **no open connections** and **no temporary files** remaining after execution has finished. The server will clean up these resources as long as you send a `QUIT_MSG` at the end for new channels that were created.

> The given `client.cpp` should already do this for the control channel.

<!------------------------------------------------------------------------------------------------------------->

## Report
###### ***15 points***

Write a report describing the design and the timing data you collected for data points, text files, and binary files. Show the time of file transfers as a function of varying file sizes and buffer capacity.