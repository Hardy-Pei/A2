#include "wsn.h"

int baseStation(int msgTag, int exitTag, int rank, MPI_Datatype eventInfo) {
	FILE* fp;
	MPI_Status stat;
	MPI_Request req;
	double start, end, startComm, endComm, startPre, endPre, startEn, endEn;
	event eventRecv;
	long pubKeys[2], priKeys[2], infoTem[maxlen], timeTemp[maxlen], encryAdd[maxlen/2];
	char timeInfo[maxlen];
	time_t triggerTime;
	int i, etArray[numOfIterations], msgArray[numOfIterations], nNodes = 20;;
	double pTime = 0, sTime = 0;
	char address[maxlen/2];

	startPre = MPI_Wtime();
	fp=fopen("log.txt","w");
	getKeys(rank, pubKeys, priKeys);
	// printf("%ld, %ld\n", pubKeys[0], pubKeys[1]);
	// printf("%ld, %ld\n", priKeys[0], priKeys[1]);
	MPI_Bcast(pubKeys, 2, MPI_LONG, rank, MPI_COMM_WORLD);
	for (i=0; i<nNodes; i++) {
		MPI_Recv(&encryAdd,maxlen/2,MPI_LONG,i+1,msgTag,MPI_COMM_WORLD,&stat);
		decrypt_p(encryAdd, priKeys[0], priKeys[1], encryAdd, maxlen/2);
		long2char(encryAdd, address, maxlen/2);
		fprintf(fp, "Node %d, IP address: %s\n", i, address);
	}
	endPre = MPI_Wtime();
	fprintf(fp, "\nPreparation finished, %f seconds taken\n\n", endPre - startPre);

	start = MPI_Wtime();
	double matrix[numOfIterations][3];
	memset(matrix, 0, sizeof(matrix[0][0])*3*numOfIterations);
	memset(etArray, 0, sizeof(etArray[0])*numOfIterations);
	memset(msgArray, 0, sizeof(msgArray[0])*numOfIterations);

	while (nNodes > 0) {
		startComm = MPI_Wtime();
		MPI_Irecv(&eventRecv,1,eventInfo,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&req);
		MPI_Wait(&req, &stat);
		endComm = MPI_Wtime();
		if (stat.MPI_TAG==exitTag) {
            nNodes -= 1;
			decrypt_p(eventRecv.timestamp, priKeys[0], priKeys[1], timeTemp, 30+numOfIterations);
			decrypt_p(eventRecv.triggerInfo, priKeys[0], priKeys[1], infoTem, 30+numOfIterations);
			for (i=0; i<numOfIterations; i++) {
				// if (timeTemp[30+i] < 0 || infoTem[30+i] > 100) {
				// 	printf("bad bad %d\n", stat.MPI_SOURCE);
				// }
				etArray[i] += timeTemp[30+i];
				msgArray[i] += infoTem[30+i];
				// if (stat.MPI_SOURCE == 6) {
				// 	printf("%ld\n", timeTemp[30+i]);
				// 	printf("%ld\n", infoTem[30+i]);
				// }
			}
			long2char(timeTemp, timeInfo, timeMsgLen);
			fprintf(fp, "Node %ld terminated at %s. ", infoTem[8], timeInfo);
			fprintf(fp,"Adjacent nodes %ld, %ld, %ld, %ld activated %ld, %ld, %ld, %ld events, %ld events in total, %f seconds taken to exchange the keys, %f seconds taken to communicate with adjacent nodes\n\n", infoTem[0], infoTem[2], infoTem[4], infoTem[6], infoTem[1], infoTem[3], infoTem[5], infoTem[7], infoTem[9], (double)infoTem[10]/scale, (double)infoTem[11]/scale); 
		} else {

			startEn = MPI_Wtime();
			decrypt(eventRecv.triggerInfo, priKeys[0], priKeys[1], infoTem, infoNum);
			decrypt(eventRecv.timestamp, priKeys[0], priKeys[1], timeTemp, timeMsgLen);
			endEn = MPI_Wtime();
			sTime += endEn - startEn;

			startEn = MPI_Wtime();
			decrypt_p(eventRecv.triggerInfo, priKeys[0], priKeys[1], infoTem, infoNum);
			decrypt_p(eventRecv.timestamp, priKeys[0], priKeys[1], timeTemp, timeMsgLen);
			endEn = MPI_Wtime();
			pTime += endEn -startEn;

			long2char(timeTemp, timeInfo, timeMsgLen);
			fprintf(fp, "Event occurred at %s\n", timeInfo);
			fprintf(fp, "Event reported from Node %ld, adjacent nodes %ld, %ld, %ld, %ld, gave number %ld at iteration %ld\n", infoTem[4], infoTem[0], infoTem[1], infoTem[2], infoTem[3], infoTem[5], infoTem[6]+1);   
			fprintf(fp, "This communication between the baseStation and the node took %f seconds\n\n", endComm-startComm);
			matrix[infoTem[6]][0] += infoTem[7];
			matrix[infoTem[6]][1] += infoTem[8];
			matrix[infoTem[6]][2] += endEn - startEn;
		}
	}
	MPI_Wait(&req, &stat);
	// for (i=0; i<numOfIterations; i++) {
	// 	printf("%d\n", etArray[i]);
	// 	printf("%d\n", msgArray[i]);
	// }
	double totalTime = 0;
	double totalTimeNB = 0;
	fprintf(fp, "Iteration\tEvents\tMessages To BS\tBS Encryption Time\n");
	int each;
	for (each = 0; each < numOfIterations; each++) {
		fprintf(fp, "%d\t\t%d\t\t%d\t\t%f\n", each+1, (int)matrix[each][0], (int)matrix[each][1], matrix[each][2]);
		// totalTime += matrix[each][0];
		totalTimeNB += matrix[each][2];
	}
	// printf("%f\n", totalTime);
	// printf("%f\n", totalTimeNB);
	fprintf(fp, "\nIteration\tNodes Encryption Time\tAll Messages\n");
	for (each = 0; each < numOfIterations; each++) {
		totalTime +=etArray[each];
		fprintf(fp, "%d\t\t%f\t\t%d\n", each+1, (double)etArray[each]/scale, msgArray[each]);
	}
	fprintf(fp, "\nEncryption Time in the nodes: %f\n", totalTime/scale);
	fprintf(fp, "Encryption Time in the BaseStation: %f\n", totalTimeNB);
	end = MPI_Wtime();
	fprintf(fp, "\nThe program finished correctly, it has taken %f seconds in total\n", end-start);
	fclose(fp);
	printf("Serial: %f\n", sTime);
	printf("Parallel: %f\n", pTime);
	return 0;
}