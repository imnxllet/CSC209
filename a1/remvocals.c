#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
	FILE *fp = fopen(argv[1], "r"); /*Open the input file for reading.*/
	FILE *outfp = fopen(argv[2], "w"); /*Open the output file for writing.*/
    char head[44];
    int i=0;
    for (i=0; i<44; i++){
        fread(&(head[i]), sizeof(head[i]), 1, fp);
        fwrite(&(head[i]), sizeof(head[i]), 1, outfp);
    }  
    /*int nread = fread(&head, sizeof(head), 1, fp);
    int nwrite = fwrite(&head, sizeof(head), 1, outfp);*/
    while(1){
        short left;
        short right;
    	fread(&left, sizeof(left), 1, fp);
    	fread(&right, sizeof(right), 1, fp);
    	short combined = (left - right) / 2;
        if(feof(fp)){
            break;
        } 
    	fwrite(&combined, sizeof(combined), 1, outfp);
    	fwrite(&combined, sizeof(combined), 1, outfp); 

    }
    fclose(fp);
    fclose(outfp);
}    
    
