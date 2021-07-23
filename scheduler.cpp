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

void manage_next_service_fcfs(process_t& cur_process, int& complete_num, int& dispatched_tick,
	int& cur_tick, vector<process_t>& ready_queue, vector<process_t>& processes_done,
	vector<process_t>& block_queue_D, vector<process_t>& block_queue_K, vector<process_t>& block_queue_mtx) {
	bool process_completed = cur_process.proceed_to_next_service();
	if (process_completed) {		// the whole process is completed
		complete_num++;
		cur_process.log_working(dispatched_tick, cur_tick + 1);
		move_process_from(ready_queue, processes_done);		// remove current process from ready queue
	}
	else if (cur_process.cur_service.type == "D") {		// next service is disk I/O, block current process
		cur_process.log_working(dispatched_tick, cur_tick + 1);
		move_process_from(ready_queue, block_queue_D);
	}
	else if (cur_process.cur_service.type == "K") {		// next service is keyboard input, block current process
		cur_process.log_working(dispatched_tick, cur_tick + 1);
		move_process_from(ready_queue, block_queue_K);
	}
	else if (cur_process.cur_service.type == "L") {     // next service is mutex lock
		if (mutex.status_lock == false) {
			mutex.mutex_lock();
			manage_next_service_fcfs(cur_process, complete_num, dispatched_tick, cur_tick, ready_queue,
				processes_done, block_queue_D, block_queue_K, block_queue_mtx); // look for next service
		}
		else {
			cur_process.log_working(dispatched_tick, cur_tick + 1);
			move_process_from(ready_queue, block_queue_mtx);    // block current process
		}
	}
	else if (cur_process.cur_service.type == "U") {     // next service is mutex unlock
		if (mutex.status_lock == true) {
			mutex.mutex_unlock();
			manage_next_service_fcfs(cur_process, complete_num, dispatched_tick, cur_tick, ready_queue,
				processes_done, block_queue_D, block_queue_K, block_queue_mtx); // look for next service
		}
	}
}


int fcfs(vector<process_t> processes, const char* output_path)
{
	vector<process_t> ready_queue;
	vector<process_t> block_queue_D;
	vector<process_t> block_queue_K;
	vector<process_t> processes_done;
	vector<process_t> block_queue_mtx;

	int complete_num = 0;
	int dispatched_tick = 0;
	int cur_process_id = -1, prev_process_id = -1;

	// main loop
	for (int cur_tick = 0; cur_tick < MAX_LOOP; cur_tick++) {

		// long term scheduler
		for (int i = 0; i < processes.size(); i++) {
			if (processes[i].arrival_time == cur_tick) {		// process arrives at current tick
				ready_queue.push_back(processes[i]);
			}
		}

		// disk I/O device scheduling
		if (!block_queue_D.empty()) {
			process_t& cur_io_process = block_queue_D.front();	// always provide service to the first process in block queue
			if (cur_io_process.cur_service_tick >= cur_io_process.cur_service.time_cost) {	// I/O service is completed
				cur_io_process.proceed_to_next_service();
				move_process_from(block_queue_D, ready_queue);
			}
			cur_io_process.cur_service_tick++;		// increment the num of ticks that have been spent on current service
		}
		// keyboard I/O device scheduling
		if (!block_queue_K.empty()) {
			process_t& cur_io_process = block_queue_K.front();	// always provide service to the first process in block queue
			if (cur_io_process.cur_service_tick >= cur_io_process.cur_service.time_cost) {	// I/O service is completed
				cur_io_process.proceed_to_next_service();
				move_process_from(block_queue_K, ready_queue);
			}
			cur_io_process.cur_service_tick++;		// increment the num of ticks that have been spent on current service
		}
		// mutex scheduling
		if (!block_queue_mtx.empty()) {
			process_t& cur_mtx_process = block_queue_mtx.front();	// always provide service to the first process in block queue
			if (cur_mtx_process.cur_service.type == "L") {
				if (mutex.status_lock == false) {  //lock mutex if unlocked
					mutex.mutex_lock();
					cur_mtx_process.proceed_to_next_service();
					move_process_from(block_queue_mtx, ready_queue);
				}
			}
		}
		// CPU scheduling
		if (ready_queue.empty()) {	// no process for scheduling
			prev_process_id = -1;	// reset the previous dispatched process ID to empty
		}
		else {
			process_t& cur_process = ready_queue.front();	// always dispatch the first process in ready queue
			cur_process_id = cur_process.process_id;
			if (cur_process_id != prev_process_id) {		// store the tick when current process is dispatched
				dispatched_tick = cur_tick;
			}
			cur_process.cur_service_tick++;		// increment the num of ticks that have been spent on current service
			if (cur_process.cur_service_tick >= cur_process.cur_service.time_cost) {	// current service is completed
				manage_next_service_fcfs(cur_process, complete_num, dispatched_tick, cur_tick, ready_queue,
					processes_done, block_queue_D, block_queue_K, block_queue_mtx);  // look for next service
			}
			prev_process_id = cur_process_id;	// log the previous dispatched process ID
		}
		if (complete_num == processes.size()) {	// all process completed
			break;
		}
	}
	write_file(processes_done, output_path);	// write output
	return 1;
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