extern "C"
{
#include "simlib.h"             
}
#include <iostream>
#include <string>
#include <fstream>


//types
#define ITEM_COUNT 2
#define ITEM_TYPE_JACKET 0
#define ITEM_TYPE_PANT 1
#define ITEM_STATUS_DAMAGED 0
#define ITEM_STATUS_UNDAMAGED 1
constexpr float PROBABILITY_OF_DAMAGED[] = {0.05, 0.10};

//Events
#define EVENT_ARRIVEL_SYSTEM					1  
#define EVENT_DEPARTURE_FIRST_SERVER			2	
#define EVENT_DEPARTURE_PARTS_SERVER			3
#define EVENT_DEPARTURE_ASSMEMBLY_SERVER		4
#define EVENT_DEPARTURE_FIXING_SERVER           5
#define EVENT_END								6
//statistic variables
#define SAMPSET_DAMAGED_TIME_DELAY				1
#define SAMPSET_UNDAMAGED_TIME_DELAY	        2

//streams
#define STREAM_FIRST_SERVER_SERVICE_TIME		1 
constexpr int STREAM_PARTS_SERVER_SERVICE_TIME[] = { 2,3 };
constexpr int STREAM_ASSEMBLER_SERVER_SERVICE_TIME[] = { 5,4};
#define STREAM_FIXING_SERVER_SERVICE_TIME		6
#define STREAM_INTERARRIVAL						7
constexpr int STREAM_ITEM_STATUS[] = { 8,9 };

//event attributes
#define EVENT_ATTR_ITEM_TYPE					3

//suit (queue/server) attributes
#define ATTR_ITEM_TYPE						1
#define ATTR_ITEM_STATUS					2
#define ATTR_SYSTEM_ARRIVAL					3

//server lists
#define LIST_FIRST_SERVER						1
constexpr int LIST_PARTS_SERVER[] =			{ 2,3 };
#define LIST_ASSEMBLY_SERVER					4
#define LIST_FIXING_SERVER						5

//queue lists
#define LIST_QUEUE_FIRST_SERVER					6
constexpr int LIST_QUEUE_PARTS_SERVER[] =	{ 7,8 };
constexpr int LIST_QUEUE_ASSEMBLY_SERVER[] ={ 9,10 };
#define LIST_QUEUE_FIXING_SERVER					11


//settings [TIME IS IN MINUTES]
#define SIMULATION_PERIOD						12 * 60 
#define MEAN_INTERARRIVAL_FIRST_SERVER			10 
#define MEAN_FIRST_SERVER_SERVICE_TIME			6
constexpr int MEAN_PARTS_SERVICE_TIME[] =		{ 4,5 };
constexpr int MEAN_ASSEMBLER_SERVICE_TIME[] =	{ 8,5 };
#define MEAN_FIXING_SERVER_SERVICE_TIME			12
float probability_distribution[2][3]; //simlib random int start from 1, so use 3 indices though we only need 2


bool stop_simulation = false;
int iter = 1;


FILE* outfile;

void init_model();
void init_probability_distribution();
void event_arrive();
void first_server_departure();
void parts_server_departure();
void fixing_server_departure();
void assembly_server_departure();
void report();
int assemble_suite_items();
void emptyList(int list_number);
bool is_server_busy(int server_id);
bool is_queue_busy(int queue_id);
bool shouldBeDamadged(int item_type);
void set_server_status(bool is_busy, int server_id);
bool has_found_matching_suite_items(int item_type);


