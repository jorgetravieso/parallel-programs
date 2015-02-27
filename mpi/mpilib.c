

#include <mpi.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>



#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))
#define LOGGING_MODE 1


void addall(int x, int* sum, int root);
void collectall(char* , int , char* );


int p;       //number of processes
int rank;     //id

int main(int argc, char ** argv){



	int sum = 0;		//w
	


	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD, &p); 		   //Find # of processes
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	addall(rank, &sum, 0);
	if(rank==0) printf("addall->%d\n", sum);
	char ch = 'A'+rank;
	char* buf = (char*)malloc(p+1);
	collectall(&ch, 1, buf);
	buf[p] = '\0';
	if(rank==2) printf("%d: %s\n", rank, buf);
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

		if(root < 0 || root >= p){
			printf("Error invalid root parameter\n");
			exit(-1);
		}

		int n =  ceil(log2(p)) ;
		int temp = 0;

		for(int i = n - 1; i >= 0; i--){								//from the leftmost bit to the right most    
			if(CHECK_BIT(rank,i)){
				int dest = rank ^ 1 << i;								//toggle the ith bit
				MPI_Send(&x, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
				//if(LOGGING_MODE) printf("------- Process %d sent %d to %d -------\n", rank, x, dest);
				break;
			}
			else{
				int src = rank ^ 1 << i;
				if(src >= p){
					printf("Skipping a recv %d\n", src);
					continue;
				}
				temp = 0;
				MPI_Recv(&temp, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, 0);
				if(LOGGING_MODE) printf("------- Process %d received %d\n",rank,temp);
				x += temp;
			}
		}

		if(rank == 0 && root == 0){
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
		
		//MPI_Barrier(MPI_COMM_WORLD);
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

		
		char temp[p + 1];
		for(int i = 0; i < p; i++){
			temp[i] = '*'; 
		}

		temp[rank] = sendbuf[0];
		

		int n  = log2(p);
		for(int i = n - 1; i >= 0; i--){
			int dest = rank ^ 1 << i;
			//printf("sending to %d\n", dest);
			MPI_Send(temp, p, MPI_CHAR, dest, 3, MPI_COMM_WORLD);
			//memset(&temp, 0, 256);
			MPI_Recv(recvbuf, p, MPI_CHAR, dest, 3, MPI_COMM_WORLD, 0);
			//printf("Receiving: %s\n", recvbuf);


			for(int z = 0; z < p; z++){
				if(z != rank && (temp[z] - 'A' < 0 || temp[z] - 'A' > p)){
					temp[z] = recvbuf[z];
					
				}
				recvbuf[z] = temp[z];
			}


		}

		//free(temp);


	}
