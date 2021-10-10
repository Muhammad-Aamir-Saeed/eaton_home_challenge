#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <string>
#include <random>

float get_random()//to generate random measurements;
{
    // Random seed
    std::random_device rd;
    // Initialize Mersenne Twister pseudo-random number generator
    std::mt19937 gen(rd());
    static std::uniform_real_distribution<> dis(0, 100); // rage 0 - 100
    return dis(gen);
}


int main(int argc, char ** argv)
{
	uint8_t deviceid; //get device id from the user
	int port;
	std::vector<float> measurements;
	int sock = -1;
	struct sockaddr_in address;
	struct hostent * host;
	int len;
	
	/* checking commandline parameter */
	if (argc != 5)
	{
		std::cout<<"usage: "<<argv[0]<< " hostname port_no number_of_measurements device_id\n";
		return -1;
	}

	/* obtain port number */
	if (sscanf(argv[2], "%d", &port) <= 0)
	{
		fprintf(stderr, "%s: error: wrong parameter: port\n", argv[0]);
		return -2;
	}

	/* create socket */
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock <= 0)
	{
		fprintf(stderr, "%s: error: cannot create socket\n", argv[0]);
		return -3;
	}

	/* connect to server */
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	host = gethostbyname(argv[1]); //get the host name
	if (!host)
	{
		fprintf(stderr, "%s: error: unknown host %s\n", argv[0], argv[1]);
		return -4;
	}
	memcpy(&address.sin_addr, host->h_addr_list[0], host->h_length);
	if (connect(sock, (struct sockaddr *)&address, sizeof(address)))
	{
		fprintf(stderr, "%s: error: cannot connect to host %s\n", argv[0], argv[1]);
		return -5;
	}
	
	//send it as one one long string that is composed of the device id and the measurements
	//the header file can be 8 bits to compose the device id followed by the remaining measurements
	/* number of measurements to send to the server */
	int no_measurements = atoi(argv[3]);

	
	
	deviceid = atoi(argv[4]);//get the device id from the user
	std::string buffer = std::to_string(deviceid);
	
	//generate random measurements and store them in a vector and then convert them to a string to be send
	for (int i = 0; i < no_measurements; ++i)
	{
		buffer += ":";
		measurements.push_back(get_random());
		buffer += std::to_string(measurements.at(i));

	}

	std::cout <<"data that is send to server: "<< buffer << std::endl;
	//std::cout << buffer.size() << std::endl;
	uint32_t dataLength = htonl(buffer.size()); // Ensure network byte order when sending the data length
	write(sock, &dataLength, sizeof(uint32_t));//first send the number of bytes the server will receive
	write(sock, buffer.c_str(), buffer.size());//then send the actual data

	/* close socket */
	close(sock);

	return 0;
}
