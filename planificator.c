#include "quelib.h"
/*Algorithm names for every CPU scheduling type*/
typedef enum ALGTYPE {
	FCFS,//First Come First Serve
	SJF, //Shortest Job First
	RR,	 //Round Robin
	PP	 //Priority Planning
} Algtype;

/*Compare function for the SJF priority queue*/
int compSJF (Value a, Value b) {
	return (a.BT - b.BT);
}

/*Compare function for the PP*/
int compPP (Value a, Value b) {
	return (b.P - a.P);
}

void readyToRunning (Value *running, Queue *ready) {
	(*running) =  pop (ready);
	printf("Running: %s %d\n", running->name, running->BT);
	running->isInitialized = 1;
}

void waitingToReady (List *waiting, Queue *ready, Algtype type, char pname[]) {
	Value notWaiting;
    int found = 0;
    printf("%s\n", pname);
    Node *aux = waiting->head;
    Node *tmp = NULL;
    Node *prev = NULL;
    
    while (aux != NULL) {
    	if (strcmp(aux->value.name, pname) == 0) {
	    	printf("LLLLLLLLLLLLLLLLLLLLL\n");
    		found = 1;
    		if (prev == NULL) {
    			waiting->head = aux->next;
    			notWaiting = aux->value;
    			tmp = aux;
    			free(tmp);
    		} else {
       			notWaiting = aux->value;
    			prev->next = aux->next;
    			tmp = aux;
    			free(tmp);
    		}
    		break;
    	} else {
    		prev = aux;
    		aux = aux->next;
    	}
    }

    if (found == 0) {
        return;
    }
    if (type == FCFS || type == RR) {
        push (ready, notWaiting);
    }
    else if (type == SJF) {
        priorityPush (ready, notWaiting, compSJF);
    }
    else if (type == PP)  {
        priorityPush (ready, notWaiting, compPP);
    }
}

void runningToReady (Value *running, Queue *ready, Algtype type) {
	if (type == FCFS || type == RR) {
		push(ready, *running);
	} else if (type == SJF) {
		priorityPush (ready, *running, compSJF);
	} else if (type == PP) {
		priorityPush (ready, *running, compPP);
	}

	running->isInitialized = 0;
}

void runningToWaiting (Value *running, List *waiting) {
	insertNode (waiting, *running);
	running->isInitialized = 0;
}

void runningToTerminated (Value *running, Queue *terminated) {
	push (terminated, *running);
	running->isInitialized = 0;
}

void add (Value Value, Queue *ready, Algtype type) {
	if (type == FCFS || type == RR) {
		push(ready, Value);
	} else if (type == SJF) {
		priorityPush (ready, Value, compSJF);
	} else if (type == PP) {
		priorityPush (ready, Value, compPP);
	}
}

void wait (Value *running, List * waiting) {
	runningToWaiting (running, waiting);
}

void event (List *waiting, Queue *ready, Algtype type, char name[]) {
	waitingToReady (waiting, ready, type, name);
}

void tickFCFS (Value *running,  Queue *ready, Queue *terminated) {
	// printf("in is empty ? %s\n",isEmpty(ready) );
	if (running->isInitialized == 0 && !isEmpty(ready)) {
		// printf("sal3\n");
		readyToRunning (running, ready);
		// printf("sa4l\n");
	} else if (running->isInitialized == 1) {
		if (running->BT > 1) {
			running->BT--;
		} else {
			running->BT--;
			printf("Stopping process..\n");
			runningToTerminated (running, terminated);
			if (ready->list->head != NULL) {
				readyToRunning (running, ready);
			}
		}
	}
}

void tickRR (Value *running,  Queue *ready, Queue *terminated, int Q, int *TL) {
	if (running->isInitialized == 1) {
		if ((*TL) > 1 && running->BT > 1) {
			running->BT--;
			(*TL)--;
		} else {
			if (running->BT > 1) {
				running->BT--;
				runningToReady (running, ready, RR);
			} else {
				running->BT--;
				printf("Stopping process..\n");
				runningToTerminated (running, terminated);
			}
			(*TL) = Q;
			if (ready->list->head != NULL) {
				readyToRunning (running, ready);
			}
		}
	} else {
		if (ready->list->head != NULL) {
				readyToRunning (running, ready);
			(*TL) = Q;
		}
	}
}

void tickPP (Value *running,  Queue *ready, Queue *terminated, int Q, int *TL) {
	
	if (running->isInitialized == 1) {
		if ((*TL) > 1 && running->BT > 1) {
			running->BT--;
			(*TL)--;
		} else {
			if (running->BT > 1) {
				running->BT--;
				runningToReady (running, ready, PP);
			} else {
				running->BT--;
				printf("Stopping process..\n");
				runningToTerminated (running, terminated);
			}
			(*TL) = Q;
			if (ready->list->head != NULL) {
				readyToRunning (running, ready);
			}
		}
	} else {
		if (ready->list->head != NULL) {
				readyToRunning (running, ready);
			(*TL) = Q;
		}
	}
}

