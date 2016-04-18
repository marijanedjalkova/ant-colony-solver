__kernel                                            
void findRoute(__global struct Node *nodes,                        
            __global int *pheromones,                        
            __global struct Node *output)                        
{                                                                                                     
   // Get the work-item’s unique ID                 
   int idx = get_global_id(0);                          
   output[idx] = nodes[idx].cost + pheromones[idx];                        
}           