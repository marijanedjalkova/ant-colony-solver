#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>

// OpenCL includes
#include <CL/cl.h>

// OpenCL kernel to perform an element-wise 
// add of two arrays                        
char* programSource ="";

const int k = 5;

int * graph = NULL;
int * output = NULL;
double * next_moves = NULL;

cl_kernel kernel = NULL;
cl_platform_id *platforms = NULL;
cl_device_id *devices = NULL;
size_t globalWorkSize[1];
cl_int status;  
cl_mem bufferGraph; // input
cl_mem bufferOutput; // output
cl_mem bufferNext;
cl_mem bufferK;
cl_context context = NULL;   
cl_command_queue cmdQueue;
int elements;
size_t datasize, graphDatasize, nextDatasize, outputDatasize;
bool stop = false;
cl_program program;

int create_graph(int elements){
    int lasti = 0;
    int lastj = 1;
    for (int i = 0; i < elements; i++){
        for (int j = 0; j < 4; j++){
            if (j==0){
                if (lastj==5){
                    lasti = lasti + 1;
                    lastj = lasti + 1;
                }
                graph[i*4+j] = lasti;
                lastj = lastj + 1;
            }
            if (j == 1){
                if (lastj==5){
                    lasti = lasti + 1;
                    lastj = lasti + 1;
                }
                graph[i*4+j] = lastj;
                lastj = lastj + 1;
            }
            if (j == 2){
                // cost
                graph[i*4+j] = 1;
            }
            if (j = 3){
                //pheromones
                graph[i*4+j] = 0;
            }
        }
    }
    return 0;
} 

void initialise_output(int elements){
    for (int i = 0; i < elements*4; i++){
        output[i] = -1;
    }
}

int read_program(){
    // read the program from file
    char * buffer = 0;
    long length;
    FILE * f = fopen ("program.c", "rb");
    if (f){
      fseek (f, 0, SEEK_END);
      length = ftell (f);
      fseek (f, 0, SEEK_SET);
      buffer = malloc (length);
      if (buffer){
        fread (buffer, 1, length, f);
      }
      fclose (f);
    }
    programSource = buffer;
    return 0;
}

void initialise(){
    elements = k*(k-1)/2;
    datasize = sizeof(int)*4*elements;
    graphDatasize = sizeof(int)*4*elements;
    nextDatasize = sizeof(double)*k*2;
    outputDatasize = sizeof(int)*k;
    graph = (int*)malloc(sizeof(int)*datasize);
    output = (int*)malloc(sizeof(int)*datasize);
    next_moves = (double*)malloc(sizeof(double)*k);
    for (int i = 0; i <k; i++){
        next_moves[i] = -1;
    }
    read_program();
    create_graph(elements);
    initialise_output(elements);
    
    //-----------------------------------------------------
    // STEP 1: Discover and initialize the platforms
    //-----------------------------------------------------
    
    cl_uint numPlatforms = 0;
    status = clGetPlatformIDs(0, NULL, &numPlatforms);
    platforms =   
        (cl_platform_id*)malloc(
            numPlatforms*sizeof(cl_platform_id));
    status = clGetPlatformIDs(numPlatforms, platforms, 
                NULL);

    //-----------------------------------------------------
    // STEP 2: Discover and initialize the devices
    //----------------------------------------------------- 
    
    cl_uint numDevices = 0;
    status = clGetDeviceIDs(
        platforms[0], 
        CL_DEVICE_TYPE_ALL, 
        0, 
        NULL, 
        &numDevices);

    devices = 
        (cl_device_id*)malloc(
            numDevices*sizeof(cl_device_id));

    status = clGetDeviceIDs(
        platforms[0], 
        CL_DEVICE_TYPE_ALL,        
        numDevices, 
        devices, 
        NULL);

    //-----------------------------------------------------
    // STEP 3: Create a context
    //-----------------------------------------------------

    context = clCreateContext(
        NULL, 
        numDevices, 
        devices, 
        NULL, 
        NULL, 
        &status);

    //-----------------------------------------------------
    // STEP 4: Create a command queue
    //----------------------------------------------------- 

    cmdQueue = clCreateCommandQueue(
        context, 
        devices[0], 
        0, 
        &status);

    //-----------------------------------------------------
    // STEP 5: Create device buffers
    //----------------------------------------------------- 

    bufferGraph = clCreateBuffer(
        context, 
        CL_MEM_READ_ONLY,                         
        graphDatasize, 
        NULL, 
        &status);

    bufferNext = clCreateBuffer(
        context, 
        CL_MEM_READ_WRITE,                         
        nextDatasize, 
        NULL, 
        &status);

    bufferK = clCreateBuffer(
        context, 
        CL_MEM_READ_WRITE,                 
        sizeof(int), 
        NULL, 
        &status);

    bufferOutput = clCreateBuffer(
        context, 
        CL_MEM_READ_WRITE,                 
        outputDatasize, 
        NULL, 
        &status);
    
    //-----------------------------------------------------
    // STEP 6: Write host data to device buffers
    //----------------------------------------------------- 
    
    status = clEnqueueWriteBuffer(
        cmdQueue, 
        bufferGraph, 
        CL_FALSE, 
        0, 
        graphDatasize,                         
        graph, 
        0, 
        NULL, 
        NULL);

    status = clEnqueueWriteBuffer(
        cmdQueue, 
        bufferNext, 
        CL_FALSE, 
        0, 
        nextDatasize,                         
        graph, 
        0, 
        NULL, 
        NULL);

    status = clEnqueueWriteBuffer(
        cmdQueue, 
        bufferK, 
        CL_FALSE, 
        0, 
        sizeof(int),                         
        &k, 
        0, 
        NULL, 
        NULL);

    //-----------------------------------------------------
    // STEP 7: Create and compile the program
    //----------------------------------------------------- 
     
    program = clCreateProgramWithSource(
        context, 
        1, 
        (const char**)&programSource,                                 
        NULL, 
        &status);
    if (status<0){
        fprintf(stderr, "%s\n", "Error while creating program");
    }

    status = clBuildProgram(
        program, 
        numDevices, 
        devices, 
        NULL, 
        NULL, 
        NULL);
   
    //-----------------------------------------------------
    // STEP 8: Create the kernel
    //----------------------------------------------------- 

    kernel = clCreateKernel(program, "findRoute", &status);
    if (status<0){
        fprintf(stderr, "%s\n", "Error while creating kernel");
    }

    //-----------------------------------------------------
    // STEP 10: Configure the work-item structure
    //----------------------------------------------------- 
    
        
    globalWorkSize[0] = k;

}

