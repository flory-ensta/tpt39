__kernel void gaussianBlur( global const char * image, global char * FilteredImage )
{
    int idx = get_global_id(0);
    int idy = get_global_id(1);
    int width = get_global_size(0);
    float tmp;

    tmp = 0.0625*image[(idy)*(width+2)  + idx] + 0.125*image[(idy)*(width+2) + idx+1] + 0.0625*image[(idy)*(width+2) + idx+2] 
            + 0.125*image[(idy+1) * (width+2) + idx] + 0.25*image[(idy+1) * (width+2) + idx+1] + 0.125*image[(idy+1) * (width+2) + idx+2]
            + 0.0625*image[(idy+2) * (width+2) + idx] + 0.125*image[(idy+2) * (width+2) + idx+1] + 0.0625*image[(idy+2) * (width+2) + idx+2];
    FilteredImage[(idy)*width+idx] = tmp;
}

