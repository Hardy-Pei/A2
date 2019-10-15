#include "wsn.h"

int nodes(MPI_Comm cartComm, int interNodeTag, int eventTag, int msgTag, int exitTag, int rank, MPI_Datatype eventInfo) {
	int cartRank;
	int dim[2], periods[2], reorder;
	int potentialAdjNodes[MAXADJ], adjNum=0;
	static int *adjNodes = NULL;
	int totalEventNum = 0;
	int nreq=0, tem, each, i, j, occurrence;
	double startIteration, endIteration, start, end, startComm, endComm, allComm, startKey, endKey, startEn, endEn;
	int encryptTime, msgNum, msgArray[numOfIterations], etArray[numOfIterations];
	event eventSend;
	long triggerNodes[infoNum];
	long pubKeys[2], priKeys[2], baseKeys[2];
	time_t triggerTime;
	char timeString[timeMsgLen];
	char address[maxlen] = {0};
	long encryAdd[maxlen/2];

	getAddress(address);
	getKeys(rank, pubKeys, priKeys);
	MPI_Bcast(baseKeys, 2, MPI_LONG, rank, MPI_COMM_WORLD);
	dim[0] = ROW; dim[1] = COLUMN;
	periods[0] = 0; periods[1] = 0;
	reorder = 1;
	char2long(address, encryAdd, maxlen/2);
	encrypt_p(encryAdd, baseKeys[0], baseKeys[1], encryAdd, maxlen/2);
	MPI_Send(&encryAdd, maxlen/2, MPI_LONG, 0, msgTag, MPI_COMM_WORLD);
	
	MPI_Cart_create(cartComm, 2, dim, periods, reorder, &cartComm);
	MPI_Comm_rank(cartComm, &cartRank);
	MPI_Cart_shift(cartComm, 1, 1, &potentialAdjNodes[0], &potentialAdjNodes[1]);
	MPI_Cart_shift(cartComm, 0, 1, &potentialAdjNodes[2], &potentialAdjNodes[3]);
	for (i=0; i<MAXADJ; i++) {
		if (potentialAdjNodes[i] >= 0) {
			adjNum ++;
		}
	};
	adjNodes = (int*) malloc(adjNum*sizeof(int));
	int theIndex = 0;
	for (i=0; i<MAXADJ; i++) {
		if (potentialAdjNodes[i] >= 0) {
			adjNodes[theIndex] = potentialAdjNodes[i];
			theIndex ++;
		}
	};

	long rRand[adjNum], eRand[adjNum], rNum, eNum;
	long adjKeys[adjNum*2];
	MPI_Request r[adjNum*2], req;
	MPI_Status statuses[adjNum*2], stat;
	startKey = MPI_Wtime();
	for (i=0; i<adjNum; i++) {
		MPI_Isend(pubKeys, 2, MPI_LONG, adjNodes[i], interNodeTag, cartComm, &r[nreq++]);
		MPI_Irecv(adjKeys+(2*i), 2, MPI_LONG, adjNodes[i], interNodeTag, cartComm, &r[nreq++]);
	}
	MPI_Waitall(nreq, r, statuses);
	endKey = MPI_Wtime();
	allComm += endKey - startKey;

	int result[rSize][adjNum];
	int window[windowSize][adjNum];
	memset(result, 0, sizeof(result[0][0])*rSize*adjNum);
	memset(window, -1, sizeof(window[0][0])*windowSize*adjNum);
	int numOccurrence[adjNum], triggerNum[adjNum];
	memset(triggerNum, 0, sizeof(triggerNum[0])*adjNum);
	srand(time(NULL)+cartRank);

	for (each=0; each<numOfIterations; each++) {
		startIteration = MPI_Wtime();
		encryptTime = 0;
		msgNum = 0;
		memset(triggerNodes, -1, sizeof(triggerNodes[0])*infoNum);	
		memset(numOccurrence, -1, sizeof(numOccurrence[0])*adjNum);
		rNum = rand()%rSize;
		eNum = 0;
		nreq = 0;
		startComm = MPI_Wtime();
		for (i=0; i<adjNum; i++) {
			startEn = MPI_Wtime();
			encrypt_p(&rNum, adjKeys[i*2], adjKeys[i*2+1], &eNum, 1);
			endEn = MPI_Wtime();
			encryptTime = encryptTime + (endEn - startEn)*scale;
			MPI_Isend(&eNum, 1, MPI_LONG, adjNodes[i], interNodeTag, cartComm, &r[nreq++]);
			MPI_Irecv(&eRand[i], 1, MPI_LONG, adjNodes[i], interNodeTag, cartComm, &r[nreq++]);
			msgNum += 1;
		}
		// if (cartRank == 5 && each == 1) {
		// 	printf("123 %d\n", encryptTime);
		// }

		MPI_Waitall(nreq, r, statuses);
		startEn = MPI_Wtime();
		decrypt_p(eRand, priKeys[0], priKeys[1], rRand, adjNum);
		endEn = MPI_Wtime();
		encryptTime += encryptTime + (endEn - startEn)*scale;
		endComm = MPI_Wtime();
		allComm += endComm - startComm;
		if (each >= windowSize) {
			for (i=0; i<adjNum; i++) {
				tem = window[each%windowSize][i];
				result[tem][i] -= 1;
			}
		}
		for (i=0; i<adjNum; i++) {
			tem = rRand[i];
			window[each%windowSize][i] = tem;
			result[tem][i] += 1;
			for (j=0; j<adjNum; j++) {
				if (numOccurrence[j] == tem) {
					break;
				} else if (numOccurrence[j] == -1) {
					numOccurrence[j] = tem;
					break;
				}
			}
		}
		if (totalEventNum > 0) {
			MPI_Wait(&req, &stat);
		}
		for (i=0; i<adjNum; i++) {
			tem = numOccurrence[i];
			if (tem != -1) {
				occurrence = 0;
				for (j=0; j<adjNum; j++) {
					if (result[tem][j] > 0) {
						occurrence ++;
					}
				}
				if (occurrence >= 3) {
					triggerTime = time(NULL);
					sprintf(timeString, "%s", asctime(localtime(&triggerTime)));
					timeString[timeMsgLen-1] = 0;
					char2long(timeString, eventSend.timestamp, timeMsgLen);
					encrypt_p(eventSend.timestamp, baseKeys[0], baseKeys[1], eventSend.timestamp, timeMsgLen);
					memset(triggerNodes, -1, sizeof(triggerNodes[0])*infoNum);
					for (j=0; j<adjNum; j++) {
						if (result[tem][j] > 0) {
							triggerNodes[j] = adjNodes[j]+1;
							triggerNum[j] += 1;
						}
					}
					triggerNodes[MAXADJ] = rank;
					triggerNodes[MAXADJ+1] = tem;
					triggerNodes[MAXADJ+2] = each;
					triggerNodes[MAXADJ+3] = 1;
					triggerNodes[MAXADJ+4] = 1;
					startEn = MPI_Wtime();
					encrypt_p(triggerNodes, baseKeys[0], baseKeys[1], eventSend.triggerInfo, infoNum);
					endEn = MPI_Wtime();
					encryptTime += encryptTime + (endEn - startEn)*scale;

					MPI_Isend(&eventSend, 1, eventInfo, 0, eventTag, MPI_COMM_WORLD, &req);
					msgNum += 1;
					totalEventNum += 1;
				}
			} else {
				break;
			}
		}
		etArray[each] = encryptTime;
		msgArray[each] = msgNum;
		// if (cartRank == 5) {
		// 	printf("%d\n", etArray[each]);
		// 	printf("%d\n", msgArray[each] = msgNum);
		// }

		endIteration = MPI_Wtime();
		sleep(iterationTime+startIteration-endIteration);
		// printf("%f\n", iterationTime+startIteration-endIteration);
	}
	long summary[maxlen];
	triggerTime = time(NULL);
	sprintf(timeString, "%s", asctime(localtime(&triggerTime)));
	timeString[timeMsgLen-1] = 0;
	char2long(timeString, eventSend.timestamp, timeMsgLen);
	for (i=0; i<numOfIterations; i++) {
		eventSend.timestamp[30+i] = etArray[i];
		summary[30+i] = msgArray[i];
	}
	encrypt_p(eventSend.timestamp, baseKeys[0], baseKeys[1], eventSend.timestamp, 30+numOfIterations);
	memset(summary, -1, 12);
	for (j=0; j<MAXADJ; j++) {
		if (j < adjNum) {
			summary[2*j] = adjNodes[j]+1;
			summary[2*j+1] = triggerNum[j];
		} else {
			summary[2*j] = -1;
			summary[2*j+1] = -1;
		}
	}
	summary[MAXADJ*2] = rank;
	summary[MAXADJ*2+1] = totalEventNum;
	summary[MAXADJ*2+2] = (endKey - startKey)*scale;
	summary[MAXADJ*2+3] = allComm*scale;
	encrypt_p(summary, baseKeys[0], baseKeys[1], eventSend.triggerInfo, 30+numOfIterations);
	MPI_Isend(&eventSend, 1, eventInfo, 0, exitTag, MPI_COMM_WORLD, &req);
	free(adjNodes);

	return 0;
}