void tick (Value *running, Queue *ready, Queue *terminated, Algtype type, int Q, int P, int *TL) {
	if (type == FCFS) {
		tickFCFS (running, ready, terminated);
	} else if (type == SJF) {
		tickFCFS (running, ready, terminated);
	} else if (type == RR) {
		tickRR (running, ready, terminated, Q, TL);
	} else if (type == PP) {
		tickPP (running, ready, terminated, Q, TL);
	}
	printf("Name: %s.BT remaining: %d --- %d\n", running->name, running->BT, running->isInitialized);
}

void parseAndRun (FILE *input_file, FILE *output_file, Algtype type, int Q, int Prl, int TL) {
	Queue *ready = createQueue();
	Queue *terminated = createQueue();
	List *waiting = createList();
	Value *running = (Value *)malloc(sizeof(Value));
	running->isInitialized = 0;
	printf("WTF is ? %d\n", isEmpty(ready));
	//more variables for reading
	//line by line from input_file
	char buff[100], pname[21];
	int BT, P;
	/**things for testing**/
	// int l = 2;
	// int time = 0;
	char *tok;
	TL = Q;
	// printf("Time now: %d second(s)\n", time);
	while (fgets(buff, 100, (FILE *)input_file)) {
		printf("%c==>:",buff[0]);
		if (buff[0] == 'a') {
			sscanf(buff, "a %s %d %d", pname, &BT, &P);
			Value new_value = createValue (pname, BT, P);
			add (new_value, ready, type);
			printf("Added: %s %d\n", ready->list->head->value.name, ready->list->head->value.BT);
			tick (running, ready, terminated, type, Q, Prl, &TL);
		} else if (buff[0] ==  't') {
			tick (running, ready, terminated, type, Q, Prl, &TL);
		} else if (buff[0] == 'w') {
			wait (running, waiting);
			printf("%s is now waiting.Bt left: %d\n",waiting->head->value.name, waiting->head->value.BT );
			tick (running, ready, terminated, type, Q, Prl, &TL);
		} else if (buff[0] == 'e') {
			sscanf(buff, "e %s", pname);
			event (waiting, ready, type, pname);
			tick (running, ready, terminated, type, Q, Prl, &TL);
		} else if (buff[0] == 's') {
			if (running->isInitialized == 1) {
				fprintf(output_file, "%s %d\n", running->name, running->BT);
			} else if (running->isInitialized == 0) {
				fprintf(output_file, "\n");
				printf("salut\n");
			}
		} else {
			strcpy (buff,buff + 3);
			tok = strtok(buff, " "); 

			while (tok) {
				strcpy(pname, tok);
				tok = strtok(NULL, " ");
				BT = atoi(tok);
				tok = strtok(NULL, " ");
				P = atoi(tok);
				Value new_value = createValue (pname, BT, P);
				add (new_value, ready, type);
		
				tok = strtok(NULL, " ");
			}
			tick (running, ready, terminated, type, Q, Prl, &TL);
		}
	}

}

/*Manages I/O files and chooses CPU schedule*/
void manageInOut(char *input, char *output) {
	FILE *input_file;
	FILE *output_file;
	input_file = fopen(input, "r");
	output_file = fopen(output, "w");

	// buff for reading line by line from "input_file"
	char buff[100];

	// CPU scheduling type variables
	int type;//scheduling type number
	int q;//time quantum
	int p;//priority levels

	fgets(buff, 100, (FILE *)input_file);
	sscanf(buff, "%d", &type);

	if (type > 3) {
		sscanf(buff, "%d %d %d", &type, &q, &p);
	} else if (type == 3) {
		sscanf(buff, "%d %d", &type, &q);
	}

	switch (type) {
		case 1 :
			parseAndRun (input_file, output_file, FCFS , 0, 0, 0);
			break;
		case 2 :
			parseAndRun (input_file, output_file, SJF, 0, 0, 0);
			break;
		case 3 :
			parseAndRun (input_file, output_file, RR, q, 0, q);
			break;
		case 4 :
			parseAndRun (input_file, output_file, PP, q, p, q);
			break;

	}
	fclose(input_file);
	fclose(output_file);
}

int main (int argc, char **argv) {
	if (argc < 3) {
		// Not enough arguments
		printf("Not enough arguments!\n");
		return 0;
	} else {
		// Good to go
		manageInOut(argv[1], argv[2]);
	}

	return 0;
}
