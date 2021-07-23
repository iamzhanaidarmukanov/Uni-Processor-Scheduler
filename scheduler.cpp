#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
using namespace std;

#define MAX_LOOP 1000
#define K 5


// Service structure
struct service_t {
	string type;	// C(CPU), K(keyboard input), D(diskI/O), L(mutex lock) and U(mutexunlock)
	int time_cost;
	service_t() : type( "" ), time_cost( -1 ) {}
	service_t(string type, string desc) : type(type) {
		if (type == "L") {
			this->time_cost = 0;
		}
		else if (type == "U") {
			this->time_cost = 0;
		}
		else {
			this->time_cost = stoi(desc);
		}
	}
};


// Process structure
struct process_t {
	int process_id;
	int arrival_time;
	vector<service_t> service_seq;
	service_t cur_service;
	int cur_service_idx;
	int cur_service_tick;	// num of ticks that has been spent on current service
	vector<int> working;	// working sequence on CPU, for loging output
	int priority=0;        // which priority queue located in ONLY FOR FEEDBACK 

	// Call when current service completed
	// if there are no service left, return true. Otherwise, return false
	bool proceed_to_next_service() {
		this->cur_service_idx++;
		this->cur_service_tick = 0;
		if (this->cur_service_idx >= this->service_seq.size()) {	// all services are done, process should end
			return true;
		}
		else {		// still requests services
			this->cur_service = this->service_seq[this->cur_service_idx];
			return false;
		}
	};

    // Log the working ticks on CPU (from `start_tick` to `end_tick`)
	void log_working(int start_tick, int end_tick)
	{
		this->working.push_back(start_tick);
		this->working.push_back(end_tick);
	};
};


// write output log
int write_file(vector<process_t> processes, const char* file_path)
{
	ofstream outputfile;
	outputfile.open(file_path);
	for (vector<process_t>::iterator p_iter = processes.begin(); p_iter != processes.end(); p_iter++) {
		outputfile << "process " << p_iter->process_id << endl;
		for (vector<int>::iterator w_iter = p_iter->working.begin(); w_iter != p_iter->working.end(); w_iter++) {
			outputfile << *w_iter << " ";
		}
		outputfile << endl;
	}
	outputfile.close();
	return 0;
}


// Split a string according to a delimiter
void split(const string& s, vector<string>& tokens, const string& delim = " ")
{
	string::size_type last_pos = s.find_first_not_of(delim, 0);
	string::size_type pos = s.find_first_of(delim, last_pos);
	while (string::npos != pos || string::npos != last_pos) {
		tokens.push_back(s.substr(last_pos, pos - last_pos));
		last_pos = s.find_first_not_of(delim, pos);
		pos = s.find_first_of(delim, last_pos);
	}
}


vector<process_t> read_processes(const char* file_path)
{
	vector<process_t> process_queue;
	ifstream file(file_path);
	string str;
	while (getline(file, str)) {
		process_t new_process;
		stringstream ss(str);
		int service_num;
		char syntax;
		ss >> syntax >> new_process.process_id >> new_process.arrival_time >> service_num;
		for (int i = 0; i < service_num; i++) {	// read services sequence
			getline(file, str);
			str = str.erase(str.find_last_not_of(" \n\r\t") + 1);
			vector<string> tokens;
			split(str, tokens, " ");
			service_t ser(tokens[0], tokens[1]);
			new_process.service_seq.push_back(ser);
		}
		new_process.cur_service_idx = 0;
		new_process.cur_service_tick = 0;
		new_process.cur_service = new_process.service_seq[new_process.cur_service_idx];
		process_queue.push_back(new_process);
	}
	return process_queue;
}


// move the process at the front of q1 to the back of q2 (q1 head -> q2 tail)
int move_process_from(vector<process_t>& q1, vector<process_t>& q2)
{
	if (!q1.empty()) {
		process_t& tmp = q1.front();
		q2.push_back(tmp);
		q1.erase(q1.begin());
		return 1;
	}
	return 0;
}



// main Function Start

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cout << "Incorrect inputs: has to be 4 arguments" << endl;
        return 0;
    }

    const char* scheduling_algorithm = argv[1];
    const char* process_path = argv[2];
    const char* output_path = argv[3];

    vector<process_t> process_queue = read_processes(process_path);

    if (strcmp(scheduling_algorithm, "FCFS") == 0) {
        fcfs(process_queue, output_path);
    } 
    // else if (strcmp(scheduling_algorithm, "RR") == 0) {
    //     rr(process_queue, output_path);
    // } 
    // else if (strcmp(scheduling_algorithm, "FB") == 0) {
    //     fb(process_queue, output_path);
    // } 
    else {
        cout << "Wrong scheduling algorithm format, has to be FCFS, RR or FB" << endl;
    }

    return 0;
}