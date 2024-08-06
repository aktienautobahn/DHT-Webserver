
#include "node.h"
#include "sockets_setup.h"
#include "chord_processor.h"


/*
*
*  The program expects 3; otherwise, it returns EXIT_FAILURE.
*  Call as without CHORD DHT Node functionality:
*
*  ./build/webserver self.ip self.port
*
*  The parameter 4 is added for expanding the Webserver functionality with CHORD DHT Node
*  For the CHORD DHT Node functionality there are environmental variables should be used as follows:
*  e.g.: PRED_ID=49152 PRED_IP=127.0.0.1 PRED_PORT=2002 SUCC_ID=49152 SUCC_IP=127.0.0.1 SUCC_PORT=2002
*
*  Call as with CHORD DHT Node functionality:
*  ./build/webserver self.ip self.port self.nodeid
*/
int main(int argc, char** argv) {
    if (argc < 3) {
        return EXIT_FAILURE;
    }

    struct NetworkNodes node_data;

    // derive server socket addresses
    struct sockaddr_in addr = derive_sockaddr(argv[1], argv[2]);

    // Set up a TCP socket.
    int stream_socket = setup_stream_socket(addr);

    // Set up an UDP socket:
    int datagram_socket = setup_datagram_socket(addr);

    //Check, if simple webserver must be started
    if (argc == 3){

        //Init the node_data struct
        node_data.self_id = 0;
        memset(&node_data.succ, 0, sizeof(node_data.succ));
        memset(&node_data.pred, 0, sizeof(node_data.pred));
        
        //Start chord_processor
        chord_processor(addr, stream_socket, datagram_socket, node_data);
    }
   
    //Start chord server
    node_data = derive_nodes_data(argv[3]);
    chord_processor(addr, stream_socket, datagram_socket, node_data);
    
    return EXIT_SUCCESS;
}
