#include <zmq.hpp>
#include <string>
#include <iostream>
#include <unistd.h>
#include <thread>

int main() {
    zmq::context_t context(1);
    zmq::socket_t socket(context, zmq::socket_type::stream);

    socket.connect("tcp://localhost:8080");

    zmq::message_t identity;
    socket.recv(&identity);
    zmq::message_t message;
    socket.recv(&message);

    std::string send_msg = "send msg ";
    int cnt = 0; 

    socket.send(identity, zmq::send_flags::sndmore);
    socket.send(zmq::message_t(send_msg + std::to_string(++cnt)));

    while (true) {
        socket.recv(&identity);

        zmq::message_t message;
        socket.recv(&message);

        std::string msg(static_cast<char*>(message.data()), message.size());
        std::cout << "Received message: " << msg << std::endl;

        if (message.size() > 0)
        {
            socket.send(identity, zmq::send_flags::sndmore);
            socket.send(zmq::message_t(send_msg + std::to_string(++cnt)));
        }
    }
    return 0;
}
