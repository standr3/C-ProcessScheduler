#include <ctype.h>
#include <values.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>



#define NODE_T_SIZE sizeof(node_t)
#define PROC_T_SIZE sizeof(proc_t)
#define PROC_NAME_LEN 12


typedef struct proc {
    char name[PROC_NAME_LEN];
    int runtime;
    int priority;
} proc_t;

typedef struct node {
    void *data;
    size_t data_size; 
    struct node *next;
} node_t;

typedef enum algorithm {
    ALG_NONE = 0,
    ALG_FCFS = 1,
    ALG_SJF  = 2,
    ALG_RR   = 3,
    ALG_PP   = 4
} algorithm_t;

typedef enum cmd_type{
    CMD_NONE       = 0,
    CMD_INIT_SCHED = 1,
    CMD_TICK       = 2,
    CMD_ADD        = 3,
    CMD_MULTI_ADD  = 4,
    CMD_WAIT       = 5,
    CMD_EVENT      = 6,
    CMD_SHOW       = 7
} cmd_type_t;

typedef struct command {
    cmd_type_t type;
    size_t argc;
    char** argv;
} command_t;

typedef struct proc_states {
    node_t* new;
    node_t* ready;
    node_t* running;
    node_t* waiting;
    node_t* terminated;
    int time;
} proc_states_t;

typedef struct scheduler {
    FILE *file_out;
    algorithm_t type;
    int cpu_time;
    int proc_time;
    proc_states_t states;
    void (*execute_task)(struct scheduler*, command_t);
} scheduler_t;

void enqueue_node(node_t** list, node_t* new) {
    if (*list == NULL) {
        *list = new;
    } else {
        node_t* cursor = *list;
        while (cursor->next) {
            cursor = cursor->next;
        }
        cursor->next = new;
    }   
}
node_t* dequeue_node(node_t** list) {
    node_t* deq_node = NULL;

    if(*list != NULL) {
        deq_node = *list; 
        *list = (*list)->next;
        deq_node->next = NULL;
    }
    return deq_node;
}

void print_proc_node(node_t* node) {
    if (node) {
        proc_t* p = (proc_t*)node->data;
        printf("{%s/%d/%d}>",p->name,p->priority,p->runtime);
    }
}

void print_proc_list(node_t* list) {
    if (list) {
        node_t* cursor = list;
        while (cursor) {
            print_proc_node(cursor);
            cursor = cursor->next;
        }
        printf("{}\n");
    } else {
        printf(" - \n");
    }
}

node_t* node_create_empty() {
    node_t *new = (node_t*) malloc(NODE_T_SIZE);

    *new = (node_t) {
        .data = NULL,
        .data_size = -1,
        .next = NULL,
    };

    return new;
}

void* _node_copy_data(const void *data_ptr,const size_t data_size) {
    void* clone_ptr = malloc(data_size);
    memcpy(clone_ptr, data_ptr, data_size);
    return clone_ptr;
}

void node_set_data(node_t* node_ptr, const void *data_ptr, const size_t data_size) {
    *node_ptr = (node_t) {
        .data = _node_copy_data(data_ptr, data_size),
        .data_size = data_size
    };
}

node_t* node_create(const void* data_ptr, const size_t data_size) {
    node_t *new = node_create_empty();

    node_set_data(new, data_ptr, data_size);
    
    return new;
}

char* to_string_algorithm(algorithm_t type) {
    char* names[] = {
    "ALG_NONE",
    "ALG_FCFS",
    "ALG_SJF" , 
    "ALG_RR"  , 
    "ALG_PP" 
    };
    return names[(int)type];
}

scheduler_t* schedule_create_empty() {
    scheduler_t* new = (scheduler_t*) malloc(sizeof(scheduler_t));
    
    *new = (scheduler_t) {
        .file_out = NULL,
        .type = ALG_NONE,
        .cpu_time = 0,
        .proc_time = 0,
        .states = (proc_states_t) {
            .new = NULL,
            .ready = NULL,
            .running = NULL,
            .waiting = NULL,
            .terminated = NULL,
            .time = 0
        },
        .execute_task = NULL,
    };
    
    return new;
}

char* to_string_cmd_type(cmd_type_t type) {
    char* names[] = {
    "CMD_NONE"      ,
    "CMD_INIT_SCHED",
    "CMD_TICK"      ,
    "CMD_ADD"       ,
    "CMD_MULTI_ADD" ,
    "CMD_WAIT"      ,
    "CMD_EVENT"     ,
    "CMD_SHOW" 
    };
    return names[(int)type];
}

