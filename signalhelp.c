/* 14-735 Secure Coding Assignment #1
 * 
 * Author:  Qinyu Tong
 *          qtong@andrew.cmu.edu
 *
 * Date:Tue Jan 20 15:48:47 EST 2015 
 *
 * This program provides a signal handler service for Linux system. In this
 * assignment, the help feature which outputs a description of a specified 
 * signal was implemented.
 *
 * The program accepts a single command line argument:
 * Usage: %s database_file
 * The database_file argument specifies the name of the file containing 
 * the signal database.
 *
 * The program also accepts an environmental variable DATA_PATH. If DATA_PATH 
 * is set, the program reads the specified input file from the DATA_PATH 
 * directory. If not, the program reads the input file from the current 
 * working directory.
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define SIG_ID_SIZE 6
#define SIG_DSP_SIZE 100

struct entry {
    int sig_num;
    char sig_ID[SIG_ID_SIZE + 1]; // add null-terminate
    char sig_dsp[SIG_DSP_SIZE];
};

int read_sigdb(const char* pathname, struct entry** sig_table);
void query_sigdb(int sig_count, struct entry* sig_table);

int main(int argc, const char* argv[]){
    struct entry *sig_table;
    int sig_count = 0;
    char *pathvar;
    pathvar = getenv("DATA_PATH");

    if (argc != 2){
        printf("Usage: %s database_file\n",argv[0]);
        return 0;
    }

    if (pathvar == NULL)
        sig_count = read_sigdb(argv[1],&sig_table);
    else
        sig_count = read_sigdb(pathvar,&sig_table);

    if (sig_table != NULL){
        query_sigdb(sig_count, sig_table);
        free(sig_table);
    }

    return 0;
}


int read_sigdb(const char* pathname, struct entry** sig_table){
    FILE *fp = NULL;
    const char *mode = "r";
    int sig_count = 0;
    int i = 0;

    fp = fopen(pathname, mode);
    if (fp == NULL){
        fprintf(stderr, "Can't open input file %s!\n",pathname);
        exit(0);
}

    fscanf(fp,"%d",&sig_count);

    *sig_table = (struct entry *)malloc(sig_count*sizeof(struct entry));
    if (*sig_table == NULL){
        printf( "Insufficient memory available\n" );
    }
    else{

        for( i = 0; i < sig_count && !feof(fp); i++){
            fscanf(fp,"%d%s%[^\n]",&((*sig_table)[i].sig_num),(*sig_table)[i].sig_ID,(*sig_table)[i].sig_dsp);
        }

        sig_count = i < sig_count ? i : sig_count; // if file ends in advance
    }

    fclose(fp); //close file descriptor

    return sig_count;
}

void query_sigdb(int sig_count, struct entry* sig_table){
    int i = 0;
    int sig_num;
    char ch = 'a';
    printf("Input the number of the signal for help information, 'q' to quit\n");
    while (ch != 'q'){
        while(scanf("%d",&sig_num)){
            for(i = 0; i < sig_count; i++){
                if (sig_table[i].sig_num == sig_num){
                    printf("%d %s%s\n",sig_table[i].sig_num,sig_table[i].sig_ID,sig_table[i].sig_dsp);
                    break;
                }
            }

            if (i == sig_count)
                printf("sig number %d does not exist.\n",sig_num);
        }
        scanf("%c",&ch);
    }
}
