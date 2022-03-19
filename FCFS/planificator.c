#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//structura pentru procese
typedef struct p {
	char name[21];
	int btime;
	int p;
	struct p *next;
} prcs;

//functii folosite
void read(char *input, char *output);
void FCFS(FILE *ifile, char *output);
prcs *newprcs();
prcs *event_cmd (prcs *ready, prcs **waiting, char prcsname[]);
prcs *wait_cmd (prcs *waiting, prcs **running);
prcs *tick_cmd(prcs *running, prcs **ready, prcs **terminated);
void add_cmd(prcs **first, char prcsname[], int x, int y);
// void operate(int type, FILE *ifile, char *output);
//descriere functii
void read(char *input, char *output) {
	FILE *ifile;
	char buff[100];
	int type, q, p;

	ifile = fopen(input, "r");
	fgets(buff, 100, (FILE *)ifile);
	sscanf(buff, "%d",&type);

	if (type > 2) {
		sscanf(buff, "%d %d", &type, &q);
	}
	if (type > 3) {
		sscanf(buff, "%d %d %d", &type, &q, &p);
	}
	switch (type) {
		case 1 :
			FCFS(ifile, output);//de schimbat
			break;
	// 	case 2 :
	// 		operate(2, ifile, output);
	// 		break;
	// 	case 3 :
	// 		operate(3, ifile, output);
	// 		break;
	// 	case 4 :
	// 		operate(4, ifile, output);
	// 		break;
	// 	case 5 :
	// 		operate(5, ifile, output);
	// 		break;
	}

	fclose(ifile);
}

void FCFS(FILE *ifile, char *output) {
	prcs *ready = NULL;
	prcs *running = NULL;
	prcs *waiting = NULL;
	prcs *terminated = NULL;
	char buff[100], prcsname[21];
	int time = 0;
	int x, y;
	int i = 2; //testare
	FILE *ofile = fopen(output, "w");

	while (fgets(buff, 100, (FILE *)ifile)) {
		/*adaugare proces nou in ready*/
		printf("=====LINIA: %d.%sTime before:%d\n", i++, buff, time);
		if (buff[0] == 'a') {
			printf("Se adauga un proces nou!\n");//test
			sscanf(buff, "a %s %d %d", prcsname, &x, &y);//vezi cu a-ul
			printf("1running:%s\n", running->name);
			printf("1ready:%s\n", ready->name);
			add_cmd(&ready, prcsname, x, y);
			printf("2running:%s\n", running->name);
			printf("2ready:%s\n", ready->name);
			/*tick implicit*/
			running = tick_cmd(running, &ready, &terminated);
			time++;
			printf("3running:%s\n", running->name);
			printf("3ready:%s\n", ready->name);
			printf("terminated:%s\n", terminated->name);

		} else if (buff[0] == 't') {
			running = tick_cmd(running, &ready, &terminated);
			time++;
			printf("3running:%s\n", running->name);
			printf("3ready:%s\n", ready->name);
			printf("terminated:%s\n", terminated->name);
		} else if (buff[0] == 'w') {
			waiting = wait_cmd (waiting, &running);
			running = tick_cmd(running, &ready, &terminated);
			time++;
			printf("3running:%s\n", running->name);
			printf("3ready:%s\n", ready->name);
			printf("terminated:%s\n", terminated->name);
		} else if (buff[0] == 'e') {
			sscanf(buff, "e %s", prcsname);
			ready = event_cmd(ready, &waiting, prcsname);
			running = tick_cmd(running, &ready, &terminated);
			time++;
			printf("3running:%s\n", running->name);
			printf("3ready:%s\n", ready->name);
			printf("terminated:%s\n", terminated->name);
		} else if (buff[0] == 's') {
			if (running) {
				fprintf(ofile, "%s %d\n", running->name, running->btime);
				printf("%s %d\n", running->name, running->btime);
			} else {
				fprintf(ofile, "\n");
				printf("NUthing\n");
			}
		} else if (buff[0] == 'm' && buff[1] == 'a') {
			while (strlen(buff) > 4) {
				sscanf(buff, "ma %s %d %d", prcsname, &x, &y);
				add_cmd(&ready, prcsname, x, y);
				int x_l = 1;
				int aux = x;
				while (aux / 10) {
					x_l++;
					aux = aux / 10;
				}
				strcpy((buff + 3), (buff + 7 + x_l + strlen(prcsname)));
			}
			running = tick_cmd(running, &ready, &terminated);
			time++;
		}
		printf("S-a citit o linie.Time after:%d\n",time);//test
	}
	fclose(ofile);
}

