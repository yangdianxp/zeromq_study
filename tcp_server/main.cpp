#include <zmq.hpp>
#include <string>
#include <iostream>
#include <unistd.h>

int main()
{
    zmq::context_t context(1);
    zmq::socket_t socket(context, zmq::socket_type::stream);

    socket.bind("tcp://*:5555");

    int num = 0;
    while (true) {
        zmq::message_t identity;
        socket.recv(&identity);

        zmq::message_t message;
        socket.recv(&message);

        std::string msg(static_cast<char*>(message.data()), message.size());
        std::cout << "Received message: " << msg << std::endl;

        socket.send(identity, zmq::send_flags::sndmore);
        std::string back_msg = "back " + std::to_string(++num);
        socket.send(zmq::message_t(back_msg));
    }

    return 0;
}
