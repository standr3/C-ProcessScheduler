#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define NODE_T_SIZE sizeof(node_t)


typedef struct node {
    void *data;
    size_t data_size; 
    struct node *next;
} node_t;

// typedef struct list {
//     node_t *head;
//     int size;
// } list_t;



node_t* node_create_empty() {
    node_t *new = NULL;
    new = (node_t*) malloc(NODE_T_SIZE);
    *new = (node_t) {
        .data = NULL,
        .data_size = -1,
        .next = NULL,
    };
    return new;
}

void* _node_copy_data(const void *data_ptr,const size_t data_size) {
    assert(data_ptr);
    assert(data_size >= 0);

    void* clone_ptr = NULL;
    clone_ptr = malloc(data_size);
    memcpy(clone_ptr, data_ptr, data_size);
    return clone_ptr;
}

void node_set_data(node_t* node_ptr, const void *data_ptr, const size_t data_size) {
    assert(node_ptr);
    assert(data_ptr);
    
    *node_ptr = (node_t) {
        .data = _node_copy_data(data_ptr, data_size),
        .data_size = data_size
    };
}

node_t* node_create(const void* data_ptr, const size_t data_size) {
    assert(data_ptr);
    assert(data_size >= 0);

    node_t *new = NULL;
    new = node_create_empty();
    node_set_data(new, data_ptr, data_size);
    return new;
}


typedef enum algorithm {
    ALG_NONE = 0,
    ALG_FCFS = 1,
    ALG_SJF  = 2,
    ALG_RR   = 3,
    ALG_PP   = 4
} algorithm_t;

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
    algorithm_t type;
    proc_states_t states;
    void (*execute_task)(algorithm_t, command_t, proc_states_t*);
} scheduler_t;


scheduler_t* schedule_create_empty() {
    scheduler_t* new = NULL;
    new = (scheduler_t*) malloc(sizeof(scheduler_t));

    *new = (scheduler_t) {
        .type = ALG_NONE,
        .execute_task = NULL
    };
    return new;
}
void schedule_set_type(scheduler_t* const schedule, algorithm_t type) {
    assert(schedule);
    
    schedule->type = type;
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



// char* skip_whitespace(char* str) {
//     while (isspace(*str)) {
//         str++;
//     }
//     return str;
// }
// int has_number_in_front(char* str) {
//     if (isdigit(*str)) {
//         return 1;
//     }
//     return 0;
// }
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
    command_t* new = NULL;
    new = (command_t*) malloc(sizeof(command_t));
    *new = (command_t) {
        .type = CMD_NONE,
        .argc = -1,
        .argv = NULL
    };
    // new->type = CMD_NONE;
    // new->argc = -1;
    // new->argv = NULL;
    puts("LOL");
    return new;
}
#define PROC_T_SIZE sizeof(proc_t)
#define PROC_NAME_LEN 12
typedef struct proc {
    int runtime;
    int priority;
    char name[PROC_NAME_LEN];
} proc_t;

proc_t* create_proc(int r, int p, char* n) {
    proc_t* new = NULL;
    new = (proc_t*) malloc(PROC_T_SIZE);
    new->runtime=r;
    new->priority=p;
    strncpy(new->name, n, PROC_NAME_LEN);

    return new;
}
void update_states(proc_states_t* states) {
    states->time++;
}

void task_tick(algorithm_t a, command_t cmd, proc_states_t* states){
    assert(a != ALG_NONE);
    assert(cmd.type == CMD_TICK);
    assert(states);
    // printf("This is task_tick\n");
    // updates
    update_states(states);
}
void task_add(algorithm_t a, command_t cmd, proc_states_t* states){
    // printf("This is task_add\n");
    assert(a != ALG_NONE);
    assert(cmd.type == CMD_ADD);
    assert(states);
    
    proc_t* new_proc = create_proc(
         atoi(cmd.argv[1]),
         atoi(cmd.argv[2]),
         cmd.argv[3]);
    states->new = node_create(new_proc, PROC_T_SIZE);
    
}
void task_multiple_add(algorithm_t a, command_t cmd, proc_states_t* states){
    printf("This is task_multiple_add\n");
}
void task_wait(algorithm_t a, command_t cmd, proc_states_t* states){
    printf("This is task_wait\n");
}
void task_event(algorithm_t a, command_t cmd, proc_states_t* states){
    printf("This is task_event\n");
}
void task_show(algorithm_t a, command_t cmd, proc_states_t* states){
    printf("This is task_show\n");
}
void task_algorithm_set(algorithm_t a, command_t cmd, proc_states_t* states){
    printf("Setting algorithm to %s\n", to_string_algorithm(a));
}

