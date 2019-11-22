__kernel void edgeDetection( global const char * image, global char * EdgedImage )
{
    int idx = get_global_id(0);
    int idy = get_global_id(1);
    int edge_x, edge_y;
    int width = get_global_size(0) + 2;

    edge_x = -3*image[(idy) * width + idx] + 3*image[(idy) * width + idx+2] 
            - 10*image[(idy+1) * width + idx] + 10*image[(idy+1) * width + idx+2]
            - 3*image[(idy+2) * width + idx] + 3*image[(idy+2) * width + idx+2];
    edge_y = -3*image[(idy) * width + idx] - 10*image[(idy) * width +idx+1] - 3*image[(idy) * width + idx+2] 
            + 3*image[(idy+2) * width + idx] +10*image[(idy+2) * width + idx+1] + 3*image[(idy+2) * width + idx+2];
    int tmp = 0.5*(edge_x + edge_y);

    EdgedImage[(idy)*(width-2)+idx] = (tmp > 80) ? 0 : 255;
}