prcs *newprcs() {
	prcs *newprcs = (prcs *)malloc(sizeof(prcs));
	newprcs->next = NULL;
	return newprcs;
}

prcs *dequeue(prcs *first) {
	prcs *tmp = first;
	first = first->next;
	free(tmp);
	return first;
} //nu stiu daca o folosesc


prcs *tick_cmd(prcs *running, prcs **ready, prcs **terminated) {
	if (running == NULL && (*ready) == NULL) {
		return running;
	} else if (running == NULL) {
		prcs *tmp = *ready;
		tmp->next = NULL;
		running = tmp;
		(*ready) = (*ready)->next;
		printf("#running:%s\n", running->name);
		printf("#running btime:%d\n", running->btime);
		printf("#ready:%s\n", (*ready)->name);
		return running;
	} else if (running != NULL) {
		if (running->btime > 1) {
			running->btime--;
			printf("#running btime:%d\n", running->btime);
		} else if (running->btime == 1) {
			running->btime--;
			printf("#running btime:%d. Terminating process...\n", running->btime);
			add_cmd(&(*terminated), running->name, 0, running->p);//////////////////////////////
			// free(running);
			// running == NULL;
			running = dequeue(running);
			running  = tick_cmd(running, &(*ready), &(*terminated));
		}
		return running;
	}
}

prcs *wait_cmd (prcs *waiting, prcs **running) {
	add_cmd(&waiting, (*running)->name, (*running)->btime, (*running)->p);
	(*running) = dequeue(*running);
	return waiting;
}

void add_cmd (prcs **first, char prcsname[], int x, int y) {
	prcs *new = newprcs();
	new->btime = x;
	new->p = y;
	strcpy(new->name, prcsname);

	if (*first == NULL) {
		*first = new;
	} else {
		prcs *iter = *first;
		while (iter->next != NULL) {
			iter = iter->next;
		}
		iter->next = new;
	}
}
prcs *event_cmd (prcs *ready, prcs **waiting, char prcsname[]) {
	if (strcmp((*waiting)->name, prcsname) == 0 ) {
		add_cmd(&ready, (*waiting)->name, (*waiting)->btime, (*waiting)->p);
		(*waiting) = dequeue(*waiting);
	} else {
		prcs *tmp = (*waiting);
		while (tmp->next) {
			if (strcmp((tmp->next->name), prcsname) == 0) {
				prcs *del = tmp->next;
				tmp->next = tmp->next->next;
				add_cmd(&ready, del->name, del->btime, del->p);
				free(del);
			} else {
				tmp = tmp->next;
			}
		}
	}
	return ready;
}

// prcs *add_cmd(prcs *ready, char prcsname[], int x, int y) {
// 	prcs *tmp = newprcs();
// 	if (ready == NULL) {
// 		tmp->btime = x;
// 		tmp->p = y;
// 		strcpy(tmp->name, prcsname);
// 		ready = tmp;
// 		return ready;
// 	} else {
// 		tmp = ready;
// 		while(tmp->next) {
// 			tmp = tmp->next;
// 		}
// 		tmp->next = newprcs();
// 		tmp->next->btime = x;
// 		tmp->next->p = y;
// 		strcpy(tmp->next->name, prcsname);
// 		return ready;
// 	}
// }


int main(int argc, char **argv)
{
	if (argc < 3) {
		// nu sunt destule argumente date
		printf("Nu sunt destule argumente date!\n");
		return 0;
	} else {
		// incepem rularea programului 
		read(argv[1], argv[2]);
	}
	return 0;
}

// void operate(int type, FILE *ifile, char *output) {
// 	//se executa functiile specifice type-ului dat
// 	char buff[100], cmd[3], pname[21];
// 	pss *ready = NULL;

// 	int x, y;
// 	while (fgets(buff, 100, (FILE *)ifile)) {
// 		sscanf(buff, "%s", cmd);

// 		if(strcmp(cmd,"a") == 0) {
// 			printf("plm e a\n");
// 			sscanf(buff, "%s %s %d %d",cmd ,pname, &x, &y);
// 			ready = a_cmd(type, pname, x, y, ready)
// 		}
// 		printf("plmsss\n");
// 	}
// }

// pss *a_cmd(int type, char pname[], int x, int y, pss* ready) {
// 	if (ready == NULL) {
// 		pss* new = (pss *)malloc(pss);
// 		new->btime = x;
// 		new->p = y;
// 		strcpy(new->name,pname);
// 		new->next = NULL;
// 		ready = new;
// 	} else {

// 	}
// }