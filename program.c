bool not_visited(__global int* output, int position){
	for (int i = 0; i < 5; i++){
		int current_position = i * 4;
		if (output[current_position] == position){
			return false;
		}
	}
	return true;
} 

void add_nominative(__global double* next_moves, double nomin){
	int i = 0;
	bool done = false;
	for (i = 0; i < 5; i++){
		if (!done){
			if (next_moves[i]<0){
				next_moves[i] = nomin;
				done = true;
			}
		}
	}
}


__kernel                                            
void findRoute(__global int *graph,
			__global double* next_moves,                        
            __global int *output)                        
{
	// assume output is of side k*4? 
	
	float alpha = 2.0;
	float beta = 2.0;             
    int start_position = get_global_id(0);
    int current_position = start_position;
    double sum = 0.1;
    for (int i = 0; i < 10; i++){
    	int edge_start = i*4;
   	    if (graph[edge_start]==current_position){
   	    	// can take this node
   	    	if (not_visited(output, edge_start)){
			    int end2 = graph[edge_start + 1];
			    int cost = graph[edge_start + 2];
			    int pheromones = graph[edge_start + 3];
			    double thao = pow(pheromones, alpha);
			    double attr = pow(cost, -beta);
			    double nomin = thao * attr;
			    sum = sum + nomin;
			    add_nominative(next_moves, nomin);
   	    	}
   	    } else if (graph[edge_start+1]==current_position){

   	    }
    }
    for (int i = 0; i < 5; i++){
    	next_moves[i] = next_moves[i] / sum;
    }
    
    
    
   // find allowed edges
   // calculate each TN
   // sum them up to find the sum
   //calculate each probability

} 