int main()  /* Main function. */
{
	outfile = fopen("ques26.out", "w");

	/* Write report heading and input parameters. */

	fprintf(outfile, "Hard Problem  using simlib\n\n");
	
	fprintf(outfile, "Stop after %d  minutes\n\n\n", SIMULATION_PERIOD);


	/* Initialize simlib */
	init_simlib();
	/* Set maxatr = max(maximum number of attributes per record, 4) */
	maxatr = 6;  /* NEVER SET maxatr TO BE SMALLER THAN 4. */

	init_probability_distribution();

	float total_damaged_delay = 0, total_undamaged_delay = 0;
	float first_server_utilization = 0, assembly_server_utilization = 0, fixing_server_utilization = 0;
	float parts_server_utilization[] = { 0,0 };
	float first_queue_length = 0, fixing_queue_length = 0;
	float parts_queue_length[] = { 0,0 };
	float assembly_queue_length[] = { 0,0 };

	/* Run the simulation while more delays are still needed. */
	for (int i = 0; i < iter; i++)
	{

		/* Initialize the model. */
		init_model();
		while (!stop_simulation) {

			/* Determine the next event. */
			timing();

			/* Invoke the appropriate event function. */
			switch (next_event_type) {
			case EVENT_ARRIVEL_SYSTEM:
				event_arrive();
				break;
			case EVENT_DEPARTURE_FIRST_SERVER	:
				first_server_departure();
				break;
			case EVENT_DEPARTURE_PARTS_SERVER:
				parts_server_departure();
				break;
			case EVENT_DEPARTURE_ASSMEMBLY_SERVER:
				assembly_server_departure();
				break;
			case EVENT_DEPARTURE_FIXING_SERVER:
				fixing_server_departure();
				break;
			case EVENT_END:
				stop_simulation = true;
				break;
			}
		}

		/* Invoke the report generator and end the simulation. */

		report();
		
		total_damaged_delay += sampst(0, -SAMPSET_DAMAGED_TIME_DELAY);
		total_undamaged_delay += sampst(0, -SAMPSET_UNDAMAGED_TIME_DELAY);

		first_server_utilization += filest(LIST_FIRST_SERVER);
		assembly_server_utilization += filest(LIST_ASSEMBLY_SERVER);
		fixing_server_utilization += filest(LIST_FIXING_SERVER);
		parts_server_utilization[ITEM_TYPE_JACKET] += filest(LIST_PARTS_SERVER[ITEM_TYPE_JACKET]);
		parts_server_utilization[ITEM_TYPE_PANT] += filest(LIST_PARTS_SERVER[ITEM_TYPE_PANT]);

		first_queue_length += filest(LIST_QUEUE_FIRST_SERVER);
		fixing_queue_length += filest(LIST_QUEUE_FIXING_SERVER);
		parts_queue_length[ITEM_TYPE_JACKET] += filest(LIST_QUEUE_PARTS_SERVER[ITEM_TYPE_JACKET]);
		parts_queue_length[ITEM_TYPE_PANT] += filest(LIST_QUEUE_PARTS_SERVER[ITEM_TYPE_PANT]);
		assembly_queue_length[ITEM_TYPE_JACKET] +=filest(LIST_QUEUE_ASSEMBLY_SERVER[ITEM_TYPE_JACKET]);
		assembly_queue_length[ITEM_TYPE_PANT] += filest(LIST_QUEUE_ASSEMBLY_SERVER[ITEM_TYPE_PANT]);


		fprintf(outfile, "\n\n");
	}
	total_damaged_delay /= iter;
	total_undamaged_delay /= iter;

	first_server_utilization /= iter;
	assembly_server_utilization /= iter;
	fixing_server_utilization /= iter;
	parts_server_utilization[ITEM_TYPE_JACKET] /= iter;
	parts_server_utilization[ITEM_TYPE_PANT] /= iter;

	first_queue_length /= iter;
	fixing_queue_length /= iter;
	parts_queue_length[ITEM_TYPE_JACKET] /= iter;
	parts_queue_length[ITEM_TYPE_PANT] /= iter;
	assembly_queue_length[ITEM_TYPE_JACKET] /= iter;
	assembly_queue_length[ITEM_TYPE_PANT] /= iter;

	fprintf(outfile, "\n\n\nAverages:\n\n");
	fprintf(outfile, "\ntotal_damaged_delay: %f", total_damaged_delay);
	fprintf(outfile, "\ntotal_undamaged_delay: %f", total_undamaged_delay);

	fprintf(outfile, "\n\nfirst_server_utilization: %f", first_server_utilization);
	fprintf(outfile, "\nassembly_server_utilization: %f", assembly_server_utilization);
	fprintf(outfile, "\nfixing_server_utilization: %f", fixing_server_utilization);
	fprintf(outfile, "\nparts_server_utilization[ITEM_TYPE_JACKET]: %f", parts_server_utilization[ITEM_TYPE_JACKET]);
	fprintf(outfile, "\nparts_server_utilization[ITEM_TYPE_PANT]: %f", parts_server_utilization[ITEM_TYPE_PANT]);

	fprintf(outfile, "\n\nfirst_queue_length: %f", first_queue_length);
	fprintf(outfile, "\nfixing_queue_length: %f", fixing_queue_length);
	fprintf(outfile, "\nparts_queue_length[ITEM_TYPE_JACKET]: %f", parts_queue_length[ITEM_TYPE_JACKET]);
	fprintf(outfile, "\nparts_queue_length[ITEM_TYPE_PANT]: %f", parts_queue_length[ITEM_TYPE_PANT]);
	fprintf(outfile, "\nassembly_queue_length[ITEM_TYPE_JACKET]: %f", assembly_queue_length[ITEM_TYPE_JACKET]);
	fprintf(outfile, "\nassembly_queue_length[ITEM_TYPE_PANT]: %f", assembly_queue_length[ITEM_TYPE_PANT]);
	fclose(outfile);
	return 0;
}



