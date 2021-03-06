bool not_visited(__global double* output, __global double* graph, int position, int k, int idx){
	// check YOUR area of output
	int start = k * idx;
	int end = k*idx+k;
	for (int i = start; i < end; i++){
		if (output[i] == graph[position]){
			return false;
		}
	}
	return true;
} 

void add_nominative(__global double* next_moves, double nomin, int edge_index_in_graph, int k, int idx, __global double* messages){
	// add to YOUR area of next_moves
	for (int i = idx*k*2; i < idx*k*2+k*2; i=i+2){
		if (next_moves[i] == -1.0){
			next_moves[i] = nomin;
			next_moves[i+1] = edge_index_in_graph;
			break;
		}
	}
}

double get_random(__global double* rands, int k, int idx, int count){
	return rands[idx*k+count];
}

int get_next_move(__global double* next_moves, double random, int k, int idx){
	// every other value is possibility and which node it is
	//we have the random number, so go through the array of possibilities
	// and stop at the right place. Choose the next cell for the next node
	double sum = 0.0;
	int choice = -1;
	// pick YOUR area for next moves
	for (int i = idx*k*2; i<idx*k*2+k*2; i=i+2){
		double s = next_moves[i]*(1.0);
		if (next_moves[i]>-1.0){
			if ((sum + next_moves[i]) >=random){
				choice = next_moves[i+1];
				// i + 1 because returning the second value from the tuple
				//(possibility, index_in_graph)
				break;
			} else {
				sum = sum + next_moves[i];
			}
		}
	}
	return choice;
}

void add_to_array(__global double *output, int k, double value, int idx, __global double* messages){
	// add to YOUR area of output
	for (int i = idx*k; i <idx*k+k ; i++){
		if (output[i] == -1.0){
			output[i] = value;
			return;
		}
	}
}

void clear_next_moves(__global double* next_moves, int k, int idx){
	for (int i = idx*k*2; i < idx*k*2+k*2; i++){
		next_moves[i] = -1.0;
	}
}


__kernel                                            
void findRoute(__global double *graph,
			__global double* next_moves,                        
			__global double *output,
			__global int* constK,
			__global double* messages,
			__global double* rands
			)                        
{ 	
	int k = constK[0];
	//next_moves has size k*2*k
	// graph has size edges*4
	// output has size k*k
	int num_edges = k*(k-1)/2;
	double alpha = 2.0;
	double beta = 2.0;             
	int idx = get_global_id(0);
	int idy = get_global_id(1);
	int current_position = idx;
	add_to_array(output, k, current_position, idx, messages);
	bool possible_to_move = true;
	int count = 0;
	
	while(possible_to_move){
		possible_to_move = false;
		double sum = 0.0;
		for (int i = 0; i < num_edges; i++){
			int edge_start = i*4;
			int possible_goal;
			bool fits = false;
			
			if (graph[edge_start]==current_position){
				possible_goal = edge_start+1;
				fits = true;
			} else if(graph[edge_start+1]==current_position) {
				possible_goal = edge_start;
				fits = true;
			}
			if (fits && not_visited(output, graph, possible_goal, k, idx)){
				possible_to_move = true;
				double cost = graph[edge_start + 2];
				double pheromones = graph[edge_start + 3];
				double thao = pow(pheromones, alpha);
				double attr = pow(cost, -beta);
				double nomin = thao * attr;
				sum = sum + nomin;
				add_nominative(next_moves, nomin, edge_start, k, idx, messages);
			}
			
		}
		if (possible_to_move){
			for (int i = idx*k*2; i < idx*k*2+k*2; i=i+2){
				next_moves[i] = next_moves[i] / sum;
			}
			double random = get_random(rands, k, idx, count);
			int edge_choice_index = get_next_move(next_moves, random, k, idx);
			// in that edge, one value is our current position and the other value
			// is the next node
			double next_node = -1.0;
			if (current_position == graph[edge_choice_index]){
				// e.g. edge 1-2 and we are in 1. Put 2 in output and into current pos
				next_node = graph[edge_choice_index + 1];
			} else {
				// e.g. edge 2-1 and we are in 1. Put 2 in output and into current pos
				next_node = graph[edge_choice_index];
			}
			add_to_array(output, k, next_node, idx, messages);
			current_position = next_node;
		} else {
			return;
		}
		clear_next_moves(next_moves, k, idx);
		count = count + 1;
	}
	
} 