char** get_words(char* str, int* num_words) {
    char** words = NULL;
    char* word = strtok(str, " \t\n");   // split string into words using whitespace characters as delimiters
    
    (*num_words) = 0;
    while (word != NULL) {
        (*num_words)++;   // increment number of words
        words = realloc(words, (*num_words) * sizeof(char*));   // resize array to hold one more word

        if (words == NULL) {   // check if allocation failed
            printf("Error: could not allocate memory\n");
            exit(1);
        }

        words[(*num_words) - 1] = malloc((strlen(word) + 1) * sizeof(char));   // allocate memory for the word

        if (words[(*num_words) - 1] == NULL) {   // check if allocation failed
            printf("Error: could not allocate memory\n");
            exit(1);
        }

        strcpy(words[(*num_words) - 1], word);   // copy the word into the array
        word = strtok(NULL, " \t\n");   // get the next word
    }

    return words;
}

command_t* command_create_empty() {
    command_t* new = (command_t*) malloc(sizeof(command_t));
    
    *new = (command_t) {
        .type = CMD_NONE,
        .argc = -1,
        .argv = NULL
    };

    return new;
}



proc_t* get_proc(node_t* node) {
    return (proc_t*)node->data;
}
proc_t* create_proc(char*name, int runtime, int priority) {
    proc_t* new = NULL;
    new = (proc_t*) malloc(PROC_T_SIZE);
    strncpy(new->name, name, PROC_NAME_LEN);
    new->runtime=runtime;
    new->priority=priority;

    return new;
}
proc_t* str_to_proc(char** words) {
    assert(words && words[0] && words[1] && words[2]);
    return create_proc(
        words[0], 
        atoi(words[1]), 
        atoi(words[1])
    );
}
node_t* proc_to_node(proc_t* proc) {
    assert(proc);
    node_t * wrapper = NULL;
    wrapper = node_create(proc, PROC_T_SIZE);
    return wrapper;
}
void print_states(proc_states_t* states) {
    printf("--------STATES-------- \n");
    printf("NEW: \n");
    print_proc_list(states->new);
    // printf("-%s\n",((proc_t*)states->new->data)->name);
    printf("---------------------- \n\n");
}

void move_node(node_t** src, node_t** dest) {
    if (*src != NULL) {
        node_t* deq = NULL;
        deq = dequeue_node(src);
        enqueue_node(dest, deq);
    }
}
void move_priority_first(node_t** src, node_t** dest) {
    
    node_t* current = *src;
    node_t* node = current;
    
    // Get the first node with the shortest runtime
    while (current != NULL) {
        if (get_proc(current)->priority > get_proc(node)->runtime) {
            node = current;
        }
        current = current->next;
    }

    // Remove the node from the source queue
    current = *src;
    if (node == current) {
        *src = current->next;
    } else {
        while (current->next != NULL) {
            if (current->next == node) {
                current->next = node->next;
                break;
            }
            current = current->next;
        }
    }

    // Insert the node into the destination queue
    current = *dest;
    if (current == NULL || get_proc(current)->priority < get_proc(node)->priority) {
        node->next = current;
        *dest = node;
    } else {
        while (current->next != NULL && get_proc(current->next)->priority > get_proc(node)->priority) {
            current = current->next;
        }
        node->next = current->next;
        current->next = node;
    }
}
void move_shortest_first(node_t** src, node_t** dest) {
    node_t* current = *src;
    node_t* node = current;
    
    // Get the first node with the shortest runtime
    while (current != NULL) {
        if (get_proc(current)->runtime < get_proc(node)->runtime) {
            node = current;
        }
        current = current->next;
    }

    // Remove the node from the source queue
    current = *src;
    if (node == current) {
        *src = current->next;
    } else {
        while (current->next != NULL) {
            if (current->next == node) {
                current->next = node->next;
                break;
            }
            current = current->next;
        }
    }

    // Insert the node into the destination queue
    current = *dest;
    if (current == NULL || get_proc(current)->runtime > get_proc(node)->runtime) {
        node->next = current;
        *dest = node;
    } else {
        while (current->next != NULL && get_proc(current->next)->runtime < get_proc(node)->runtime) {
            current = current->next;
        }
        node->next = current->next;
        current->next = node;
    }
}
// new to ready
void proc_admit(algorithm_t a, proc_states_t* states) {
    if (a == ALG_FCFS || a == ALG_RR) {
        move_node(&(states->new), &(states->ready));
    } else if (a == ALG_SJF) {
        move_shortest_first(&(states->new), &(states->ready));
    } else if (a == ALG_PP) {
        move_priority_first(&(states->new), &(states->ready));
    }
}
void check_new(algorithm_t a, proc_states_t* states) {
    while(states->new) {
        proc_admit(a, states);
    }
}
// ready to running
int is_running_empty(proc_states_t* states) {
    return (states->running == NULL);
}
void proc_dispatch(proc_states_t* states) {
    move_node(&(states->ready), &(states->running));
}
void check_ready(algorithm_t a, proc_states_t* states) {
    if (states->ready) {
        // if (a == ALG_FCFS || a == ALG_SJF || a == ALG_RR) {
            if (is_running_empty(states)) {
                proc_dispatch(states);
            }
        // } 
    }
}
void decrement_runtime(proc_t* proc) {
    proc->runtime--;
}
int is_runtime_over(proc_t* proc) {
    return (proc->runtime <= 0) ;
}
void proc_interrupt(algorithm_t a, proc_states_t* states) {
    if (a == ALG_RR)
        move_node(&(states->running), &(states->ready));
    else if (a == ALG_PP)
    {
        // print_proc_list(states->running);
        // print_proc_list(states->ready);
        move_priority_first(&(states->running), &(states->ready));
        
        // TESTARE
        // print_proc_list(states->running);
        // print_proc_list(states->ready);
    }
}
void proc_exit(proc_states_t* states) {
    move_node(&(states->running), &(states->terminated));
}
void check_running(scheduler_t* schedule) {
    
    algorithm_t a = schedule->type;
    proc_states_t* states = &(schedule->states);
    int* proc_time = &(schedule->proc_time);
    // TESTARE
    if (states->running) {
        printf("---%d\n",*proc_time);
        proc_t* running_proc = get_proc(states->running);
        decrement_runtime(running_proc);
        (*proc_time)++;
        if(is_runtime_over(running_proc)) {
            proc_exit(states); 
            *proc_time = 0;
        }


        if (a == ALG_FCFS || a == ALG_SJF) {
            // 
        }
        else if (a == ALG_RR) {
            if(*proc_time >= schedule->cpu_time) {
                proc_interrupt(a,states);
                *proc_time = 0;
            }
        }
        else if (a == ALG_PP) {
            if(*proc_time >= schedule->cpu_time) {
                print_proc_node(states->ready);
                enqueue_node(&(states->running), dequeue_node(&(states->ready)));
                // move_node(&(states->ready), &(states->running));
                proc_interrupt(a,states);
                *proc_time = 0;
            }


        }
    }
}

