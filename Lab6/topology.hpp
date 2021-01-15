#pragma once

#include <iostream>
#include <list>

class Topology {
    public: 
        bool Insert(const int parent, const int node);
        bool Erase(const int node);
        int Find(const int node);
    private:
	    using list_type = std::list< std::list<int> >;
	    using iterator = typename std::list<int>::iterator;
	    using list_iterator = typename list_type::iterator;
	    list_type data;
};

bool Topology::Insert(const int parent, const int node) {
	if (parent == -1) {
	    std::list<int> new_list;
		new_list.push_back(node);
		data.push_back(new_list);
		return true;
	}
	for (list_iterator i = data.begin(); i != data.end(); ++i) {
		for (iterator j = i->begin(); j != i->end(); ++j) {
			if (*j == parent) {
			    ++j;
				i->insert(j, node);
				return true;
			}
		}
	}
	return false;
}
bool Topology::Erase(const int node) {
	for (list_iterator i = data.begin(); i != data.end(); ++i) {
		for (iterator j = i->begin(); j != i->end(); ++j) {
			if (*j == node) {
			    i->erase(j);
			    if (i->size() == 0) {
			        data.erase(i);
			    }
				return true;
			}
		}
	}
	return false;
}
int Topology::Find(const int node) {
	int num_list = 0;
	for (list_iterator i = data.begin(); i != data.end(); ++i) {
		for (iterator j = i->begin(); j != i->end(); ++j) {
			if (*j == node) {
				return num_list;
			}
		}
		++num_list;
	}
	return -1;
}
