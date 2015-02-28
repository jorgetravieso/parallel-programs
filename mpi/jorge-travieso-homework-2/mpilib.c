
/*
Name: Jorge Travieso
Class: COP 4520
Professor Liu
Spring 2015
*/



#include <mpi.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

//Macros
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))
#define IS_POWER_OF_TWO(x) ((x & (x - 1)) == 0)


//Functions Prototypes
void addall(int x, int* sum, int root);
void collectall(char* sendbuf, int sendcnt, char* recvbuf);


int p;      			 						//number of processes

int main(int argc, char ** argv){

	int id;     								//id of process
	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD, &p); 			//Find # of processes
	MPI_Comm_rank(MPI_COMM_WORLD, &id);		//Find current process rank

	int sum;
	addall(id, &sum, 0);
	if(id==0) printf("addall->%d\n", sum);
	char ch = 'A'+id;
	char* buf = (char*)malloc(p+1);
	collectall(&ch, 1, buf);
	buf[p] = '\0';
	if(id==2) printf("%d: %s\n", id, buf);
	free(buf);
	
	MPI_Finalize();
	return 0;
}


/* 
	This function is a collective operation; that is, it shall be called by all mpi processes or none at all. 
	Each process is expected to provide a value 'x', and when the function returns, the process whose rank is 
	'root' shall have the sum stored in the location pointed by 'sum'. 
*/
	void addall(int x, int* sum, int root){

		MPI_Barrier(MPI_COMM_WORLD);

																		//input validation
		if(root < 0 || root >= p){
			printf("Error invalid root parameter\n");
			exit(-1);
		}

		int rank;
		MPI_Comm_rank(MPI_COMM_WORLD, &rank);	   						//Find current process rank
		int n =  ceil(log2(p)) ;
		int temp = 0;

		

		int i;
		for( i = n - 1; i >= 0; i--){								//from the leftmost bit to the right most  
																		//look at my rank  
			if(CHECK_BIT(rank,i)){										//if the ith bit is set
				int dest = rank ^ 1 << i;								//process will send to rank 'dest' by toggling the ith bit
				MPI_Send(&x, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
				break;													//Done!
			}
			else{
				int src = rank ^ 1 << i;								//we are going to receive
				if(src >= p){											//avoid receiving from inexisting processes
					continue;
				}
				temp = 0;
				MPI_Recv(&temp, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, 0);
				x += temp;
			}
		}

		if(rank == 0 && root == 0){										//send to the specfified root
			*sum = x;
		}
		else if(root == rank){
			temp = 0;
			MPI_Recv(&temp, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, 0);
			*sum = temp;
		}
		else if(rank == 0){ 
			MPI_Send(&x, 1, MPI_INT, root, 1, MPI_COMM_WORLD);
		}
	}


/* 
	This function is a collective operation; that is, it shall be called by all mpi processes or none at all. 
	Each process is expected to provide a block of data of size 'sendcnt' stored in 'sendbuf'. When the function
	returns, each process will have the data blocks sent from all other processes (including itself) stored in 
	'recvbuf'. The receive buffer shall be allocated before hand with enough room to store data from all processes. 
	The block of data from the i-th process is received by every process and placed in the i-th block of the 
	receive buffer  'recvbuf'. You can assume that all processes provide the same 'sendcnt'. 
*/


	void collectall(char* sendbuf, int sendcnt, char* recvbuf){

		MPI_Barrier(MPI_COMM_WORLD);

		if (!IS_POWER_OF_TWO(p)){
			printf("Error invalid number of processes\n");
			exit(-1);
		}

		int rank;
		MPI_Comm_rank(MPI_COMM_WORLD, &rank);	   							//Find current process rank
		
		int size = sendcnt * p;												
		char temp[size];													//temporal buffer to send data
		memset(&temp, 0, size);												//clear the buffer
		int i, j = rank * sendcnt;											//get the initial values fo the the sendbuf	
		for(i = 0; i < sendcnt; i++){								
			temp[j] = sendbuf[i];
			recvbuf[j++] = sendbuf[i];
		}

		int n  = log2(p);													//get the number of bits
		for(i = n - 1; i >= 0; i--){										//for each bit
			int dest = rank ^ 1 << i;										//flip the ith bit to get a destination
			MPI_Send(temp, size, MPI_CHAR, dest, 4, MPI_COMM_WORLD);		//send the current temp buffer to the destination
			MPI_Recv(recvbuf, size, MPI_CHAR, dest, 4, MPI_COMM_WORLD, 0);	//receive the other's buffer

			int j;															//merge the buffers into temp
			for(j = 0; j < size; j++){
				if(recvbuf[j] != 0){
					temp[j] = recvbuf[j];			
				}
			}
		}															
		memcpy(recvbuf, temp, size);										//copy the temp buffer back to the recvbuf

	}
