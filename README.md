# Eaton challenge

**1) The challenge**
	Have a server that is receiving data from multiple devices where each device has a unique ID and sends several measurements to the server via the ethernet.

**2) My proposed implementation:**

* A Server application listening on a TCP port from various devices
* Use of multithreading with a mutex

**3) Details:**

I develop a simple server that can handle multiple requests from the devices using multiple threads. I use the TCP protocal as it is the most reliable. The communication protocol between the server and the devices is quite basic. Where when one device would like to send information to the server, it would first send the number of bits that the actual data it is composed of and then send the actual data. As different devices might send different number of measurements, we can compute at run time the size of the measurements that why I  send the size of the data before the actual data.

The devices send information that is composed of their device id and all its measurements which is bundled together as one long string. The information is then broken down at the server end using a delimiter which is known by the server before hand.



**4) Running the project**

The task has a simple makefile which generates the binaries of both the server and the devices. I have also included a basic script that tells the device program to send data to the server. 

To run the project run the server binary with the a valid socket number. e.g. ```./server 6000``` . To run the devices open another terminal, and run the ./device application together with the host, the listening socket, the device id and the number of measurements it should send. (As it is a test application i did not fix the device id's and the number of measurements a device should send). e.g ```./device localhost 6000 1 2``` . The device program can also be runned by the script which accepts the listening port number of the server and runs a small test. 

The script should also be runned in another terminal window separate from the server terminal. Also the server application should be started first before the device application else the device would not be able to send any data.


