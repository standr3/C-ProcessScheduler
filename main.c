#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.c"

#define test puts("testing!");

int gTicks;
Queue *gReady, *gTerminated;
Node *gRunning;
char ready[256] = "\0";
char buffer[20];

void input_cmd(char);


int main() {

    gTicks =  0;
    gReady = newQueue();
    gTerminated = newQueue();
    gRunning = NULL;

    system("clear");

    char cmd = 1;
    while (cmd != 'q') {
        printf("Global time: %d", gTicks);
        printf("\nReady: ");
        printQueue(gReady);

        printf("\nRunning: ");
        if (gRunning) {
            printf("%s::%d ", gRunning->name
                            , gRunning->life);
        } else {
            printf("-empty-");
        }

        printf("\nTerminated: ");
        printQueue(gTerminated);

        printf("\nq:quit; space:tick; a:add; w:wait; e:event; s:show; \n");
        system ("/bin/stty raw");
        cmd = getchar();
        system ("/bin/stty cooked");
        
        if (cmd != 'q') {
            input_cmd(cmd);
        }
        
        system("clear");
    }
    
    return 0;
}

void input_cmd(char c) {

    if (c == ' ') { 
        gTicks++; 
        if ( !gRunning ) {
            // Nothing is running
            if ( !isEmpty(gReady) ) { gRunning = popNode(gReady); }
        } else {
            // Proccess in running state
            gRunning->life--;
            if ( gRunning->life == 0 ) {
                pushNode(gTerminated, gRunning);
                gRunning = NULL;

                if ( !isEmpty(gReady) ) { gRunning = popNode(gReady); }
            }
        }
    }
    
    if (c == 'a') { 
        system("clear");
        Node *new = (Node *) malloc(sizeof(Node));
        
        scanf("%s %d", buffer, &(new->life));
        strcpy(new->name, buffer);
        new->arrival = gTicks;
        new->next = NULL;
        pushNode(gReady, new);
    }
}