#include "builtin.h"
#include "command.h"
#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tests/syscall_mock.h"

char *internal_cmd[] = {"cd", "help", "exit",
                        NULL}; // array con los comandos internos permitidos

enum internal_cases {
    cd,
    help,
    Exit
}; // tipo enumerado para despues usar en un switch

bool builtin_is_internal(scommand cmd) {
    bool res;
    assert(cmd != NULL);

    int i = 0;

    do {
        res = strcmp(scommand_front(cmd), internal_cmd[i]) ==
              0; // comparo el comando obtenido con los elementos del arreglo
        i++;
    } while (!res && internal_cmd[i]);

    return res;
}

bool builtin_alone(pipeline p) {
    assert(p != NULL);

    return pipeline_length(p) == 1 && builtin_is_internal(pipeline_front(p));
}

/*
Esta funcion busca la posicion del comando interno en el arreglo de comandos

*/
static int where_in_internal(scommand cmd) {
    int pos = 0;
    bool found = false;

    while (!found && internal_cmd[pos]) {
        found = strcmp(scommand_front(cmd), internal_cmd[pos]) ==
                0; // comparo el comando obtenido con los elementos del arreglo
        pos++;
    }

    return --pos;
}

static void options(void) {
    printf("\t\tINTERNAL OPTIONS\t\t\n"
           "cd <directory> --> changes to another directory\n"
           "help--> prints information\n"
           "exit--> you can well... exit\n");
}

void builtin_run(scommand cmd) {
    assert(builtin_is_internal(cmd));
    int position = where_in_internal(cmd);
    switch (position) {
    case (cd):
        scommand_pop_front(cmd); // quitamos el "cd" de la instruccion

        if (scommand_is_empty(cmd)) {
            printf("wtf did you say\n");
        } else if (chdir(scommand_front(cmd)) != 0) {
            printf("path does not exist in the current directory\n");
        }
        break;
    case (help):

        printf("\t\tMY BASH\t\t\n");
        printf("A project created by a Lilen Bena, Camila Castillo, Nicolas "
               "Mansutti and Pietro Palombini\n");
        printf("There is only one thing that we can't do and is talking to "
               "women\n");
        printf("This is a bash to rule them all\n");
        options();
        break;
    case (Exit):
        exit(EXIT_SUCCESS);
        break;
    }
}
