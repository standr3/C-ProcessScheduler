#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*Value structure*/
typedef struct VALUE {
	char name[21];
	int BT;
	int P;
	int isInitialized;
} Value;

/*Creates a new value with given name ,BT and P*/
Value createValue (char name[], int BT, int P) {
	Value new_value;

	strcpy(new_value.name, name);
	new_value.BT = BT;
	new_value.P = P;
	new_value.isInitialized = 1;

	return new_value;
}

/*Node structure*/
typedef struct NODE {
	Value value;
	struct NODE *next;
} Node;

/*Creates a new node with given value*/
Node *createNode (Value val) {
	Node *new_node = (Node *)malloc(sizeof(Node));

	new_node->value = val;
	new_node->next = NULL;

	return new_node;
}

/*List structure*/
typedef struct LIST {
	Node *head;
} List;

/*Creates a new list*/
List *createList () {
	List *list = (List *)malloc(sizeof(List));
	list->head = NULL;
	return list;
}

/*Inserts node with given value at beggining of a list*/
void insertNode (List *list, Value val) {
	Node *new_node = createNode (val);

	if (list->head) {
		Node *tmp = list->head;
		while (tmp->next) {
			tmp = tmp->next;
		}
		tmp->next = new_node;
	} else {
		list->head = new_node;
	}
}

void insertNodeInOrder (List *list, Value val, int (*comp) (Value, Value)) {
	if (list->head == NULL) {
		insertNode(list, val);
		return;
	}
	Node *new_node = createNode (val);

	Node *tmp = list->head;
	Node *prev = NULL;

	while (tmp != NULL && comp(val, tmp->value) >= 0) {
		prev = tmp;
		tmp = tmp->next;
	}

	if (prev == NULL) {
		Node *aux = list->head;
		list->head = new_node;
		new_node->next = aux;
	} else {
		prev->next = new_node;
		new_node->next = tmp;
	}
}

/*Removes first Node from list*/
void removeNode (List *list) {
	if (list->head) {
		Node *aux = list->head;
		list->head = list->head->next;
		free(aux);
	} else {
		list->head = NULL;
	}
}

/*Queue structure*/
typedef struct QUEUE {
	List *list;
} Queue;

/*Creates a new queue*/
Queue *createQueue() {
	Queue *new_queue = (Queue *)malloc(sizeof(Queue));

	new_queue->list = createList();
	new_queue->list->head = NULL;

	return new_queue;
}

/*Checks if queue is empty*/
int isEmpty (Queue *queue) {
	return queue->list->head == NULL;
}

/*Adds new element to a queue*/
void push (Queue *queue, Value val) {
	insertNode (queue->list, val);
}

/*Removes element from a queue*/
Value pop (Queue *queue) {
	if (queue->list->head ==NULL) {
		printf("Queue is empty, can't pop!\n");
		exit(0);
	}
	Value popped = queue->list->head->value;
	removeNode (queue->list);

	return popped;
}

/*Adds new element to a priority queue*/
void priorityPush (Queue * queue, Value val, int (*comp) (Value, Value)) {
	insertNodeInOrder (queue->list, val, comp);
}