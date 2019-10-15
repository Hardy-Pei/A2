//////////////////////////////////////////////////////////////////////////////////////
// Student Name: Zhihao Pei
// Student ID: 28294335
// Student Email: zpei0001@student.monash.edu
//////////////////////////////////////////////////////////////////////////////////////
#include "wsn.h"

int main(int argc, char* argv[])
{
	int rank;
	int interNodeTag = 0, eventTag = 1, msgTag = 2, exitTag = 3;
	// int triggerNodes[eventInfo];
	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	// MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm cartComm;
	MPI_Comm_split(MPI_COMM_WORLD, rank==0, rank, &cartComm);

	MPI_Datatype eventInfo;
	MPI_Aint disps[2] = {offsetof(event, timestamp), offsetof(event, triggerInfo)};
	int blocklens[2] = {maxlen, maxlen};
	MPI_Datatype types[2] = {MPI_LONG, MPI_LONG};
	MPI_Type_struct(2, blocklens, disps, types, &eventInfo);
	MPI_Type_commit(&eventInfo);

	if (rank == 0) {
		baseStation(msgTag, exitTag, rank, eventInfo);
	} else {
		nodes(cartComm, interNodeTag, eventTag, msgTag, exitTag, rank, eventInfo);
	}
	MPI_Finalize();
	return 0;
}