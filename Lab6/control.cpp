#include "topology.hpp"
#include "interface.hpp"

#include <unistd.h>
#include <vector>
#include <string>
#include <cstdlib>
#include <iostream>
#define check_ok(VALUE, OKVAL, MSG) if (VALUE != OKVAL) { std::cout << MSG << std::endl; exit(-1); }
#define check_wrong(VALUE, WRONGVAL, MSG) if (VALUE == WRONGVAL) { std::cout << MSG << std::endl; exit(-1); }

char* const CALCULATE_NAME = "calculate";

auto main() -> int {
    int res;
	Topology nodes;
	std::vector< std::pair<void*, void*> > nodes_info;
	std::string oper;
	int id;
	while (std::cin >> oper >> id) {
		if (oper == "create") {
			int par_id;
			std::cin >> par_id;
			if (nodes.Find(id) != -1) {
			    std::cout << "Error: Already exist" << std::endl;
			    continue;
			}
			if (par_id == -1) {
				void* new_context = NULL;
				void* new_socket = NULL;
				CreateSocket(new_context, new_socket);
				res = zmq_bind(new_socket, ("tcp://*:" + std::to_string(VALUE_PORT + id)).c_str());
				check_ok(res, 0, "Error when bind socket with ....");

				int fork_id = fork();
				check_wrong(fork_id, -1, "Error when creating new process with fork()");
				if (fork_id == 0) {
					res = execl(CALCULATE_NAME, CALCULATE_NAME, std::to_string(id).c_str(), NULL);
					check_wrong(res, -1, "Error when changing execution child process");
					return 0;
				} 
				else {
					bool success = true;
					Token reply_info({Action::fail, id, id});
					success = RecieveMessage(reply_info, new_socket);
					if (success && reply_info.action == Action::info) {
						nodes_info.push_back(std::make_pair(new_context, new_socket));
						nodes.Insert(par_id, id);
						std::cout << "OK: " << reply_info.id << std::endl;
					}
					else {
						DeleteSocket(new_context, new_socket);
					}
				}
			} 
			else {
			    int ind = nodes.Find(par_id);
			    if (ind == -1) {
			        std::cout << "Error: Parent not found" << std::endl;
			        continue;
			    }
				Token* request_create = new Token({Action::create, par_id, id});
				Token reply_create({Action::fail, id, id});
				if (DialogMessages(request_create, reply_create, nodes_info[ind].second) && reply_create.action == Action::success) {
					std::cout << "OK: " << reply_create.id << std::endl;
					nodes.Insert(par_id, id);
				} 
				else {
					std::cout << "Error: Parent is unavailable" << std::endl;
				}
			}
		} 
		else if (oper == "remove") {
			int ind = nodes.Find(id);
			if (ind == -1) {
			    std::cout << "Error: Not found" << std::endl;
			    continue;
			}
			Token* request_destroy = new Token({Action::destroy, id, id});
			Token reply_destroy({Action::fail, id, id});
			bool result = DialogMessages(request_destroy, reply_destroy, nodes_info[ind].second);
			if (!result) {
				std::cout << "Error: Node is unavailable" << std::endl;
			}
			else if (reply_destroy.action == Action::fail) {
				std::cout << "Error: Erase was failed" << std::endl;
			}
			else if (reply_destroy.action == Action::success) {
				if (reply_destroy.parent_id == id) {
					DeleteSocket(nodes_info[ind].first, nodes_info[ind].second);
					nodes_info.erase(nodes_info.begin() + ind);
				}
				nodes.Erase(id);
				std::cout << "OK" << std::endl;
			} 
			else if (reply_destroy.action == Action::bind && reply_destroy.parent_id == id) { // ???
				DeleteSocket(nodes_info[ind].first, nodes_info[ind].second);
				CreateSocket(nodes_info[ind].first, nodes_info[ind].second);
				res = zmq_bind(nodes_info[ind].second, ("tcp://*:" + std::to_string(VALUE_PORT + reply_destroy.id)).c_str());
				check_ok(res, 0, "Error when bind socket with ....");
				nodes.Erase(id);
				std::cout << "OK" << std::endl;
			}
		} 
		else if (oper == "ping") {
			int ind = nodes.Find(id);
			if (ind == -1) {
			    std::cout << "Error: Not found" << std::endl;
			    continue;
			}
			Token* request_ping = new Token({Action::ping, id, id});
			Token reply_ping({Action::fail, id, id});
			if (DialogMessages(request_ping, reply_ping, nodes_info[ind].second) && reply_ping.action == Action::success) {
				std::cout << "OK: 1" << std::endl;
			} 
			else {
				std::cout << "OK: 0" << std::endl;
			}
		}
		else if (oper == "exec") {
			std::string subcom;
			std::cin >> subcom;
			int ind = nodes.Find(id);
			if (ind == -1) {
			    std::cout << "Error: Not found" << std::endl;
			    continue;
			}
			if (subcom == "start") {
				Token* request_start = new Token({Action::exec_start, id, id});
				Token reply_start({Action::fail, id, id});
				if (DialogMessages(request_start, reply_start, nodes_info[ind].second) && reply_start.action == Action::success) {
					std::cout << "OK: " << reply_start.id << std::endl;
				}
				else {
					std::cout << "Error starting timer in " << id << std::endl;
				} 
			}
			else if (subcom == "stop") {
				Token* request_stop = new Token({Action::exec_stop, id, id});
				Token reply_stop({Action::fail, id, id});
				if (DialogMessages(request_stop, reply_stop, nodes_info[ind].second) && reply_stop.action == Action::success) {
					std::cout << "OK: " << reply_stop.id << std::endl;
				}
				else {
					std::cout << "Error stoping timer in " << id << std::endl;
				} 
			}
			else if (subcom == "time") {
				Token* request_time = new Token({Action::exec_time, id, id});
				Token reply_time({Action::fail, id, id});
				if (DialogMessages(request_time, reply_time, nodes_info[ind].second) && reply_time.action == Action::success) {
					std::cout << "OK: " << reply_time.parent_id << ": " << reply_time.id << std::endl;
				}
				else {
					std::cout << "Error getting time in " << id << std::endl;
				} 
			}
			else {
			    std::cout << "Error: Wrong command of timer" << std::endl;
			}
		}
		else {
		    std::cout << "Error: Wrong command" << std::endl;
		}
	}
	for (size_t i = 0; i < nodes_info.size(); ++i) {
		DeleteSocket(nodes_info[i].first, nodes_info[i].second);
	}
}
