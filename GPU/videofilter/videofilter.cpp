#include <stdio.h>
#include <stdlib.h>
#include <iostream> // for standard I/O
#include <fstream>
#include <time.h>
#include "opencv2/opencv.hpp"
#include <math.h>
#include <CL/cl.h>
#include <CL/cl_ext.h>

using namespace cv;
using namespace std;

#define SHOW
#define STRING_BUFFER_LEN 1024


const char *getErrorString(cl_int error)
{
switch(error){
    // run-time and JIT compiler errors
    case 0: return "CL_SUCCESS";
    case -1: return "CL_DEVICE_NOT_FOUND";
    case -2: return "CL_DEVICE_NOT_AVAILABLE";
    case -3: return "CL_COMPILER_NOT_AVAILABLE";
    case -4: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
    case -5: return "CL_OUT_OF_RESOURCES";
    case -6: return "CL_OUT_OF_HOST_MEMORY";
    case -7: return "CL_PROFILING_INFO_NOT_AVAILABLE";
    case -8: return "CL_MEM_COPY_OVERLAP";
    case -9: return "CL_IMAGE_FORMAT_MISMATCH";
    case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
    case -11: return "CL_BUILD_PROGRAM_FAILURE";
    case -12: return "CL_MAP_FAILURE";
    case -13: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
    case -14: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
    case -15: return "CL_COMPILE_PROGRAM_FAILURE";
    case -16: return "CL_LINKER_NOT_AVAILABLE";
    case -17: return "CL_LINK_PROGRAM_FAILURE";
    case -18: return "CL_DEVICE_PARTITION_FAILED";
    case -19: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

    // compile-time errors
    case -30: return "CL_INVALID_VALUE";
    case -31: return "CL_INVALID_DEVICE_TYPE";
    case -32: return "CL_INVALID_PLATFORM";
    case -33: return "CL_INVALID_DEVICE";
    case -34: return "CL_INVALID_CONTEXT";
    case -35: return "CL_INVALID_QUEUE_PROPERTIES";
    case -36: return "CL_INVALID_COMMAND_QUEUE";
    case -37: return "CL_INVALID_HOST_PTR";
    case -38: return "CL_INVALID_MEM_OBJECT";
    case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
    case -40: return "CL_INVALID_IMAGE_SIZE";
    case -41: return "CL_INVALID_SAMPLER";
    case -42: return "CL_INVALID_BINARY";
    case -43: return "CL_INVALID_BUILD_OPTIONS";
    case -44: return "CL_INVALID_PROGRAM";
    case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
    case -46: return "CL_INVALID_KERNEL_NAME";
    case -47: return "CL_INVALID_KERNEL_DEFINITION";
    case -48: return "CL_INVALID_KERNEL";
    case -49: return "CL_INVALID_ARG_INDEX";
    case -50: return "CL_INVALID_ARG_VALUE";
    case -51: return "CL_INVALID_ARG_SIZE";
    case -52: return "CL_INVALID_KERNEL_ARGS";
    case -53: return "CL_INVALID_WORK_DIMENSION";
    case -54: return "CL_INVALID_WORK_GROUP_SIZE";
    case -55: return "CL_INVALID_WORK_ITEM_SIZE";
    case -56: return "CL_INVALID_GLOBAL_OFFSET";
    case -57: return "CL_INVALID_EVENT_WAIT_LIST";
    case -58: return "CL_INVALID_EVENT";
    case -59: return "CL_INVALID_OPERATION";
    case -60: return "CL_INVALID_GL_OBJECT";
    case -61: return "CL_INVALID_BUFFER_SIZE";
    case -62: return "CL_INVALID_MIP_LEVEL";
    case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
    case -64: return "CL_INVALID_PROPERTY";
    case -65: return "CL_INVALID_IMAGE_DESCRIPTOR";
    case -66: return "CL_INVALID_COMPILER_OPTIONS";
    case -67: return "CL_INVALID_LINKER_OPTIONS";
    case -68: return "CL_INVALID_DEVICE_PARTITION_COUNT";

    // extension errors
    case -1000: return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
    case -1001: return "CL_PLATFORM_NOT_FOUND_KHR";
    case -1002: return "CL_INVALID_D3D10_DEVICE_KHR";
    case -1003: return "CL_INVALID_D3D10_RESOURCE_KHR";
    case -1004: return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
    case -1005: return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
    default: return "Unknown OpenCL error";
    }
}