void emptyList(int list_number)
{
	while (list_size[list_number] > 0)
	{
		list_remove(FIRST, list_number);
	}
}
bool is_server_busy(int server_id)
{
	return list_size[server_id] > 0;
}

bool is_queue_busy(int queue_id)
{
	return is_server_busy(queue_id);
}
void set_server_status(bool is_busy, int server_id)
{
	if (is_busy)
		list_file(LAST, server_id);
	else
		list_remove(FIRST, server_id);
}
bool has_found_matching_suite_items(int item_type)
{
	for (int i = 0; i < ITEM_COUNT; i++)
	{
		if (i == item_type)
			continue;
		if (list_size[LIST_QUEUE_ASSEMBLY_SERVER[i]] == 0)
			return false;
	}

	return true;
	
}
void init_model()
{
	emptyList(LIST_EVENT);

	emptyList(LIST_ASSEMBLY_SERVER);
	emptyList(LIST_FIRST_SERVER);
	emptyList(LIST_FIXING_SERVER);
	emptyList(LIST_PARTS_SERVER[ITEM_TYPE_JACKET]);
	emptyList(LIST_PARTS_SERVER[ITEM_TYPE_PANT]);

	emptyList(LIST_QUEUE_FIRST_SERVER);
	emptyList(LIST_QUEUE_FIXING_SERVER);
	emptyList(LIST_QUEUE_ASSEMBLY_SERVER[ITEM_TYPE_JACKET]);
	emptyList(LIST_QUEUE_ASSEMBLY_SERVER[ITEM_TYPE_PANT]);
	emptyList(LIST_QUEUE_PARTS_SERVER[ITEM_TYPE_JACKET]);
	emptyList(LIST_QUEUE_ASSEMBLY_SERVER[ITEM_TYPE_PANT]);
	sampst(0, 0);
	timest(0, 0);
	sim_time = 0;

	list_rank[LIST_EVENT] = EVENT_TIME;

	//event schedule
	event_schedule(sim_time+expon(MEAN_INTERARRIVAL_FIRST_SERVER, STREAM_INTERARRIVAL), EVENT_ARRIVEL_SYSTEM);
	event_schedule(sim_time+SIMULATION_PERIOD, EVENT_END);

}
void init_probability_distribution()
{
	probability_distribution[ITEM_TYPE_JACKET][ITEM_STATUS_DAMAGED+1] = PROBABILITY_OF_DAMAGED[ITEM_TYPE_JACKET];
	probability_distribution[ITEM_TYPE_JACKET][ITEM_STATUS_UNDAMAGED+1] = 1 - PROBABILITY_OF_DAMAGED[ITEM_TYPE_JACKET];
	probability_distribution[ITEM_TYPE_PANT][ITEM_STATUS_DAMAGED+1] = PROBABILITY_OF_DAMAGED[ITEM_TYPE_PANT];
	probability_distribution[ITEM_TYPE_PANT][ITEM_STATUS_UNDAMAGED+1] = 1 - PROBABILITY_OF_DAMAGED[ITEM_TYPE_PANT];
}
void event_arrive()
{
	float event_time = transfer[EVENT_TIME];

	//set suite arrive time
	transfer[ATTR_SYSTEM_ARRIVAL] = event_time;

	//check if we should add to queue
	if (is_queue_busy(LIST_QUEUE_FIRST_SERVER) || is_server_busy(LIST_FIRST_SERVER))
	{
		//add to queue
		list_file(LAST, LIST_QUEUE_FIRST_SERVER);
	}
	else 
	{
		//add user to server
		set_server_status(true, LIST_FIRST_SERVER);
		//schedule departure with system arrival
		event_schedule(sim_time + expon(MEAN_FIRST_SERVER_SERVICE_TIME, STREAM_FIRST_SERVER_SERVICE_TIME),
			EVENT_DEPARTURE_FIRST_SERVER);
	}

	//schedule next arrival
	event_schedule(sim_time + expon(MEAN_INTERARRIVAL_FIRST_SERVER, STREAM_INTERARRIVAL), EVENT_ARRIVEL_SYSTEM);
		

}
void first_server_departure()
{
	//remove current suite from the dry cleaner
	set_server_status(false, LIST_FIRST_SERVER);

	//save the arrival as transfer will be messed up when we scedule a parts departure event
	int suite_system_arrival = transfer[ATTR_SYSTEM_ARRIVAL];

	//split suit, and let each item go to its corresponding server
	for (int i = 0; i < ITEM_COUNT; i++)
	{
		//set this item info
		transfer[ATTR_ITEM_TYPE] = i;
		transfer[ATTR_SYSTEM_ARRIVAL] = suite_system_arrival;

		//check if we should add to queue
		if (is_queue_busy(LIST_QUEUE_PARTS_SERVER[i]) || is_server_busy(LIST_PARTS_SERVER[i]))
		{
			//add to queue
			list_file(LAST, LIST_QUEUE_PARTS_SERVER[i]);
		}
		else
		{
			//add to parts server
			set_server_status(true, LIST_PARTS_SERVER[i]);

			//schedule departure event
			transfer[EVENT_ATTR_ITEM_TYPE] = i;
			event_schedule(sim_time+expon(MEAN_PARTS_SERVICE_TIME[i], STREAM_PARTS_SERVER_SERVICE_TIME[i]), EVENT_DEPARTURE_PARTS_SERVER);

		}
	}
	//check if queue has suites
	if (is_queue_busy(LIST_QUEUE_FIRST_SERVER))
	{
		//remove from queue
		list_remove(FIRST, LIST_QUEUE_FIRST_SERVER);
		//let the suit enter the dry cleaner
		set_server_status(true, LIST_FIRST_SERVER);
		//schedule this suite  departure
		event_schedule(sim_time+expon(MEAN_FIRST_SERVER_SERVICE_TIME, STREAM_FIRST_SERVER_SERVICE_TIME), EVENT_DEPARTURE_FIRST_SERVER);
	}
}
void parts_server_departure()
{
	int item_type = transfer[EVENT_ATTR_ITEM_TYPE];
	//remove this suite item from server
	set_server_status(false, LIST_PARTS_SERVER[item_type]);

	//check if this item would be damadged
	transfer[ATTR_ITEM_STATUS] = shouldBeDamadged(item_type) ? ITEM_STATUS_DAMAGED:ITEM_STATUS_UNDAMAGED;

	// check if the assembly server busy
	if (is_server_busy(LIST_ASSEMBLY_SERVER))
	{
		list_file(LAST, LIST_QUEUE_ASSEMBLY_SERVER[item_type]);
	}
	else
	{
		//check if all parts of the suite are done
		if (has_found_matching_suite_items(item_type))
		{
			if (list_size[LIST_QUEUE_ASSEMBLY_SERVER[item_type]] != 0)
			{
				throw std::runtime_error("Unexpected behaviour: item part added but not found in the beginning of the queue");
			}
			//put this new part in the queue (just for being consistence, it will be removed immedeatly)
			list_file(LAST, LIST_QUEUE_ASSEMBLY_SERVER[item_type]);
			//assemble those parts
			int item_status = assemble_suite_items();
			//schedule departure for assembly server
			event_schedule(sim_time+expon(MEAN_ASSEMBLER_SERVICE_TIME[item_status],
				STREAM_ASSEMBLER_SERVER_SERVICE_TIME[item_status]), EVENT_DEPARTURE_ASSMEMBLY_SERVER);

		}
		else
		{
			list_file(LAST, LIST_QUEUE_ASSEMBLY_SERVER[item_type]);
		}
	}

	//check if queue has suites
	if (is_queue_busy(LIST_QUEUE_PARTS_SERVER[item_type]))
	{
		//remove from queue
		list_remove(FIRST, LIST_QUEUE_PARTS_SERVER[item_type]);
		//let the suit enter the dry cleaner
		set_server_status(true, LIST_PARTS_SERVER[item_type]);
		//schedule this suite  departure
		transfer[EVENT_ATTR_ITEM_TYPE] = item_type;
		event_schedule(sim_time+expon(MEAN_PARTS_SERVICE_TIME[item_type], STREAM_PARTS_SERVER_SERVICE_TIME[item_type]),
			EVENT_DEPARTURE_PARTS_SERVER);
	}

}

