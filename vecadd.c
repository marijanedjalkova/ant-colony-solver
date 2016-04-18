#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>

// OpenCL includes
#include <CL/cl.h>

// OpenCL kernel to perform an element-wise 
// add of two arrays                        
char* programSource ="";

#define k 5

int * graph = NULL;
int * output = NULL;

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

int initialise_output(int elements){
    for (int i = 0; i < elements*4; i++){
        output[i] = 0;
    }
    return 0;
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

int main() {
    //read data from the file
    
    int elements = k*(k-1)/2;
    size_t datasize = sizeof(int)*4*elements;
    graph = (int*)malloc(sizeof(int)*datasize);
    output = (int*)malloc(sizeof(int)*datasize);
    read_program();
    create_graph(elements);
    initialise_output(elements);
    
    // Use this to check the output of each API call
    cl_int status;  
     
    //-----------------------------------------------------
    // STEP 1: Discover and initialize the platforms
    //-----------------------------------------------------
    
    cl_uint numPlatforms = 0;
    cl_platform_id *platforms = NULL;
    
    // Use clGetPlatformIDs() to retrieve the number of 
    // platforms
    status = clGetPlatformIDs(0, NULL, &numPlatforms);
 
    // Allocate enough space for each platform
    platforms =   
        (cl_platform_id*)malloc(
            numPlatforms*sizeof(cl_platform_id));
 
    // Fill in platforms with clGetPlatformIDs()
    status = clGetPlatformIDs(numPlatforms, platforms, 
                NULL);

    //-----------------------------------------------------
    // STEP 2: Discover and initialize the devices
    //----------------------------------------------------- 
    
    cl_uint numDevices = 0;
    cl_device_id *devices = NULL;

    // Use clGetDeviceIDs() to retrieve the number of 
    // devices present
    status = clGetDeviceIDs(
        platforms[0], 
        CL_DEVICE_TYPE_ALL, 
        0, 
        NULL, 
        &numDevices);

    // Allocate enough space for each device
    devices = 
        (cl_device_id*)malloc(
            numDevices*sizeof(cl_device_id));

    // Fill in devices with clGetDeviceIDs()
    status = clGetDeviceIDs(
        platforms[0], 
        CL_DEVICE_TYPE_ALL,        
        numDevices, 
        devices, 
        NULL);

    //-----------------------------------------------------
    // STEP 3: Create a context
    //----------------------------------------------------- 
    
    cl_context context = NULL;

    // Create a context using clCreateContext() and 
    // associate it with the devices
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
    
    cl_command_queue cmdQueue;

    // Create a command queue using clCreateCommandQueue(),
    // and associate it with the device you want to execute 
    // on
    cmdQueue = clCreateCommandQueue(
        context, 
        devices[0], 
        0, 
        &status);

    //-----------------------------------------------------
    // STEP 5: Create device buffers
    //----------------------------------------------------- 
    
    cl_mem bufferGraph; // input
    cl_mem bufferOutput; // output

    // Use clCreateBuffer() to create a buffer object (d_A) 
    // that will contain the data from the host array A
    bufferGraph = clCreateBuffer(
        context, 
        CL_MEM_READ_ONLY,                         
        datasize, 
        NULL, 
        &status);

    // Use clCreateBuffer() to create a buffer object (d_C) 
    // with enough space to hold the output data
    bufferOutput = clCreateBuffer(
        context, 
        CL_MEM_WRITE_ONLY,                 
        datasize, 
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
        datasize,                         
        graph, 
        0, 
        NULL, 
        NULL);

    //-----------------------------------------------------
    // STEP 7: Create and compile the program
    //----------------------------------------------------- 
     
    // Create a program using clCreateProgramWithSource()
    cl_program program = clCreateProgramWithSource(
        context, 
        1, 
        (const char**)&programSource,                                 
        NULL, 
        &status);

    // Build (compile) the program for the devices with
    // clBuildProgram()
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

    cl_kernel kernel = NULL;
    kernel = clCreateKernel(program, "findRoute", &status);

    //-----------------------------------------------------
    // STEP 9: Set the kernel arguments
    //----------------------------------------------------- 
    
    // Associate the input and output buffers with the 
    // kernel 
    // using clSetKernelArg()
    status  = clSetKernelArg(
        kernel, 
        0, 
        sizeof(cl_mem), 
        &bufferGraph);
    status |= clSetKernelArg(
        kernel, 
        1, 
        sizeof(cl_mem), 
        &bufferOutput);

    //-----------------------------------------------------
    // STEP 10: Configure the work-item structure
    //----------------------------------------------------- 
    
    // Define an index space (global work size) of work 
    // items for 
    // execution. A workgroup size (local work size) is not 
    // required, 
    // but can be used.
    size_t globalWorkSize[1];    
    // There are 'elements' work-items 
    globalWorkSize[0] = elements;

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
    for(int i = 0; i < elements; i++) {
        for (int j = 0; j < 4; j++){
            if (j==0){
                printf("Before pulling: %d - %d \n", output[i*4+j], output[i*4+j+1]);
            }
        }
        
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

    printf("%s\n", programSource);
    
    // Verify the output
    for(int i = 0; i < elements; i++) {
        for (int j = 0; j < 4; j++){
            if (j==0){
                printf("Afer pulling: %d - %d \n", output[i*4+j], output[i*4+j+1]);
            }
        }
        
    }
    
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