void print_clbuild_errors(cl_program program,cl_device_id device) {
        cout<<"Program Build failed\n";
        size_t length;
        char buffer[2048];
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &length);
        cout<<"--- Build log ---\n "<<buffer<<endl;
        exit(1);
    }

unsigned char ** read_file(const char *name) {
  size_t size;
  unsigned char **output=(unsigned char **)malloc(sizeof(unsigned char *));
  FILE* fp = fopen(name, "rb");
  if (!fp) {
    printf("no such file:%s",name);
    exit(-1);
  }

  fseek(fp, 0, SEEK_END);
  size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  *output = (unsigned char *)malloc(size);
  unsigned char **outputstr=(unsigned char **)malloc(sizeof(unsigned char *));
  *outputstr= (unsigned char *)malloc(size);
  if (!*output) {
    fclose(fp);
    printf("mem allocate failure:clock_t%s",name);
    exit(-1);
  }

  if(!fread(*output, size, 1, fp)) printf("failed to read file\n");
  fclose(fp);
  printf("file size %d\n",size);
  printf("-------------------------------------------\n");
  snprintf((char *)*outputstr,size,"%s\n",*output);
  printf("%s\n",*outputstr);
  printf("-------------------------------------------\n");
  return outputstr;
}

void callback(const char *buffer, size_t length, size_t final, void *user_data) {
    fwrite(buffer, 1, length, stdout);
}

void checkError(int status, const char *msg) {
	if(status!=CL_SUCCESS)	
		printf("%s\n",msg);
}