void proc_wait(proc_states_t* states) {
    move_node(&(states->running), &(states->waiting));
}
void proc_wake(scheduler_t* schedule, const char* proc_name) {
    
    proc_states_t*states =&(schedule->states);

    node_t* called = NULL;
    node_t*auxlist=NULL;
    while(states->waiting!=NULL) {
        node_t * deq = dequeue_node(&(states->waiting));
        if (strcmp(proc_name, get_proc(deq)->name)==0) {
            called = deq;
        } else {
            enqueue_node(&auxlist, deq);
        }
    } 
    states->waiting = auxlist;


    if (schedule->type == ALG_FCFS ||schedule->type == ALG_RR ) {
        enqueue_node(&(states->ready), called);
    } else if (schedule->type == ALG_SJF) {
        node_t* current;
        node_t** list = &(states->ready);

        // Case 1: Empty list or node has shorter runtime than first node
        if (*list == NULL || get_proc((*list))->runtime >= get_proc(called)->runtime) {
            called->next = *list;
            *list = called;
        } else {
            // Case 2: Node has longer runtime than first node, find its position
            current = *list;
            while (current->next != NULL && get_proc(current->next)->runtime <= get_proc(called)->runtime) {
                if (get_proc(current->next)->runtime == get_proc(called)->runtime) {
                    current = current->next;
                } else {
                    break;
                }
            }
            called->next = current->next;
            current->next = called;
        }
    }
}

void printfirst(const char* state, node_t* list) {
    if (list){
        printf(" %s -> %s %d %d <\n",
        state,
        get_proc(list)->name,
        get_proc(list)->runtime,
        get_proc(list)->priority);
    }
}
void update_states(scheduler_t* schedule) {
    
    // printfirst("NEW", schedule->states.new);
    check_new(schedule->type, &(schedule->states));
    // printfirst("READY", schedule->states.ready);
    check_running(schedule);
    // printfirst("RUN", schedule->states.running);
    check_ready(schedule->type, &(schedule->states));
    // printfirst("-READY", schedule->states.ready);
    // printfirst("-RUN", schedule->states.running);
    puts("||");
    
    schedule->states.time++;
    
}

