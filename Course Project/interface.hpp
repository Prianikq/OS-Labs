#pragma once

#include <string.h>
#include <zmq.h>
#include <stdio.h>
#include <stdlib.h>
#define check_ok1(VALUE, OKVAL, MSG) if (VALUE != OKVAL) { fprintf(stderr, MSG); exit(-1); }
#define check_wrong1(VALUE, WRONGVAL, MSG) if (VALUE == WRONGVAL) { fprintf(stderr, MSG); exit(-1); }
const int WAIT_TIME = 1000000;

const int DATA_SIZE = 5;
const int start = 1;
const int info = 2;
struct Token {
	int action;
	int data[DATA_SIZE];
};

void CreateSocket(void* & context, void* & socket) {
	int res;
	context = zmq_ctx_new();
	check_wrong1(context, NULL, "Error creating context!");
	socket = zmq_socket(context, ZMQ_PAIR);
	check_wrong1(socket, NULL, "Error creating socket!");
	res = zmq_setsockopt(socket, ZMQ_RCVTIMEO, &WAIT_TIME, sizeof(int));
	check_ok1(res, 0, "Error changing options of socket!");
	res = zmq_setsockopt(socket, ZMQ_SNDTIMEO, &WAIT_TIME, sizeof(int));
	check_ok1(res, 0, "Error changing options of socket!");
}

void DeleteSocket(void* & context, void* & socket) {
    int res;
    res = zmq_close(socket);
    check_ok1(res, 0, "Error when socket closed!");
	res = zmq_ctx_term(context);
	check_ok1(res, 0, "Error when context closed!");
}

bool SendMessage(Token* token, void* socket, int type_work) { // ZMQ_DONTWAIT - dont wait, 0 - with waiting
	int res;
	zmq_msg_t message;
	res = zmq_msg_init_data(&message, token, sizeof(Token), NULL, NULL);
	check_ok1(res, 0, "Error creating message!");
	res = zmq_msg_send(&message, socket, type_work);
	if (res == -1) {
	    fprintf(stderr, "Error sending message!\n");
		zmq_msg_close(&message);
		return false;
	}
	check_ok1(res, sizeof(Token), "Error getting wrong message!");
	return true;
}

bool RecieveMessage(Token& reply_data, void* socket, int type_work) {
	int res = 0;
	zmq_msg_t reply;
	zmq_msg_init(&reply);
	check_ok1(res, 0, "Error creating message-reply!");
	res = zmq_msg_recv(&reply, socket, type_work);
	if (res == -1) {
		if (type_work == 0) {
			fprintf(stderr, "Error getting message!\n");
			res = zmq_msg_close(&reply);
			check_ok1(res, 0, "Error closing message!");
		}
		return false;
	}
	check_ok1(res, sizeof(Token), "Error getting wrong message!");
	reply_data = *(Token*)zmq_msg_data(&reply);
	res = zmq_msg_close(&reply);
	check_ok1(res, 0, "Error closing message!");
	return true;
}

bool DialogMessages(Token* send, Token& reply, void* socket) {
	if (SendMessage(send, socket, 0) && RecieveMessage(reply, socket, 0)) {
	    return true;
	}
	return false;
}