void assembly_server_departure()
{
	//remove suite
	set_server_status(false, LIST_ASSEMBLY_SERVER);
	int item_status = transfer[ATTR_ITEM_STATUS];

	//if suite is damadged send to fixing server
	if (item_status == ITEM_STATUS_DAMAGED)
	{
		//check if we should add to queue
		if (is_queue_busy(LIST_QUEUE_FIXING_SERVER) || is_server_busy(LIST_FIXING_SERVER))
		{
			//add to queue
			list_file(LAST, LIST_QUEUE_FIXING_SERVER);
		}
		else
		{
			//add user to server
			set_server_status(true, LIST_FIXING_SERVER);
			//schedule departure with system arrival
			event_schedule(sim_time+expon(MEAN_FIXING_SERVER_SERVICE_TIME, STREAM_FIXING_SERVER_SERVICE_TIME), EVENT_DEPARTURE_FIXING_SERVER);
		}
	}
	else
	{
		//calculate statistics
		sampst(sim_time - transfer[ATTR_SYSTEM_ARRIVAL], SAMPSET_UNDAMAGED_TIME_DELAY);
	}

	//check if a new suite exist and we should assemble it
	if (has_found_matching_suite_items(-1))
	{
		//assemble those parts
		int item_status = assemble_suite_items();
		//schedule departure for assembly server
		event_schedule(sim_time+expon(MEAN_ASSEMBLER_SERVICE_TIME[item_status],
			STREAM_ASSEMBLER_SERVER_SERVICE_TIME[item_status]), EVENT_DEPARTURE_ASSMEMBLY_SERVER);
	}


}