void task_tick(scheduler_t* schedule, command_t cmd){
    update_states(schedule);
}
void _to_new(proc_states_t* states, node_t* node) {
    enqueue_node(&(states->new), node);
}
void task_add(scheduler_t* schedule, command_t cmd){
    proc_t* new_proc = str_to_proc(cmd.argv + 1);
    node_t* new_node = proc_to_node(new_proc);

    _to_new(&(schedule->states), new_node);
    update_states(schedule);
}
void task_multiple_add(scheduler_t* schedule, command_t cmd){
    int offset = 1;
    while (offset < cmd.argc) {
        proc_t* new_proc = str_to_proc(cmd.argv + offset);
        offset += 3;
        node_t* new_node = proc_to_node(new_proc);

        _to_new(&(schedule->states), new_node);
    }
    update_states(schedule);

}
void task_wait(scheduler_t* schedule, command_t cmd){
    
    schedule->proc_time = 0;
    proc_wait(&(schedule->states));
    update_states(schedule);
}
void task_event(scheduler_t* schedule, command_t cmd){

    const char* proc_name = cmd.argv[1];    
    proc_wake(schedule, proc_name);
    update_states(schedule);
}
void task_show(scheduler_t* schedule, command_t cmd){
    if(schedule->states.running) {
        if(schedule->file_out != NULL) {
            fprintf(schedule->file_out,
                "%s %d\n", 
                get_proc(schedule->states.running)->name,
                get_proc(schedule->states.running)->runtime
            );
        } else {
            printf("%s %d\n", 
                get_proc(schedule->states.running)->name,
                get_proc(schedule->states.running)->runtime
            );
        }

    } else {
        if(schedule->file_out != NULL) {
            fprintf(schedule->file_out,"\n");
        } else {
            printf("\n");
        }
    }
}
void task_algorithm_set(scheduler_t* schedule, command_t cmd){
    // printf("Setting algorithm to %s\n", to_string_algorithm(a));
    if (schedule->type == ALG_RR || schedule->type == ALG_PP) {
        schedule->cpu_time = atoi(cmd.argv[1]);
    }
}

cmd_type_t get_cmd_type(char** words){
    cmd_type_t type = -1;
    if (!strcmp(words[0], "t")) {
        type = CMD_TICK;
    } else if (!strcmp(words[0], "a")) {
        type = CMD_ADD;
    } else if (!strcmp(words[0], "ma")) {
        type = CMD_MULTI_ADD;
    } else if (!strcmp(words[0], "w")) {
        type = CMD_WAIT;
    } else if (!strcmp(words[0], "e")) {
        type = CMD_EVENT;
    } else if (!strcmp(words[0], "s")) {
        type = CMD_SHOW;
    } else {
        type = CMD_INIT_SCHED;
    }
    return type;
}

void (*line_parse(char* line, command_t* cmd))(scheduler_t*, command_t) {
    void (*decided_action)(scheduler_t*, command_t);
    int num_words = 0;
    char** words = get_words(line, &num_words);
    cmd_type_t type = get_cmd_type(words);
    
    *cmd = (command_t) {
        .type = type,
        .argc = num_words,
        .argv = words
    };
    
    switch (type) {
        case CMD_TICK:
            decided_action = task_tick;
            break;
        case CMD_ADD:
            decided_action = task_add;
            break;
        case CMD_MULTI_ADD:
            decided_action = task_multiple_add;
            break;
        case CMD_WAIT:
            decided_action = task_wait;
            break;
        case CMD_EVENT:
            decided_action = task_event;
            break;
        case CMD_SHOW:
            decided_action = task_show;
            break;
        default:
            decided_action = task_algorithm_set;
            break;
    }
    return decided_action;
}

int main(int argc, char** argv) {
    
    if (argc >= 2) {
        char buffer[255];
        FILE *fin = fin = fopen(argv[1], "r");
        
        scheduler_t* scheduler = schedule_create_empty();
        command_t command;
        if (argc >= 3) {
            scheduler->file_out = fopen(argv[2], "w+");
        }
        while (fgets(buffer, sizeof(buffer), fin)) {
            
            scheduler->execute_task = line_parse(buffer, &command);
            
            if (command.type == CMD_INIT_SCHED) {
                // tODO:FUNCTIE MACRO pt asta
                scheduler->type = command.argv[0][0] - '0';
                scheduler->execute_task(scheduler, command);
                continue;  
            } 
            scheduler->execute_task(scheduler, command);
        }
        fclose(fin);
        
        if (argc >= 3) {
            fclose(scheduler->file_out);
        }
    }

    return 0;
}
