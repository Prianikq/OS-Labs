#pragma once

#include <string.h>
#include <zmq.h>
#include <string>
#include <cstdlib>
#include <iostream>
#define check_ok(VALUE, OKVAL, MSG) if (VALUE != OKVAL) { std::cout << MSG << std::endl; exit(-1); }
#define check_wrong(VALUE, WRONGVAL, MSG) if (VALUE == WRONGVAL) { std::cout << MSG << std::endl; exit(-1); }

const int VALUE_PORT = 8000;
const int WAIT_TIME = 1000;

enum Action {
    create,
    destroy,
    fail,
    success,
    exec_start,
    exec_stop,
    exec_time,
    bind,
    ping,
    info,
};

struct Token {
    Action action;
	int parent_id;
	int id;
};

void CreateSocket(void* & context, void* & socket) {
	int res;
	context = zmq_ctx_new();
	check_wrong(context, NULL, "Error creating context");
	socket = zmq_socket(context, ZMQ_PAIR);
	check_wrong(socket, NULL, "Error creating socket");
	res = zmq_setsockopt(socket, ZMQ_RCVTIMEO, &WAIT_TIME, sizeof(int));
	check_ok(res, 0, "Error changing options of socket");
	res = zmq_setsockopt(socket, ZMQ_SNDTIMEO, &WAIT_TIME, sizeof(int));
	check_ok(res, 0, "Error changing options of socket");
}

void DeleteSocket(void* & context, void* & socket) {
    int res;
    res = zmq_close(socket);
    check_ok(res, 0, "Error when socket closed");
	res = zmq_ctx_term(context);
	check_ok(res, 0, "Error when context closed");
}

bool SendMessage(Token* token, void* socket, int type_work) { // ZMQ_DONTWAIT - dont wait, 0 - with waiting
	int res;
	zmq_msg_t message;
	res = zmq_msg_init_data(&message, token, sizeof(Token), NULL, NULL);
	check_ok(res, 0, "Error creating message");
	res = zmq_msg_send(&message, socket, type_work);
	if (res == -1) {
	    std::cout << "Error sending message" << std::endl;
		zmq_msg_close(&message);
		return false;
	}
	check_ok(res, sizeof(Token), "Error getting wrong message");
	return true;
}

bool RecieveMessage(Token& reply_data, void* socket) {
	int res = 0;
	zmq_msg_t reply;
	zmq_msg_init(&reply);
	check_ok(res, 0, "Error creating message-reply");
	res = zmq_msg_recv(&reply, socket, 0);
	if (res == -1) {
	    std::cout << "Error getting message" << std::endl;
		res = zmq_msg_close(&reply);
		check_ok(res, 0, "Error closing message");
		return false;
	}
	check_ok(res, sizeof(Token), "Error getting wrong message");
	reply_data = *(Token*)zmq_msg_data(&reply);
	res = zmq_msg_close(&reply);
	check_ok(res, 0, "Error closing message");
	return true;
}

bool DialogMessages(Token* send, Token& reply, void* socket) {
	if (SendMessage(send, socket, 0) && RecieveMessage(reply, socket)) {
	    return true;
	}
	return false;
}