cmd_type_t get_cmd_type(char** words){
    assert(words);
    assert(words[0]);
    cmd_type_t type = -1;
    // puts("DSDS:");
    // puts(words[0]);

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

void (*line_parse(char* line, command_t* cmd))(algorithm_t, command_t, proc_states_t* states) {
    assert(line);
    assert(cmd);

    cmd_type_t type = -1;
    // cmd = command_create_empty();

    // line = skip_whitespace(line);
    int num_words = 0;
    char** words = get_words(line, &num_words);
    type = get_cmd_type(words);

    // printf("Number of words: %d\n", num_words);
    for (int i = 0; i < num_words; i++) {
        printf("[%s]", words[i]);
    }
    printf("\n");
    // // Free memory
    // for (int i = 0; i < num_words; i++) {
    //     free(words[i]);
    // }
    // free(words);
    // cmd-> type = type;
    // cmd-> argc = num_words;
    // cmd-> argv = words;
    // printf("type: %d %d \n", cmd->type, cmd->argc);
    
    *cmd = (command_t) {
        .type = type,
        .argc = num_words,
        .argv = words
    };
    void (*action)(algorithm_t, command_t, proc_states_t*);
    switch (type) {
        case CMD_TICK:
            // printf("Value is CMD_TICK\n");
            action = task_tick;
            break;
        case CMD_ADD:
            // printf("Value is CMD_ADD\n");
            action = task_add;
            break;
        case CMD_MULTI_ADD:
            // printf("Value is CMD_MULTI_ADD\n");
            action = task_multiple_add;
            break;
        case CMD_WAIT:
            // printf("Value is CMD_WAIT\n");
            action = task_wait;
            break;
        case CMD_EVENT:
            // printf("Value is CMD_EVENT\n");
            action = task_event;
            break;
        case CMD_SHOW:
            // printf("Value is CMD_SHOW\n");
            action = task_show;
            break;
        default:
            // printf("Value is CMD_INIT_SCHED\n");
            action = task_algorithm_set;
            break;
    }
    // puts("l1l");

    return action;
}



int main(int argc, char** argv) {

    // int a = 34;
    // node_t* n0 = node_create_empty();
    // node_set_data(n0, &a, sizeof(int));
    // node_t* n1 = node_create(&a, sizeof(int)); 
    // printf("%d\n", ((int*)n1->data)[0]);

    if (argc >= 2) {
        FILE *fp;
        char buffer[255];
        
        scheduler_t* scheduler = schedule_create_empty();
        command_t command;
        proc_states_t states = {
            .new = NULL,
            .ready = NULL,
            .running = NULL,
            .waiting = NULL,
            .terminated = NULL,
            .time = 0
        };
        // Open the file for reading
        fp = fopen(argv[1], "r");

        // Read each line of the file and print it to the console
        while (fgets(buffer, sizeof(buffer), fp)) {
            // printf("in while : %s =================", buffer);

            // printf("[[[lol ---\n");
            scheduler->execute_task = line_parse(buffer, &command);
            // printf("[[[[lol\n");
             

            // printf("======\n");
            // printf("ddd---%d %d %s \n", command.type, command.argc, command.argv[0]);
            if (command.type == CMD_INIT_SCHED) {
                // tODO:FUNCTIE MACRO pt asta
                scheduler->type = command.argv[0][0] - '0';
                scheduler->execute_task(scheduler->type, command, &states);
                continue;  
            } 
            scheduler->execute_task(scheduler->type, command, &states);
        }
        printf("\n");
        // Close the file
        fclose(fp);
    }

    return 0;
}