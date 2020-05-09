#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>

using namespace std;

////////////////////////////////////////////////////////////////
#define ARRIVAL 1
#define DEPARTURE 2
#define TIMESLICE 3

////////////////////////////////////////////////////////////////     //event structure
struct event {
	float a_time;
	float d_time;
	float s_time;
	float r_time;
	float w_time;
	int   type;
	// add more fields
	struct event* next;
};

////////////////////////////////////////////////////////////////
// function definition
void init(int argc, char* argv[]);
int run_sim();
void generate_report();
void process_ARRIVAL(struct event* eve);
void process_DEPARTURE(struct event* eve);
void process_TIMESLICE();
void add_arr_node(float lamda, float mu);
void add_dep_node(struct event* eve);
void add_timeslice();
void add_to_ready(struct event* eve);
float genexp(float lambda);
float urand();
void merge_sort(struct event** eve);
void merge_sortL(struct event** eve);
void list_split(struct event* eve, struct event** a, struct event** b);
struct event* merge_sorted(struct event* a, struct event* b);
struct event* merge_sortedL(struct event* a, struct event* b);

////////////////////////////////////////////////////////////////
//Global variables
struct event* head = (struct event*) malloc(sizeof(struct event));// head of event queue
struct event* current = (struct event*) malloc(sizeof(struct event));// head of event queue
float clock; // simulation clock
float SRTF_service_start;
int CPU_idle; //CPU idle flag
int sched_choice;//stores scheduler choice
struct event* r_head = (struct event*) malloc(sizeof(struct event));//head of ready queue
int counter;//counts the number of completed processes
float quantum;
float avg_turnaround;
float turnaround_count;
float total_throughput;
float CPU_util;
float CPU_start;
float CPU_end;
float end_time;
float avg_waiting;
float R_util;
float R_start;
float R_end;
float wait_count;
float lamda;
int goal;
float avg_s_time;


////////////////////////////////////////////////////////////////
// initialize all varilables, states, and end conditions
// schedule first events
void init(int argc, char* argv[])
{

	goal = 10000;
	sched_choice = atof(argv[1]);
	lamda = atof(argv[2]);
	avg_s_time = atof(argv[3]);
	quantum = atof(argv[4]);
	CPU_idle = 1;
	turnaround_count = 0;
	avg_turnaround = 0;
	avg_waiting = 0;
	end_time = 0;
	CPU_start = 0;
	CPU_end = 0;
	CPU_util = 0;
	R_start = 0;
	R_end = 0;
	R_util = 0;
	head->type = 1;
	head->a_time = 0;
	head->d_time = 0;
	head->s_time = genexp(1 / avg_s_time);
	head->r_time = 0;
	head->w_time = 0;
	head->next = NULL;
	r_head->type = 1;
	r_head->a_time = 0;
	r_head->d_time = 0;
	r_head->s_time = 0;
	r_head->r_time = 0;
	r_head->w_time = 0;
	r_head->next = NULL;
	clock = 0;
	//event queue creation
	for (int i = 0; i < 15001; i++) {
		add_arr_node(lamda, avg_s_time);
	}

}
////////////////////////////////////////////////////////////////  
void add_arr_node(float lamda, float ts) {
	struct event* new_node = (struct event*) malloc(sizeof(struct event));
	struct event* last = head;
	while (last->next != NULL)
		last = last->next;
    new_node->a_time  = last->a_time + genexp(lamda);//arrival rate + previous arrival
	new_node->d_time = 0;
	new_node->s_time = genexp(1 / ts);//service rate
	new_node->type = 1;
    new_node->next = NULL;
    last->next = new_node;
}
/////////////////////////////////////////////////////////////////////////////
void merge_sort(struct event** eve) {
	struct event* head = *eve;
	struct event* a;
	struct event* b;
	if ((head == NULL) || (head->next == NULL)) {
		return;
	}
	list_split(head, &a, &b);
	merge_sort(&a);
	merge_sort(&b);
	*eve = merge_sorted(a, b);

}
//////////////////////////////////////////////////////////////////////////
void merge_sortL(struct event** eve) {
	struct event* head = *eve;
	struct event* a;
	struct event* b;
	if ((head == NULL) || (head->next == NULL)) {
		return;
	}
	list_split(head, &a, &b);
	merge_sortL(&a);
	merge_sortL(&b);
	*eve = merge_sortedL(a, b);

}

