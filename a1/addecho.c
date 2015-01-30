#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]){
    int delay; 
    int volume;
    FILE *fp;
    FILE *outfp;
    if (argc == 3){ //If no optional arguments.
    	fp = fopen(argv[1], "rb"); /*Open the input file for reading.*/
        outfp = fopen(argv[2], "wb"); /*Open the output file for writing.*/
	printf("number of argument: %d", argc);
    }else if(argc == 5){  //if one optional argument is given.
    	fp = fopen(argv[3], "rb"); /*Open the input file for reading.*/
	outfp = fopen(argv[4], "wb"); /*Open the output file for writing.*/
	printf("number of argument: %d", argc);
    }else{  //If both optional argumenyt are given.
    	fp = fopen(argv[5], "rb"); /*Open the input file for reading.*/
	outfp = fopen(argv[6], "wb"); /*Open the output file for writing.*/
	printf("number of argument: %d", argc);
    }
    
    int ch;
    delay = 8000;  //default value.
    volume = 4; //default vaue.
    while ((ch = getopt(argc, argv, "d:v:")) != -1){
    	switch(ch){
    	    case 'd':
    	        delay = strtol(optarg, NULL, 10);
    	        break;
    	    case 'v':
    	        volume = strtol(optarg, NULL, 10);
    	        break;  
    	    default:
    	        abort();
    	        break; 
    	}            
    }
    
    //printf("delay is %d, volume is %d", delay, volume);
    
    //Copy the header.
    #define HEADER_SIZE 22
    short header[HEADER_SIZE];
    int i;
    unsigned int *sizeptr;
    unsigned int *sizeptr1;
    for (i=0; i<HEADER_SIZE; i++){
        fread(&(header[i]), sizeof(short), 1, fp);
    }
    sizeptr = (unsigned int *)(header + 2);
    sizeptr1 = (unsigned int *)(header + 20);  
    *sizeptr += (delay * 2);
    *sizeptr1 += (delay * 2);  
    int j;	
    for (j=0; j<HEADER_SIZE; j++){
        fwrite(&(header[j]), sizeof(short), 1, outfp);
    }
    //printf("sizeptr, and sizeptr1 is %d, %d", *sizeptr, *sizeptr1);
    
    //Create and allocate memory to an echo buffer.
    short *echoBuff;
    echoBuff = (short *) malloc(delay * sizeof(short));
    //If not yet at sample delay, just write the same sample from orig to your output sound.
    int k, g;
    g = 0;
    for (k=0; k<delay; k++){
        short sample;
        fread(&sample, sizeof(short), 1, fp);
    	if (feof(fp)){  //If delay is bigger than the size of input, start writing 0.
    		short extra;
    		extra = 0;
            fwrite(&extra, sizeof(extra), 1, outfp);
            g += 1;  //g is delay - size of input

        }else{
            fwrite(&sample, sizeof(short), 1, outfp);
            echoBuff[k] = sample / volume;
        }    
    }
    int ii, a;
    short c = 0;
    short *current; //point to the current sample that being read.
    current = &c;
    short temp;  //store the mixing-in value.
    if (feof(fp)){  //case1: for delay>size of input.
    	for(a=0;a<delay-g;a++){  
		fwrite((echoBuff + a), sizeof(short), 1, outfp);
		}		
    }else{  //case2:delay<size of input, and at sample delay
    	    //or later, start mixing-in the samples from the echo buffer.
	    while(1){ //keep updating the acho buffer of size delay
	        for(ii=0;ii<delay;ii++){  
	            fread(current, sizeof(short), 1, fp);
	            temp = *(current) + *(echoBuff + ii);
	            if(feof(fp)){ //if the input is done, break the loop.
	                break;
	            }  
	            fwrite(&temp, sizeof(short), 1, outfp);
	            *(echoBuff + ii) = *current / volume;  //Update the element of the echo buffer to the current / 2.
	        } 
	        if (feof(fp)){
	            for (a=ii;a<delay;a++){  //Write the echo buffer from the point we stopped.
	                fwrite((echoBuff + a), sizeof(short), 1, outfp);
	            }    
	            for (a=0;a<ii;a++){
	                fwrite((echoBuff + a), sizeof(short), 1, outfp);    
	            }
	            break;
	        }   
	 
	    }
    }
    fclose(fp);
    fclose(outfp);
}
	