void fixing_server_departure() 
{
	//remove current suite
	set_server_status(false, LIST_FIXING_SERVER);

	//calculate statistics
	sampst(sim_time - transfer[ATTR_SYSTEM_ARRIVAL], SAMPSET_DAMAGED_TIME_DELAY);
	//check if suites exist on queue
	if (is_queue_busy(LIST_QUEUE_FIXING_SERVER))
	{
		//remove from queue
		list_remove(FIRST, LIST_QUEUE_FIXING_SERVER);
		//place on server
		set_server_status(true, LIST_FIXING_SERVER);
		//schedule depart
		event_schedule(sim_time + expon(MEAN_FIXING_SERVER_SERVICE_TIME, STREAM_FIXING_SERVER_SERVICE_TIME), EVENT_DEPARTURE_FIXING_SERVER);

	}
}




int assemble_suite_items()
{
	//remove suite's parts from all queues
	bool damadged = false;
	for (int i = 0; i < ITEM_COUNT; i++)
	{
		//remove the part
		list_remove(FIRST, LIST_QUEUE_ASSEMBLY_SERVER[i]);
		if (transfer[ATTR_ITEM_STATUS] == ITEM_STATUS_DAMAGED)
		{
			damadged = true;
		}
	}

	//use the last transfer to represent the suite
	transfer[ATTR_ITEM_STATUS] = damadged ? ITEM_STATUS_DAMAGED : ITEM_STATUS_UNDAMAGED;

	//put the suite in the server
	set_server_status(true, LIST_ASSEMBLY_SERVER);

	return transfer[ATTR_ITEM_STATUS];


}
bool shouldBeDamadged(int item_type)
{
	//if item type invalid end
	if (item_type < 0 || item_type >= ITEM_COUNT)
	{
		throw std::runtime_error("Item type " + std::to_string(item_type) + " is invalid.");
	}

	return random_integer(probability_distribution[ITEM_TYPE_PANT], STREAM_ITEM_STATUS[ITEM_TYPE_PANT]) == ITEM_STATUS_DAMAGED + 1;
}

void report(void)  
{
	fprintf(outfile, "\nSAMPSET_DAMAGED_TIME_DELAY\n");
	out_sampst(outfile, SAMPSET_DAMAGED_TIME_DELAY, SAMPSET_DAMAGED_TIME_DELAY);
	fprintf(outfile, "\nSAMPSET_UNDAMAGED_TIME_DELAY\n");
	out_sampst(outfile, SAMPSET_UNDAMAGED_TIME_DELAY, SAMPSET_UNDAMAGED_TIME_DELAY);



}
