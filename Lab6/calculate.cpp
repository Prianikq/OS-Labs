#include "interface.hpp"

#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <ctime>
#include <chrono>
#define check_ok(VALUE, OKVAL, MSG) if (VALUE != OKVAL) { std::cout << MSG << std::endl; exit(-1); }
#define check_wrong(VALUE, WRONGVAL, MSG) if (VALUE == WRONGVAL) { std::cout << MSG << std::endl; exit(-1); }

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;
char* const CALCULATE_NAME = "calculate";
long long GetTime() {
	long long millisec = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	return millisec;
}

int main(int argc, char* argv[]) {
	int res;
	check_ok(argc, 2, "Error: Wrong count arguments in calculate process");
	int node_id = std::stoll(std::string(argv[1]));
	int child_id = -1;

	void* my_context = zmq_ctx_new();
	void* my_socket = zmq_socket(my_context, ZMQ_PAIR);
	void* child_context = NULL;
	void* child_socket = NULL;
	
	res = zmq_connect(my_socket, ("tcp://localhost:" + std::to_string(VALUE_PORT + node_id)).c_str());
	
	check_ok(res, 0, "Error when connecting to socket in calculate process");

	long long start = -1, finish = -1, time_ans = 0;
	
	Token* info_token = new Token({Action::info, getpid(), getpid()});
	SendMessage(info_token, my_socket, ZMQ_DONTWAIT);
	bool is_parent = false;
	bool work = true;
	while (work) {
		Token token;
		RecieveMessage(token, my_socket);
		Token* reply = new Token({Action::fail, node_id, node_id});
        if (token.action == Action::bind && token.parent_id == node_id) {
			CreateSocket(child_context, child_socket);
			res = zmq_bind(child_socket, ("tcp://*:" + std::to_string(VALUE_PORT + token.id)).c_str());
			check_ok(res, 0, "Error bind to socket in calculate process");
			is_parent = true;
			child_id = token.id;
			reply->action = Action::success;
		} 
		else if (token.action == Action::create) {
			if (token.parent_id == node_id) {
				if (is_parent) {
				    DeleteSocket(child_context, child_socket);
				}
				CreateSocket(child_context, child_socket);
				res = zmq_bind(child_socket, ("tcp://*:" + std::to_string(VALUE_PORT + token.id)).c_str());
				check_ok(res, 0, "Error when bind with child socket");
				int fork_id = fork();
				check_wrong(fork_id, -1, "Error creating calculating process using fork");
				if (fork_id == 0) {
					res = execl(CALCULATE_NAME, CALCULATE_NAME, std::to_string(token.id).c_str(), NULL);
					check_wrong(res, -1, "Error when changing execution in calculate process");
					return 0;
				} 
				else {
					bool result = true;
					Token reply_info({Action::fail, token.id, token.id});
					result = RecieveMessage(reply_info, child_socket);
					if (!result) {
						DeleteSocket(child_context, child_socket);
					}
					else {
						if (reply_info.action == Action::info) {
							reply->id = reply_info.id;
							reply->parent_id = reply_info.parent_id;
						}
						if (is_parent) {
							Token* request_bind = new Token({Action::bind, token.id, child_id});
							Token reply_bind({Action::fail, token.id, token.id});
							result = DialogMessages(request_bind, reply_bind, child_socket);
							if (result && reply_bind.action == Action::success) {
								child_id = token.id;
								reply->action = Action::success;
							}
							else {
								DeleteSocket(child_context, child_socket);
							}
						}
						else {
							reply->action = Action::success;
							child_id = token.id;
							is_parent = true;
						}
					}
				}	
			} 
			else if (is_parent) {
				Token* request_create = new Token(token);
			    Token reply_create(token);
				reply_create.action = Action::fail;
				if (DialogMessages(request_create, reply_create, child_socket) && reply_create.action == Action::success) {
					*reply = reply_create;
				}
			}
		} 
		else if (token.action == Action::ping) {
			if (token.id == node_id) {
				reply->action = Action::success;
			} 
			else if (is_parent) {
				Token* request_ping = new Token(token);
				Token reply_ping(token);
				reply_ping.action = Action::fail;
				if (DialogMessages(request_ping, reply_ping, child_socket) && reply_ping.action == Action::success) {
					*reply = reply_ping;
				}
			}
		} 
		else if (token.action == Action::destroy) {
			if (is_parent) {
				if (token.id == child_id) {
					Token* request_destroy = new Token({Action::destroy, node_id, child_id});
					Token reply_destroy({Action::fail, child_id, child_id});
					bool result = DialogMessages(request_destroy, reply_destroy, child_socket);
					if (reply_destroy.action == Action::success && reply_destroy.parent_id == child_id) {
					    DeleteSocket(child_context, child_socket);
						reply->action = Action::success;
						reply->id = child_id;
						reply->parent_id = node_id;
						child_id = -1;
						is_parent = false;
					} 
					else if (reply_destroy.action == Action::bind && reply_destroy.parent_id == node_id) {
						DeleteSocket(child_context, child_socket);
						CreateSocket(child_context, child_socket);
						res = zmq_bind(child_socket, ("tcp://*:" + std::to_string(VALUE_PORT + reply_destroy.id)).c_str());
						check_ok(res, 0, "Error binding with calculate process");
						reply->action = Action::success;
						reply->id = child_id;
						reply->parent_id = node_id;
						child_id = reply_destroy.id;
					} 
				}
				else if (token.id == node_id) {
				    DeleteSocket(child_context, child_socket);
					is_parent = false;
					reply->action = Action::bind;
					reply->parent_id = token.parent_id;
					reply->id = child_id;
					work = false;
				} 
				else {
					Token* request_destroy  = new Token(token);
					Token reply_destroy(token);
					reply_destroy.action = fail;
					if (DialogMessages(request_destroy, reply_destroy, child_socket) && reply_destroy.action == Action::success) {
						*reply = reply_destroy;
					}
				}
			} 
			else if (token.id == node_id) {
				reply->action = Action::success;
				reply->parent_id = node_id;
				reply->id = node_id;
				work = false;
			}
		} 
		else if (token.action == Action::exec_start) {
			if (token.id == node_id) {
				time_ans = 0;
				start = GetTime();
				reply->action = Action::success;
			} 
			else if (is_parent) {
				Token* request_start = new Token(token);
				Token reply_start(token);
				reply_start.action = Action::fail;
				if (DialogMessages(request_start, reply_start, child_socket) && reply_start.action == Action::success) {
					*reply = reply_start;
				}
			}
		}
		else if (token.action == Action::exec_stop) {
			if (token.id == node_id) {
				if (start != -1) {
					finish = GetTime();
					time_ans += finish - start;
					finish = -1;
					start = -1;
					reply->action = Action::success;
				}
			} 
			else if (is_parent) {
				Token* request_stop = new Token(token);
				Token reply_stop(token);
				reply_stop.action = Action::fail;
				if (DialogMessages(request_stop, reply_stop, child_socket) && reply_stop.action == Action::success) {
					*reply = reply_stop;
				}
			}
		}
		else if (token.action == Action::exec_time) {
			if (token.id == node_id) {
				if (start != -1) {
					finish = GetTime();
					time_ans += finish - start;
					start = finish;
				}
				reply->action = Action::success;
				reply->id = (int) time_ans;
			} 
			else if (is_parent) {
				Token* request_time = new Token(token);
				Token reply_time(token);
				reply_time.action = Action::fail;
				if (DialogMessages(request_time, reply_time, child_socket) && reply_time.action == Action::success) {
					*reply = reply_time;
				}
			}
		}
		SendMessage(reply, my_socket, ZMQ_DONTWAIT);
	}
	if (is_parent) {
	    DeleteSocket(child_context, child_socket);
	}
	DeleteSocket(my_context, my_socket);
}
