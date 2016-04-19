bool not_visited(__global int* output, int position){
	for (int i = 0; i < 5; i++){
		int current_position = i * 4;
		if (output[current_position] == position){
			return false;
		}
	}
	return true;
} 

void add_nominative(__global double* next_moves, double nomin, int node){
	int i = 0;
	bool done = false;
	for (i = 0; i < 5; i++){
		if (!done){
			int index = i*2;
			if (next_moves[index]<0){
				next_moves[index] = nomin;
				next_moves[index+1] = node;
				done = true;
			}
		}
	}
}

double get_random(){
	unsigned short lfsr = 0xACE1u;
	unsigned bit  = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5) ) & 1;
    lfsr =  (lfsr >> 1) | (bit << 15);
    return lfsr / 10.0;
}

int get_next_move(__global double* next_moves, double random){
	// every other value is possibility and which node it is

    return 0;
}


__kernel                                            
void findRoute(__global int *graph,
			__global double* next_moves,                        
            __global int *output)                        
{ 
	
	float alpha = 2.0;
	float beta = 2.0;             
    int start_position = get_global_id(0);
    int current_position = start_position;
    double sum = 0.1;
    for (int i = 0; i < 10; i++){
    	int edge_start = i*4;
   	    if (graph[edge_start]==current_position){
   	    	// can take this node
   	    	int possible_goal = edge_start;
   	    	if (not_visited(output, possible_goal)){
			    int end2 = graph[edge_start + 1];
			    int cost = graph[edge_start + 2];
			    int pheromones = graph[edge_start + 3];
			    double thao = pow(pheromones, alpha);
			    double attr = pow(cost, -beta);
			    double nomin = thao * attr;
			    sum = sum + nomin;
			    add_nominative(next_moves, nomin, possible_goal);
   	    	}
   	    } else if (graph[edge_start+1]==current_position){
   	    	int possible_goal = edge_start + 1;
   	    	if (not_visited(output, possible_goal)){
   	    		int end2 = graph[edge_start];
			    int cost = graph[edge_start + 2];
			    int pheromones = graph[edge_start + 3];
			    double thao = pow(pheromones, alpha);
			    double attr = pow(cost, -beta);
			    double nomin = thao * attr;
			    sum = sum + nomin;
			    add_nominative(next_moves, nomin, possible_goal);
   	    	}
   	    }
    }
    for (int i = 0; i < 5; i++){
    	next_moves[i] = next_moves[i] / sum;
    }
    double random = get_random();
    current_position = get_next_move(next_moves, random);
    
    
    
   // find allowed edges
   // calculate each TN
   // sum them up to find the sum
   //calculate each probability

} 




