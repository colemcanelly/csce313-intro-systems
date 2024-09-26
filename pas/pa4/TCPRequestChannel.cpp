#include "TCPRequestChannel.h"

using namespace std;


/**
 * @brief   Construct a new TCPRequestChannel::TCPRequestChannel object
 * 
 * @param   _ip_address 
 * @param   _port_no 
 * 
 * @note    Implementation:
 *          if server
 *              create a socket on the specified
 *              -   specify domain, type, and protocol
 *              bind the socket to addr set-ups listening
 *              mark socket as listening
 *          if client
 *              create a socket on the specified
 *              -   specify domain, type, and protocol
 *              connect socket to the IP addr of the server
 */
TCPRequestChannel::TCPRequestChannel (const std::string _ip_address, const std::string _port_no) {
    cout << "new channel constructor" << endl;
    if (_ip_address.empty()) {
        // server
        // struct sockaddr_storage their_addr;
        // socklen_t addr_size;
        struct addrinfo hints, *res;

        // first, load up address structs with getaddrinfo();
        
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;      // use IPv4 or IPv6, whichever
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;    // fill in my IP for me

        getaddrinfo(NULL, _port_no.c_str(), &hints, &res);

        // make a socket, bind it, and listen on it

        sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        bind(sockfd, res->ai_addr, res->ai_addrlen);
        listen(sockfd, 10);

        freeaddrinfo(res);

    } else {
        // client
        struct addrinfo hints, *res;

        // first, load up address structs with getaddrinfo();

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;      // use IPv4 or IPv6, whichever
        hints.ai_socktype = SOCK_STREAM;

        // we could put "80" instead on "http" on the next line:
        getaddrinfo(_ip_address.c_str(), "http", &hints, &res);

        // make a socket:

        sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

        // connect it to the address and port we passed in to getaddrinfo():

        connect(sockfd, res->ai_addr, res->ai_addrlen);
        freeaddrinfo(res);
    }

}

TCPRequestChannel::TCPRequestChannel (int _sockfd) {
    sockfd = _sockfd;
}


/**
 * @brief   Destroy the TCPRequestChannel::TCPRequestChannel object
 * 
 * @note    close the sockfd
 */
TCPRequestChannel::~TCPRequestChannel () {
    close(sockfd);
}

/**
 * @brief Accept connection
 * 
 * @return the sockfd of client 
 */
int TCPRequestChannel::accept_conn () {
    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof(their_addr);
    return accept(sockfd, (struct sockaddr*)&their_addr, &addr_size);
}

// read/write, recv/send
int TCPRequestChannel::cread (void* msgbuf, int msgsize) {
    return recv(sockfd, msgbuf, msgsize, 0);
}

int TCPRequestChannel::cwrite (void* msgbuf, int msgsize) {
    return send(sockfd, msgbuf, msgsize, 0);
}