bool finished(){
    return stop == true;
}

void process_result(){
    for(int i = 0; i < k; i++) {
        printf("Output: %d \n", output[i]);
    }
}

void global_update_pheromones(){

}

void cleanup(){
    //-----------------------------------------------------
    // STEP 13: Release OpenCL resources
    //----------------------------------------------------- 
    
    // Free OpenCL resources
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(cmdQueue);
    clReleaseMemObject(bufferGraph);
    clReleaseMemObject(bufferOutput);
    clReleaseContext(context);

    // Free host resources
    free(graph);
    free(output);
    free(platforms);
    free(devices);
}

void construct_solution(){
    //-----------------------------------------------------
    // STEP 9: Set the kernel arguments
    //----------------------------------------------------- 
 
    status  = clSetKernelArg(
        kernel, 
        0, 
        sizeof(cl_mem), 
        &bufferGraph);
    status |= clSetKernelArg(
        kernel, 
        1, 
        sizeof(cl_mem), 
        &bufferNext);
    status |= clSetKernelArg(
        kernel, 
        2, 
        sizeof(cl_mem), 
        &bufferOutput);
    status |= clSetKernelArg(
        kernel, 
        3, 
        sizeof(cl_mem), 
        &bufferK);

    if (status < 0) {
        fprintf(stderr, "%s\n", "Error when setting arguments!");
    }
    //-----------------------------------------------------
    // STEP 11: Enqueue the kernel for execution
    //----------------------------------------------------- 
    
    status = clEnqueueNDRangeKernel(
        cmdQueue, 
        kernel, 
        1, 
        NULL, 
        globalWorkSize, 
        NULL, 
        0, 
        NULL, 
        NULL);

    if (status < 0) {
        fprintf(stderr, "%s\n", "Error in the end!");
    }
    
    //-----------------------------------------------------
    // STEP 12: Read the output buffer back to the host
    //----------------------------------------------------- 

    clEnqueueReadBuffer(
        cmdQueue, 
        bufferOutput, 
        CL_TRUE, 
        0, 
        datasize, 
        output, 
        0, 
        NULL, 
        NULL);

    process_result();
}

int aco(){
    initialise();
    while (!finished()){
        construct_solution();
        global_update_pheromones();
        stop = true;
    }
}

int main() {
    aco();
    cleanup(); 
}