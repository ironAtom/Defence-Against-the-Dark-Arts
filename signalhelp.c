/* 14-735 Secure Coding Assignment #1
 * 
 * Author:  Qinyu Tong
 *          qtong@andrew.cmu.edu
 *
 * Date:Fri May  1 22:37:40 EDT 2015
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
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#define SIG_ID_SIZE 6
#define SIG_DSP_SIZE 1000
#define PATH_LENGTH 100
#define BUFF_SIZE 100

struct entry {
    int sig_num;
    char sig_ID[SIG_ID_SIZE + 1]; // add null-terminate
    char sig_dsp[SIG_DSP_SIZE];
};

int read_signum();
int read_sigdb(const char* pathname, struct entry** sig_table);
void query_sigdb(int sig_count, struct entry* sig_table);

int main(int argc, const char* argv[]){
    struct entry *sig_table;
    int sig_count = 0;
    char *pathvar;
    pathvar = getenv("DATA_PATH");

	char buffer[PATH_LENGTH];
	int i = 0;

    if (argc != 2){
        printf("Usage: %s database_file\n",argv[0]);
        return 0;
    }

	//Validation file name, eliminate path vulnerability
	//detect illegale filename, which contains '/'
	while (argv[1][i] != '\0' ) { 
    	if (argv[1][i] == '/'){
			printf("illegal filename! only filename is allowed, no path/directory allowed\n");
			exit(0); //handle erro
		}			
		i++; 
	}

    if (pathvar == NULL)
        sig_count = read_sigdb(argv[1],&sig_table);
    else{
		if (strlen(argv[1]) > PATH_LENGTH - strlen(pathvar) - 1){
			printf("path length exceed PATH_LENGTH\n");
			exit(0); //handle erro
		}
		strcat(buffer,pathvar);
		strcat(buffer,"/");
		strcat(buffer,argv[1]);
        sig_count = read_sigdb(buffer,&sig_table);
	}

    if (sig_table != NULL){
        //drop privilege
        if (setresuid(getuid(), getuid(), getuid()) < 0) {
            printf("drop privilege error\n");
            exit(0); //handle error
        }
		
		if (setuid(0) != -1) {
    		/* Privileges can be restored, handle error */
			printf("privileges can be restored\n");
			exit(0);
  		}
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
        exit(0); //handle erro
    }

    if (fscanf(fp,"%d",&sig_count) != 1){
		printf("bad database file\n");		
		exit(0);
	}
    
	size_t max_size = (size_t)-1;
    if (sig_count > max_size/sizeof(struct entry)){
        printf("memory to allocate exceed the size of size_t, wrap around will happen, exit");
        exit(0); //handle erro
    }

    *sig_table = (struct entry *)malloc(sig_count*sizeof(struct entry));
    if (*sig_table == NULL){
        printf( "Insufficient memory available\n" );
    }
    else{

        for( i = 0; i < sig_count && !feof(fp); i++){
             if (fscanf(fp,"%d%s%[^\n]",&((*sig_table)[i].sig_num),(*sig_table)[i].sig_ID,(*sig_table)[i].sig_dsp) != 3){
				printf("bad db file\n");
				exit(0);
			 } 
        }

        sig_count = i < sig_count ? i : sig_count; // if file ends in advance
    }

    fclose(fp); //close file descriptor

    return sig_count;
}

void build_sig_table(FILE* fp, int* sig_num, char* sig_ID, char *sig_dsp){
	
}

/*
 * return 0 to indicate ignore the value, -1 to indicate 'quit'
 */

int read_signum(){
    char buff[BUFF_SIZE];
    char *end_ptr;
    long num_long;

    if (fgets(buff, sizeof(buff), stdin) == NULL) {
        if (puts("EOF or read error\n") == EOF) {
            exit(0); //handle erro
        }
    }
	else {
        errno = 0;
        num_long = strtol(buff, &end_ptr, 10);
        
        if (ERANGE == errno) {
            if (puts("number out of range") == EOF) {
                exit(0); //handle erro
            }
            return 0;
        }
        else if (end_ptr == buff) {
            
            if (buff[0] == 'q') {
                if (puts("quit") == EOF) {
                    exit(0); //handle erro
                }
                return -1;
            }
            if (puts("not valid numeric input") == EOF) {
                exit(0); //handle erro
            }
            return 0;
        }
        else if ('\n' != *end_ptr && '\0' != *end_ptr) {
            if (puts("extra characters on input line") == EOF) {
                exit(0); //handle erro
            }
            return 0;
        }
        
        if (num_long <= INT_MAX){
            if(num_long <= 0){
                printf("sig number should be positive number\n");
                return 0;
            }
            return (int)num_long;
        }
        else{
            if (puts("out of int type range") == EOF) {
                exit(0); //handle erro
            }
            return 0;
        }
    }
    return 0;
}

void query_sigdb(int sig_count, struct entry* sig_table){
    int i = 0;
    int sig_num;
    char ch = 'a';
    printf("Input the number of the signal for help information, 'q' to quit\n");
    
    
    
        while((sig_num = read_signum()) != -1){
            if (sig_num == 0){
				continue;
			}
                
            for(i = 0; i < sig_count; i++){
                if (sig_table[i].sig_num == sig_num){
                    printf("%d %s%s\n",sig_table[i].sig_num,sig_table[i].sig_ID,sig_table[i].sig_dsp);
                    break;
                }
            }

            if (i == sig_count)
                printf("sig number %d does not exist.\n",sig_num);
        }
}
