__kernel                                            
void findRoute(__global int *graph,                        
            __global int *output)                        
{
	// Get the work-item’s unique ID
	// an edge is defined by the id.
	// start of the edge info is idx*4 
	/*                
    int idx = get_global_id(0);
    int edge_index = idx;
    end1 = graph[edge_index*4];
    end2 = graph[edge_index*4 + 1];
    cost = graph[edge_index*4 + 2];
    pheromones = graph[edge_index*4 + 3];
    for (int i = 0; i < 10; i=i+4){
   	    if (i != edge_index){
            if (graph[i] == end1){

            }
   	    }
    }*/
   // find allowed edges
   // calculate each TN
   // sum them up to find the sum
   //calculate each probability
   int idx = get_global_id(0);
   output[idx] = idx;

}           