int main(int, char**)
{
    VideoCapture camera("./bourne.mp4");
    if(!camera.isOpened())  // check if we succeeded
        return -1;

    const string NAME = "./gpu_output.avi";   // Form the new name with container
    int ex = static_cast<int>(CV_FOURCC('M','J','P','G')); // Compression (motion-jpeg)
    Size S = Size((int) camera.get(CV_CAP_PROP_FRAME_WIDTH),    // Acquire input size
                  (int) camera.get(CV_CAP_PROP_FRAME_HEIGHT));
	//Size S =Size(1280,720);
	
    VideoWriter outputVideo;      // Create video output
        outputVideo.open(NAME, ex, 25, S, true);  // (name, compression, fps, size, colored)
 
    if (!outputVideo.isOpened())
    {
        cout  << "Could not open the output video for write: " << NAME << endl;
        return -1;
    }
    struct timespec start, end;
	double diff;
	const char *windowName = "filter";   // Name shown in the GUI window.
    #ifdef SHOW
    namedWindow(windowName); // Resizable window, might not work on Windows.
    #endif

    // OpenCL variables declaration

    char char_buffer[STRING_BUFFER_LEN];
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_context_properties context_properties[] =
    { 
        CL_CONTEXT_PLATFORM, 0,
        CL_PRINTF_CALLBACK_ARM, (cl_context_properties)callback,
        CL_PRINTF_BUFFERSIZE_ARM, 0x1000,
        0
    };
    cl_command_queue queue;
    cl_program program;
    cl_program program2;
    cl_kernel kernelGaussian;
    cl_kernel kernelEdge;

    cl_mem input_buf; // num_devices elements
    cl_mem temp_buf; // num_devices elements
    // cl_mem result_buf; // num_devices elements
       
    cl_event write_event;
    cl_event kernel_event;

    cl_int errcode;
    int status;

    int sizeOfFilter= (S.width + 2)*(S.height + 2);
    int sizeOfImage= (S.width)*(S.height);

    // Creation of the kernels

    clGetPlatformIDs(1, &platform, NULL);
    clGetPlatformInfo(platform, CL_PLATFORM_NAME, STRING_BUFFER_LEN, char_buffer, NULL);
    printf("%-40s = %s\n", "CL_PLATFORM_NAME", char_buffer);
    clGetPlatformInfo(platform, CL_PLATFORM_VENDOR, STRING_BUFFER_LEN, char_buffer, NULL);
    printf("%-40s = %s\n", "CL_PLATFORM_VENDOR ", char_buffer);
    clGetPlatformInfo(platform, CL_PLATFORM_VERSION, STRING_BUFFER_LEN, char_buffer, NULL);
    printf("%-40s = %s\n\n", "CL_PLATFORM_VERSION ", char_buffer);

    context_properties[1] = (cl_context_properties)platform;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    context = clCreateContext(context_properties, 1, &device, NULL, NULL, NULL);

    queue = clCreateCommandQueue(context, device, 0, NULL);

    unsigned char **opencl_program=read_file("videofilter.cl");
    unsigned char **opencl_program2=read_file("edge.cl");
    program = clCreateProgramWithSource(context, 1, (const char **)opencl_program, NULL, NULL);
    if (program == NULL)
    {
      printf("Program creation failed\n");
      return 1; 
    }	
    program2 = clCreateProgramWithSource(context, 1, (const char **)opencl_program2, NULL, NULL);
    if (program2 == NULL)
    {
      printf("Program creation failed\n");
      return 1; 
    }

    int success=clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if(success!=CL_SUCCESS) print_clbuild_errors(program,device);
    cout << "Premier build réussi" << endl;
    success=clBuildProgram(program2, 0, NULL, NULL, NULL, NULL);
    if(success!=CL_SUCCESS) print_clbuild_errors(program,device);

    kernelGaussian = clCreateKernel(program, "gaussianBlur", &errcode);
    kernelEdge = clCreateKernel(program2, "edgeDetection", &errcode);

    int frames = 299;
	cout << "SIZE:" << S << endl;

    double tot = 0;

        // Input buffer
        input_buf = clCreateBuffer(context, CL_MEM_READ_ONLY,
            sizeOfFilter * sizeof(char), NULL, &status);
        checkError(status, "Failed to create buffer for input");
        // Output buffer.
        temp_buf = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
            sizeOfImage * sizeof(char), NULL, &status);
        checkError(status, "Failed to create buffer for temp output");
        // result_buf = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
        //     sizeOfImage * sizeof(char), NULL, &status);
        // checkError(status, "Failed to create buffer for result");


    Mat cameraFrame, edge, grayframe_output, edge_inv;
    Mat grayframe = Mat(S.height, S.width , CV_8U);

    for (int count = 0; count <= frames; count++)
    {
        std::cout << "Frame: "<<count << std::endl;
            /* Todo 
                X Create buffers (I/O)
                X Create events for writing anVideoCaptured computing
                X Add a 0s borders so that theVideoCapture convolution works fine
                X Map writing buffersVideoCapture
                X Unmap writing buffers
                X Send args to the kernels
                X Enqueue kernels, barriers maybe and wait them
                X Enqueue output map buffers
                Ideas
                    -read the frame
                    -one kernel computes one frame
                    -compute
                    -gather and mount

            Convolutions

                Gaussian Blur 3x3:
                        | 1 2 1 |
                    1/16*| 2 4 2 |
                        | 1 2 1 |

                Scharr transform: (Sobel operator)
                        | -3 0 3 |
                    Gx  | -10 0 10 |
                        | -3 0 3 |

                        | -3 -10 -3 |
                    Gy  |  0  0  0 |
                        | -3 -10 -3 |
            */

        char *input = (char *)clEnqueueMapBuffer(queue, input_buf, CL_TRUE,
            CL_MAP_WRITE, 0, sizeOfFilter * sizeof(char), 0, NULL, &write_event, &errcode);
        checkError(errcode, "Failed to map input");
        

        Mat grayframe_input = Mat(S.height + 2, S.width + 2, CV_8UC1);
        Mat displayframe = Mat::zeros(Size(S.width,S.height),CV_8UC3);

        camera >> cameraFrame;  // cameraframe is one frame of the original video
        cvtColor(cameraFrame, grayframe, CV_BGR2GRAY);  // create a grayscale image
        copyMakeBorder(grayframe, grayframe,1,1,1,1,BORDER_REPLICATE);  // create a border of copyMakeBorders of size 1
        
        memcpy(input, grayframe.data, sizeOfFilter*sizeof(char));

        clEnqueueUnmapMemObject(queue,input_buf,input,0,NULL,NULL);

        cvtColor(cameraFrame, edge, CV_BGR2GRAY);  // just instantating edge
        cvtColor(cameraFrame, grayframe_output, CV_BGR2GRAY);  // just instantating grayframe_output

        //////////////////// Run 1; Gaussian Blur n°1

        // Setting arguments
        unsigned argi = 0;
        status = clSetKernelArg(kernelGaussian, argi++, sizeof(cl_mem), &input_buf);
        checkError(status, "Failed to set argumsent 1");
        status = clSetKernelArg(kernelGaussian, argi++, sizeof(cl_mem), &temp_buf);
        checkError(status, "Failed to set argumsent 2");

        // GPU computation
        clock_gettime(CLOCK_MONOTONIC, &start);
        
        // const size_t global_work_size[] = {S.width, S.height};
        const size_t global_work_size[] = {640, 360};

        status = clEnqueueNDRangeKernel(queue, kernelGaussian, 2, NULL,
            global_work_size, NULL, 1, &write_event, &kernel_event);
        checkError(status, "Failed to launch kernelGaussian");
        status=clWaitForEvents(1,&kernel_event);
        checkError(status, "Failed to wait kernels");

        clock_gettime(CLOCK_MONOTONIC, &end);
        diff = (end.tv_sec - start.tv_sec) * 1e9;
        diff = (diff + (end.tv_nsec - start.tv_nsec)) * 1e-9;
        tot += diff;

        char *output = (char *)clEnqueueMapBuffer(queue, temp_buf, CL_TRUE,
            CL_MAP_READ, 0, sizeOfImage * sizeof(char), 0, NULL, NULL, &errcode);
        checkError(errcode, "Failed to map output");

        memcpy(grayframe_output.data, output, sizeOfImage*sizeof(char));

        cout << grayframe_output.rows << " " << grayframe_output.cols << endl;

        clEnqueueUnmapMemObject(queue,temp_buf,output,0,NULL,NULL);


        ////////////// Run 2; Gaussian Blur n°2

        copyMakeBorder(grayframe_output, grayframe,1,1,1,1,BORDER_REPLICATE);        
        
        input = (char *)clEnqueueMapBuffer(queue, input_buf, CL_TRUE,
            CL_MAP_WRITE, 0, sizeOfFilter * sizeof(char), 0, NULL, &write_event, &errcode);
        checkError(errcode, "Failed to map input");

        memcpy(input, grayframe.data, sizeOfFilter*sizeof(char));

        clEnqueueUnmapMemObject(queue,input_buf,input,0,NULL,NULL);

        argi = 0;
        status = clSetKernelArg(kernelGaussian, argi++, sizeof(cl_mem), &input_buf);
        checkError(status, "Failed to set argumsent 1");
        status = clSetKernelArg(kernelGaussian, argi++, sizeof(cl_mem), &temp_buf);
        checkError(status, "Failed to set argumsent 2");

        // GPU computation
        clock_gettime(CLOCK_MONOTONIC, &start);
        
        status = clEnqueueNDRangeKernel(queue, kernelGaussian, 2, NULL,
            global_work_size, NULL, 1, &write_event, &kernel_event);
        checkError(status, "Failed to launch kernelGaussian");
        status=clWaitForEvents(1,&kernel_event);
        checkError(status, "Failed to wait kernels");

        clock_gettime(CLOCK_MONOTONIC, &end);
        diff = (end.tv_sec - start.tv_sec) * 1e9;
        diff = (diff + (end.tv_nsec - start.tv_nsec)) * 1e-9;
        tot += diff;

        output = (char *)clEnqueueMapBuffer(queue, temp_buf, CL_TRUE,
            CL_MAP_READ, 0, sizeOfImage * sizeof(char), 0, NULL, NULL, &errcode);
        checkError(errcode, "Failed to map output");

        memcpy(grayframe_output.data, output, sizeOfImage*sizeof(char));

        clEnqueueUnmapMemObject(queue,temp_buf,output,0,NULL,NULL);
        
        ////////////// Run 3; Gaussian Blur n°3

        input = (char *)clEnqueueMapBuffer(queue, input_buf, CL_TRUE,
            CL_MAP_WRITE, 0, sizeOfFilter * sizeof(char), 0, NULL, &write_event, &errcode);
        checkError(errcode, "Failed to map input");

        copyMakeBorder(grayframe_output, grayframe,1,1,1,1,BORDER_REPLICATE);        
        memcpy(input, grayframe.data, sizeOfFilter*sizeof(char));

        clEnqueueUnmapMemObject(queue,input_buf,input,0,NULL,NULL);

        argi = 0;
        status = clSetKernelArg(kernelGaussian, argi++, sizeof(cl_mem), &input_buf);
        checkError(status, "Failed to set argumsent 1");
        status = clSetKernelArg(kernelGaussian, argi++, sizeof(cl_mem), &temp_buf);
        checkError(status, "Failed to set argumsent 2");

        // GPU computation
        clock_gettime(CLOCK_MONOTONIC, &start);
        
        status = clEnqueueNDRangeKernel(queue, kernelGaussian, 2, NULL,
            global_work_size, NULL, 1, &write_event, &kernel_event);
        checkError(status, "Failed to launch kernelGaussian");
        status=clWaitForEvents(1,&kernel_event);
        checkError(status, "Failed to wait kernels");

        clock_gettime(CLOCK_MONOTONIC, &end);
        diff = (end.tv_sec - start.tv_sec) * 1e9;
        diff = (diff + (end.tv_nsec - start.tv_nsec)) * 1e-9;
        tot += diff;

        output = (char *)clEnqueueMapBuffer(queue, temp_buf, CL_TRUE,
            CL_MAP_READ, 0, sizeOfImage * sizeof(char), 0, NULL, NULL, &errcode);
        checkError(errcode, "Failed to map output");

        memcpy(grayframe_output.data, output, sizeOfImage*sizeof(char));

        clEnqueueUnmapMemObject(queue,temp_buf,output,0,NULL,NULL);

        ////////////// Run 4; Edge Detection

        input = (char *)clEnqueueMapBuffer(queue, input_buf, CL_TRUE,
            CL_MAP_WRITE, 0, sizeOfFilter * sizeof(char), 0, NULL, &write_event, &errcode);
        checkError(errcode, "Failed to map input");

        copyMakeBorder(grayframe_output, grayframe,1,1,1,1,BORDER_REPLICATE);        
        memcpy(input, grayframe.data, sizeOfFilter*sizeof(char));

        clEnqueueUnmapMemObject(queue,input_buf,input,0,NULL,NULL);

        argi = 0;
        status = clSetKernelArg(kernelEdge, argi++, sizeof(cl_mem), &input_buf);
        checkError(status, "Failed to set argumsent 1");
        status = clSetKernelArg(kernelEdge, argi++, sizeof(cl_mem), &temp_buf);
        checkError(status, "Failed to set argumsent 2");

        // GPU computation
        clock_gettime(CLOCK_MONOTONIC, &start);
        
        status = clEnqueueNDRangeKernel(queue, kernelEdge, 2, NULL,
            global_work_size, NULL, 1, &write_event, &kernel_event);
        checkError(status, "Failed to launch kernelGaussian");
        status=clWaitForEvents(1,&kernel_event);
        checkError(status, "Failed to wait kernels");

        clock_gettime(CLOCK_MONOTONIC, &end);
        diff = (end.tv_sec - start.tv_sec) * 1e9;
        diff = (diff + (end.tv_nsec - start.tv_nsec)) * 1e-9;
        tot += diff;

        output = (char *)clEnqueueMapBuffer(queue, temp_buf, CL_TRUE,
            CL_MAP_READ, 0, sizeOfImage * sizeof(char), 0, NULL, NULL, &errcode);
        checkError(errcode, "Failed to map output");

        memcpy(edge.data, output, sizeOfImage*sizeof(char));
        
        clEnqueueUnmapMemObject(queue,temp_buf,output,0,NULL,NULL);

        ////////////// End

    	memset((char*)displayframe.data, 0, displayframe.step * displayframe.rows);
        grayframe_output.copyTo(displayframe,edge);
        // // // edge.copyTo(displayframe);
        // // grayframe_output.copyTo(displayframe);
        cvtColor(displayframe, displayframe, CV_GRAY2BGR);
        outputVideo << displayframe; // writes the next video frame
    
    }

    printf ("GPU took %.8lf seconds to compute.\n", diff );
    printf ("FPS: %.2lf .\n", 299.0/diff );

    // Video creation
	outputVideo.release();
	camera.release();

 

    // Release local events.

    clReleaseEvent(write_event);
    clReleaseKernel(kernelGaussian);
    clReleaseKernel(kernelEdge);
    clReleaseCommandQueue(queue);
    clReleaseMemObject(input_buf);
    clReleaseMemObject(temp_buf);
    // clReleaseMemObject(output_buf_2);
    clReleaseProgram(program);
    clReleaseContext(context);

    clFinish(queue);

    return EXIT_SUCCESS;

}
