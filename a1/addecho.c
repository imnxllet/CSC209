#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]){
    int delay; 
    int volume;
    FILE *fp;
    FILE *outfp;
    if (argc == 3){
    	fp = fopen(argv[1], "rb"); /*Open the input file for reading.*/
	outfp = fopen(argv[2], "wb"); /*Open the output file for writing.*/
	printf("number of argument: %d", argc);
    }else if(argc == 5){
    	fp = fopen(argv[3], "rb"); /*Open the input file for reading.*/
	outfp = fopen(argv[4], "wb"); /*Open the output file for writing.*/
	printf("number of argument: %d", argc);
    }else{
    	fp = fopen(argv[5], "rb"); /*Open the input file for reading.*/
	outfp = fopen(argv[6], "wb"); /*Open the output file for writing.*/
	printf("number of argument: %d", argc);
    }
    
    int ch;
    delay = 8000;
    volume = 4;
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
    
    printf("delay is %d, volume is %d", delay, volume);

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
    printf("sizeptr, and sizeptr1 is %d, %d", *sizeptr, *sizeptr1);

    short *echoBuff;
    echoBuff = (short *) malloc(delay * sizeof(short));
    //not yet at sample delay, just write the same sample from orig to your output sound.
    int k;
    for (k=0; k<delay; k++){
        short sample;
        fread(&sample, sizeof(short), 1, fp);
        if (feof(fp)){
            short extra = 0;
            fwrite(&extra, sizeof(short), 1, outfp);
            echoBuff[k] = sample / volume;
        }else{
            fwrite(&sample, sizeof(short), 1, outfp);
            echoBuff[k] = sample / volume;
        }    
    }
    printf("echobuff[1] is %d", *(echoBuff+1));

    //at sample delay or later, start mixing-in+ the samples from the echo buffer.
    int ii, a;
    short c = 0;
    short *current;
    current = &c;
    short temp;
    while(1){ 
 
        for(ii=0;ii<delay;ii++){  
            fread(current, sizeof(short), 1, fp);
            temp = *(current) + *(echoBuff + ii);
            if(feof(fp)){
                break;
            }  
            fwrite(&temp, sizeof(short), 1, outfp);
            *(echoBuff + ii) = *current / volume;
        } 
        if (feof(fp)){
            for (a=ii;a<delay;a++){
            	//printf("%d", *(echoBuff + a));
                fwrite((echoBuff + a), sizeof(short), 1, outfp);
            }    
            for (a=0;a<ii;a++){
            	//printf("%d", *(echoBuff + a));
                fwrite((echoBuff + a), sizeof(short), 1, outfp);    
            }
            break;
        }   
 
    }
    fclose(fp);
    fclose(outfp);
}
	