struct event* merge_sortedL(struct event* a, struct event* b) {
	struct event* result = NULL;
	if (a == NULL)
		return (b);
	else if (b == NULL)
		return (a);
	if (a->s_time < b->s_time) {
		result = a;
		result->next = merge_sortedL(a->next, b);
	}
	else {
		result = b;
		result->next = merge_sortedL(a, b->next);
	}
	return (result);
}
//////////////////////////////////////////////////////////////////////////
void list_split(struct event* eve, struct event** a, struct event** b) {
	
	struct event* fast;
	struct event* slow;
	slow = eve;
	fast = eve->next;
	while (fast != NULL) {
		fast = fast->next;
		if (fast != NULL) {
			slow = slow->next;
			fast = fast->next;
		}
	}
	*a = eve;
	*b = slow->next;
	slow->next = NULL;
}
//////////////////////////////////////////////////////////////////////////
struct event* merge_sorted(struct event* a, struct event* b) {
	struct event* result = NULL;
	if (a == NULL)
		return (b);
	else if (b == NULL)
		return (a);
	if (a->r_time > b->r_time) {
		result = a;
		result->next = merge_sorted(a->next, b);
	}
	else {
		result = b;
		result->next = merge_sorted(a, b->next);
	}
	return (result);
}
//////////////////////////////////////////////////////////////// NEED TO DEALLOCATE AND FREE
void add_dep_node(struct event* eve) {
	struct event* new_node = (struct event*) malloc(sizeof(struct event));
	struct event* previous = (struct event*) malloc(sizeof(struct event));
	float dep_time = clock + eve->s_time;
	new_node->d_time = dep_time;
	new_node->a_time = eve->a_time;
	new_node->s_time = eve->s_time;
	new_node->type = 2;
	struct event* spot = head;
	previous = head;
	if (spot->next != NULL && (spot->d_time <= dep_time && spot->a_time <= dep_time)) {
		if (sched_choice == 2 && spot->next->next == NULL) {
			spot = spot->next;
			previous->next = new_node;
			new_node->next = spot;
			return;
		}
		previous = spot;
		spot = spot->next;
	}
	while (spot->next != NULL && (spot->d_time < dep_time && spot->a_time < dep_time))
	{
		previous = spot;
		spot = spot->next;
	}
	if (spot->next == NULL)
	{
		spot->next = new_node;
	}
	else{
		new_node->next = previous->next;
		previous->next = new_node;
	}
}
//////////////////////////////////////////////////////////////// 
void add_to_ready(struct event* eve) {
	struct event* new_node = (struct event*) malloc(sizeof(struct event));
	struct event* previous = (struct event*) malloc(sizeof(struct event));
	struct event* last = r_head;
	struct event* b_spot = (struct event*) malloc(sizeof(struct event));
	struct event* sort = (struct event*) malloc(sizeof(struct event));
	struct event* temp = r_head;
	bool flag = true;
	struct event* update_r_time = (struct event*) malloc(sizeof(struct event));
	new_node->a_time = eve->a_time;
	new_node->d_time = eve->d_time;
	new_node->s_time = eve->s_time;
	new_node->r_time = 0;
	new_node->type = eve->type;
	new_node->next = NULL;
	switch (sched_choice)
	{
	case 1:
		while (last->next != NULL)
			last = last->next;
		if (last->s_time == 0)
			r_head = new_node;
		else
			last->next = new_node;
		break;
	case 2:
		while (last->next != NULL)
			last = last->next;
		if (last->s_time == 0)
			r_head = new_node;
		else
			last->next = new_node;
		merge_sortL(&r_head);
		break;
	case 3:
		update_r_time = r_head;
		new_node->r_time = ((clock - SRTF_service_start) + new_node->s_time) / new_node->s_time;
		new_node->w_time = SRTF_service_start;
		if (update_r_time->s_time != 0) {
			update_r_time->r_time = ((clock - update_r_time->w_time) + update_r_time->s_time) / update_r_time->s_time;
		}
		while (update_r_time->next != NULL) {
			update_r_time = update_r_time->next;
			update_r_time->r_time = ((clock - update_r_time->w_time) + update_r_time->s_time) / update_r_time->s_time;
		}

		while (last->next != NULL)
			last = last->next;
		if (last->s_time == 0)
			r_head = new_node;
		else
			last->next = new_node;

		merge_sort(&r_head);
		break;
	case 4:
		while (last->next != NULL)
			last = last->next;
		if (last->s_time == 0)
			r_head = new_node;
		else
			last->next = new_node;
		break;
	}
	
}
////////////////////////////////////////////////////////////////
void add_timeslice() {
	struct event* new_node = (struct event*) malloc(sizeof(struct event));
	struct event* previous = (struct event*) malloc(sizeof(struct event));
	struct event* spot = head;
	float dep_time = clock + quantum;
	new_node->d_time = dep_time;
	new_node->a_time = 0;
	new_node->s_time = 0;
	new_node->type = 3;

	while (spot->next != NULL && (spot->d_time < dep_time && spot->a_time < dep_time))
	{
		previous = spot;
		spot = spot->next;
	}
	if (spot->next == NULL)
		spot->next = new_node;
	else {
		new_node->next = previous->next;
		previous->next = new_node;
	}
}
////////////////////////////////////////////////////////////////
//outputs average turnaround time, total throughput, CPU utilization, average # of processes in ready queue
void generate_report()
{
	switch (sched_choice) {
	case 1:
		cout << "First Come First Serve chosen, lamda: "<< lamda << ", average service time:" << avg_s_time <<", Quantum: "<< quantum << endl;
		break;
	case 2:
		cout << "Shortest Time Remaining First chosen, lamda: " << lamda << ", average service time:" << avg_s_time << ", Quantum: " << quantum << endl;
		break;
	case 3:
		cout << "Highest Response Ratio Next chosen, lamda: " << lamda << ", average service time:" << avg_s_time << ", Quantum: " << quantum << endl;
		break;
	case 4:
		cout << "Round Robin chosen, lamda: " << lamda << ", average service time:" << avg_s_time << ", Quantum: " << quantum << endl;
		break;
	}
	avg_turnaround = turnaround_count / counter;
	cout << "Average Turnaround Time: " << avg_turnaround << endl;
	total_throughput = counter / clock;
	cout << "Total Throughput: " << total_throughput << endl;
	CPU_util = 1 - ((clock -CPU_util) / clock);
	cout << "CPU utilization: " << CPU_util << endl;
	avg_waiting = (avg_waiting/clock) * lamda;
	cout << "Average # of processes in ready queue: " << avg_waiting << endl;

	// output statistics
}
////////////////////////////////////////////////////////////////
// returns a random number between 0 and 1
float urand()
{
	return((float)rand() / RAND_MAX);
}
/////////////////////////////////////////////////////////////
// returns a random number that follows an exp distribution
float genexp(float lambda)
{
	float u, x;
	x = 0;
	while (x == 0)
	{
		u = urand();
		x = (-1 / lambda) * log(u);
	}
	return(x);
}
////////////////////////////////////////////////////////////
//processes the arrival based on the scheduler
void process_ARRIVAL(struct event* eve) 
{
	switch (sched_choice)
	{
	case 1:
		//FCFS arrival sort
		if (CPU_idle)
		{
			add_dep_node(eve);
			CPU_idle = 0;
			CPU_start = clock;
		}
		else
		{
			add_to_ready(eve);
			R_start = clock;
		}
		break;
	case 2:
		//SRTF arrival sort
		current->s_time = current->s_time - (clock - SRTF_service_start);
		SRTF_service_start = clock;
		if (CPU_idle) 
		{
			current->a_time = eve->a_time;
			current->d_time = eve->d_time;
			current->s_time = eve->s_time;
			current->type = eve->type;
			current->next = NULL;
			CPU_idle = 0;
			CPU_start = clock;
		}
		else {
			if (eve->s_time <= current->s_time) 
			{
				add_to_ready(current);
				R_start = clock;
				current->a_time = eve->a_time;
				current->d_time = eve->d_time;
				current->s_time = eve->s_time;
				current->type = eve->type;
				current->next = NULL;
			}
			else
			{
				add_to_ready(eve);
				R_start = clock;
			}
		}
		if (head->next != NULL) {
			if (head->next->a_time >= (clock + current->s_time) || head->next->d_time >= (clock + current->s_time)) {
				add_dep_node(current);
			}
		}
		else{
			add_dep_node(current);
		}
		break;
	case 3:
		//HRRN ARRIVAL sort
		if (CPU_idle)
		{
			add_dep_node(eve);
			CPU_idle = 0;
			CPU_start = clock;
		}
		else
		{
			SRTF_service_start = clock;
			add_to_ready(eve);
			R_start = clock;
		}
		
		break;
	case 4:
		// RR ARRIVAL sort
		current->s_time = current->s_time - (clock - SRTF_service_start);
		SRTF_service_start = clock;
		if (CPU_idle)
		{
			current->s_time = eve->s_time - (clock - SRTF_service_start);
			current->a_time = eve->a_time;
			current->d_time = eve->d_time;
			current->type = eve->type;
			current->next = NULL;
			if (eve->s_time > quantum)
			{
				add_timeslice();
			}
			else
			{
				add_dep_node(eve);
			}
			CPU_idle = 0;
			CPU_start = clock;
		}
		else
		{
			add_to_ready(eve);
			R_start = clock;
		}

		break;
	}
}
////////////////////////////////////////////////////////////
void process_DEPARTURE(struct event* eve) 
{
	counter++;
	turnaround_count = turnaround_count + (eve->d_time - eve->a_time);
	avg_waiting = avg_waiting + (eve->d_time - eve->a_time) - eve->s_time;
	if (counter == goal)
	{
		CPU_util = CPU_util + (clock - CPU_start);
		return;
	}
	switch (sched_choice) 
	{
		case 1:
			//FCFS DEPARTURE sort
			if (r_head->s_time == 0)
			{
				CPU_idle = 1;
				CPU_end = clock;
				CPU_util = CPU_util + (CPU_end - CPU_start);
			}
			else 
			{
				
				add_dep_node(r_head);
				if (r_head->next != NULL)
					r_head = r_head->next;
				else {
					r_head->s_time = 0;
				}
			}
			break;
		case 2:
			SRTF_service_start = clock;
			if (r_head->s_time == 0)
			{
				CPU_idle = 1;
				CPU_end = clock;
				CPU_util = CPU_util + (CPU_end - CPU_start);
			}
			else
			{
				
				current->a_time = r_head->a_time;
				current->d_time = r_head->d_time;
				current->s_time = r_head->s_time;
				current->type = r_head->type;
				current->next = NULL;
				SRTF_service_start = clock;
				if (head->next != NULL) {
					if (head->next->a_time >= (clock + current->s_time) || head->next->d_time >= (clock + current->s_time))
					{
						add_dep_node(current);
					}
					else {

					}
				}
				if (r_head->next != NULL) 
				{
					r_head = r_head->next;
				}
				else
				{
					r_head->s_time = 0;
				}
			}
			break;
		case 3:
			if (r_head->s_time == 0)
			{
				CPU_idle = 1;
				CPU_end = clock;
				CPU_util = CPU_util + (CPU_end - CPU_start);
			}
			else
			{
				add_dep_node(r_head);
				if (r_head->next != NULL)
					r_head = r_head->next;
				else
					r_head->s_time = 0;
			}
			break;
		case 4:
			/*RR
			*/
			SRTF_service_start = clock;
			if (r_head->s_time == 0)
			{
				CPU_idle = 1;
				CPU_end = clock;
				CPU_util = CPU_util + (CPU_end - CPU_start);
			}
			else
			{
				current->a_time = r_head->a_time;
				current->d_time = r_head->d_time;
				current->s_time = r_head->s_time;
				current->type = r_head->type;
				current->next = NULL;
				if(current->s_time <= quantum)
				{
					add_dep_node(current);
				}
				else {
					add_timeslice();
				}
				if (r_head->next != NULL)
				{
					r_head = r_head->next;
				}
				else
				{
					r_head->s_time = 0;
				}
			}
			break;
	}
}
void process_TIMESLICE() {
	current->type = 1;
	current->s_time = current->s_time - (clock - SRTF_service_start);
	add_to_ready(current);
	SRTF_service_start = clock;
	if (r_head->s_time != 0) {
		current->a_time = r_head->a_time;
		current->d_time = r_head->d_time;
		current->s_time = r_head->s_time;
		current->type = r_head->type;
		current->next = NULL;
		if (current->s_time <= quantum)
			add_dep_node(current);
		else
			add_timeslice();
	}
	if (r_head->next != NULL)
	{
		r_head = r_head->next;
	}
	else
	{
		r_head->s_time = 0;
	}
}
////////////////////////////////////////////////////////////
int run_sim()
{
	struct event* eve = (struct event*) malloc(sizeof(struct event));
	while (counter < goal)
	{
		eve = head;
		switch (eve->type)
		{
		case ARRIVAL:
			clock = eve->a_time;
			process_ARRIVAL(eve);
			break;
		case DEPARTURE:
			clock = eve->d_time;
			process_DEPARTURE(eve);
			break;
		case TIMESLICE:
			clock = eve->d_time;
			process_TIMESLICE();
			break;

		}

		head = eve->next;
		free(eve);
		eve = NULL;
	}
	return 0;
}
////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
	// parse arguments
	init(argc, argv);
	run_sim();
	generate_report();
	return 0;
}