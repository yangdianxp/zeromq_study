// 2015-05-12T11:55+08:00
//  Load-balancing broker
//  Demonstrates use of the CZMQ API

#include "czmq.h"

#include <iostream>
#include <thread>
#include <vector>

#define NBR_CLIENTS 1
#define NBR_WORKERS 1
#define WORKER_READY   "READY"      //  Signals worker is ready

//  Basic request-reply client using REQ socket
//
static void *
client_task(void *args)
{
	zsock_t *client= zsock_new (ZMQ_REQ);

#if (defined (WIN32))
	zsock_connect(client, "tcp://localhost:5672"); // frontend
#else
	zsock_connect(client, "ipc://frontend.ipc");
#endif

	//  Send request, get reply
	zstr_send(client, "HELLO");
	char *reply = zstr_recv(client);
	if (reply) {
		std::cout << "Client: " << reply << std::endl;
		free(reply);
	}

	zsock_destroy(&client);
	return NULL;
}

//  Worker using REQ socket to do load-balancing
//
static void *
worker_task(void *args)
{
	zsock_t *worker = zsock_new (ZMQ_REQ);

#if (defined (WIN32))
	zsock_connect(worker, "tcp://localhost:5673"); // backend
#else
	zsock_connect(worker, "ipc://backend.ipc");
#endif

	//  Tell broker we're ready for work
	zframe_t *frame = zframe_new(WORKER_READY, strlen(WORKER_READY));
	zframe_send(&frame, worker, 0);

	//  Process messages as they arrive
	while (1) {
		zmsg_t *msg = zmsg_recv(worker);
		if (!msg)
			break;              //  Interrupted
		zframe_print(zmsg_last(msg), "Worker: ");
		zframe_reset(zmsg_last(msg), "OK", 2);
		zmsg_send(&msg, worker);
	}
	zsock_destroy (&worker);
	return NULL;
}

//  .split main task
//  Now we come to the main task. This has the identical functionality to
//  the previous {{lbbroker}} broker example, but uses CZMQ to start child 
//  threads, to hold the list of workers, and to read and send messages:

int main(void)
{
	zsock_t *frontend = zsock_new (ZMQ_ROUTER);
	zsock_t *backend = zsock_new (ZMQ_ROUTER);

	// IPC doesn't yet work on MS Windows.
#if (defined (WIN32))
	zsock_bind(frontend, "tcp://*:5672");
	zsock_bind(backend, "tcp://*:5673");
#else
	zsock_bind(frontend, "ipc://frontend.ipc");
	zsock_bind(backend, "ipc://backend.ipc");
#endif

	std::vector<std::thread> client_thread;
	int client_nbr;
	for (client_nbr = 0; client_nbr < NBR_CLIENTS; client_nbr++)
	{
		client_thread.push_back(std::thread(client_task, nullptr));
	}
		
	std::vector<std::thread> worker_thread;
	int worker_nbr;
	for (worker_nbr = 0; worker_nbr < NBR_WORKERS; worker_nbr++)
	{
		worker_thread.push_back(std::thread(worker_task, nullptr));
	}
		

	//  Queue of available workers
	zlist_t *workers = zlist_new();

	//  .split main load-balancer loop
	//  Here is the main loop for the load balancer. It works the same way
	//  as the previous example, but is a lot shorter because CZMQ gives
	//  us an API that does more with fewer calls:
	while (1) {
		zmq_pollitem_t items[] = {
				{ backend, 0, ZMQ_POLLIN, 0 },
				{ frontend, 0, ZMQ_POLLIN, 0 }
		};
		//  Poll frontend only if we have available workers
		int rc = zmq_poll(items, zlist_size(workers) ? 2 : 1, -1);
		if (rc == -1)
			break;              //  Interrupted

		//  Handle worker activity on backend
		if (items[0].revents & ZMQ_POLLIN) {
			//  Use worker identity for load-balancing
			zmsg_t *msg = zmsg_recv(backend);
			if (!msg)
				break;          //  Interrupted

#if 0
			// zmsg_unwrap is DEPRECATED as over-engineered, poor style
			zframe_t *identity = zmsg_unwrap(msg);
#else
			zframe_t *identity = zmsg_pop(msg);
			zframe_t *delimiter = zmsg_pop(msg);
			zframe_destroy(&delimiter); 
#endif

			zlist_append(workers, identity);

			//  Forward message to client if it's not a READY
			zframe_t *frame = zmsg_first(msg);
			if (memcmp(zframe_data(frame), WORKER_READY, strlen(WORKER_READY)) == 0) {
				zmsg_destroy(&msg);
			} else {
				zmsg_send(&msg, frontend);
				if (--client_nbr == 0)
					break; // Exit after N messages
			}
		}
		if (items[1].revents & ZMQ_POLLIN) {
			//  Get client request, route to first available worker
			zmsg_t *msg = zmsg_recv(frontend);
			if (msg) {
#if 0
				// zmsg_wrap is DEPRECATED as unsafe
				zmsg_wrap(msg, (zframe_t *)zlist_pop(workers));
#else
				zmsg_pushmem(msg, NULL, 0); // delimiter
				zmsg_push(msg, (zframe_t *)zlist_pop(workers));
#endif

				zmsg_send(&msg, backend);
			}
		}
	}
	//  When we're done, clean up properly
	while (zlist_size(workers)) {
		zframe_t *frame = (zframe_t *)zlist_pop(workers);
		zframe_destroy(&frame);
	}
	zlist_destroy(&workers);
	for (auto&& th : client_thread)
	{
		th.join();
	}
	for (auto&& th : worker_thread)
	{
		th.join();
	}
	return 0;
}
