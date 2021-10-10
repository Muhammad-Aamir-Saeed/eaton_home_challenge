#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <netinet/in.h >	// struct sockaddr_in
#include <arpa/inet.h >	// htons(), htonl()
#include <sys/socket.h>
#include <linux/in.h>
#include <unistd.h>
#include <mutex>
#include <cstring>
#include <string>
#include <iostream>
#include <vector>
#include <map>

//storage of measurements received per device id 
//will also be responsible for keeping the total number of messages received
std::map<uint8_t, size_t> per_device_messages;
std::mutex mtx01;

//structure for storing the socket details
typedef struct
{
	int sock;
	struct sockaddr address;
	int addr_len;
}
connection_t;

//decode the messages that have been send by the various devices
void message_decode(const std::string &buffer, uint8_t &device_id, size_t &measurementcount)
{
	std::string s = buffer;
	std::vector<float> measurements;	//TODO make this a global or pass it to the main thread, currently gets deleted when the function is ended
	std::string delim = ":";
	auto start = 0 U;
	auto end = s.find(delim);

	while (end != std::string::npos)
	{
		if (measurementcount == 0)	//the first string contains the device id
		{
			device_id = (uint8_t) atoi((s.substr(start, end - start)).c_str());
		}
		measurements.push_back(stof(s.substr(start, end - start)));

		start = end + delim.length();
		end = s.find(delim, start);
		measurementcount++;
	}
	measurements.push_back(stof(s.substr(start, end - start)));
	/*to print the individual measurements of the message that has been send*/

	//uncomment to print all the measurements

	//std::cout<<"device id: "<< device_id<<std::endl;
	//
	//for(int i =0;i<measurementcount+1;++i)
	//{
	//	std::cout<<measurements.at(i)<<std::endl;
	//}
	//std::cout << s.substr(start, end);

}

void *process(void *ptr)
{
	char *buffer;
	int len;
	connection_t * conn;
	uint8_t device_id;
	size_t device_message_count = 0;
	size_t number_of_measurements = 0;

	if (!ptr) pthread_exit(0);
	conn = (connection_t*) ptr;

	/*read length of message */	//some devices can send more than one measurement of data
	read(conn->sock, &len, sizeof(int));
	len = ntohl(len);	// Ensure host system byte order
	if (len > 0)	//we have successfully received a data packet from a device
	{
		buffer = (char*) malloc((len + 1) *sizeof(char));	//create a buffer so as to store the data
		buffer[len] = 0;	//make it zero

		/*read message */
		read(conn->sock, buffer, len);

		//breaking down the message to individual measurements 
		message_decode(std::string(buffer), device_id, number_of_measurements);
		std::cout << "device " << (int) device_id << " just send: " << number_of_measurements << " message/s now\n";
		std::cout << "--------\n";

		free(buffer);

		//lock the threads
		mtx01.lock();
		//find which device has previous measurements and update accordingly
		if (per_device_messages.find(device_id) != per_device_messages.end())
		{
			device_message_count = per_device_messages[device_id];
		}

		per_device_messages[device_id] = ++device_message_count;

		//count the number of total measurements that have been received
		int sum = 0;

		for (const auto &pair: per_device_messages)
		{
			std::cout << "device " << (int) pair.first << " has a send a total of: " << pair.second << " message/s\n";
			// printf("device %d has send a total of: %ld messages\n", pair.first, pair.second);
			sum += pair.second;
		}
		std::cout << "number of measurements received from all devices: " << sum << std::endl;
		std::cout << "\n--------------\n";
		mtx01.unlock();
	}

	/*close socket and clean up */
	close(conn->sock);
	free(conn);
	pthread_exit(0);
}

int main(int argc, char **argv)
{
	int sock = -1;
	struct sockaddr_in address;
	int port;
	connection_t * connection;
	pthread_t thread;

	/*check for command line arguments */
	if (argc != 2)
	{
		fprintf(stderr, "usage: %s port\n", argv[0]);
		return -1;
	}

	/*obtain port number */
	if (sscanf(argv[1], "%d", &port) <= 0)
	{
		fprintf(stderr, "%s: error: wrong parameter: port\n", argv[0]);
		return -2;
	}

	/*create socket */
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock <= 0)
	{
		fprintf(stderr, "%s: error: cannot create socket\n", argv[0]);
		return -3;
	}

	/*bind socket to port */
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	if (bind(sock, (struct sockaddr *) &address, sizeof(struct sockaddr_in)) < 0)
	{
		fprintf(stderr, "%s: error: cannot bind socket to port %d\n", argv[0], port);
		return -4;
	}

	/*listen on port */
	if (listen(sock, 10) < 0)
	{
		fprintf(stderr, "%s: error: cannot listen on port\n", argv[0]);
		return -5;
	}

	std::cout << argv[0] << ": is up and running\n";
	std::cout << "--------------\n\n";

	while (1)
	{
		/*accept incoming connections */
		connection = (connection_t*) malloc(sizeof(connection_t));
		connection->sock = accept(sock, &connection->address, (socklen_t*) &connection->addr_len);
		if (connection->sock <= 0)
		{
			free(connection);
		}
		else
		{ /*start a new thread but do not wait for it */
			pthread_create(&thread, 0, process, (void*) connection);
			pthread_detach(thread);
		}
	}

	return 0;
}