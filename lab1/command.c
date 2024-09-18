#include "command.h"
#include <assert.h>
#include <glib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

struct scommand_s {
    GQueue *args;
    char *redir_input;
    char *redir_output;
};

scommand scommand_new(void) {
    scommand result = malloc(sizeof(struct scommand_s));
    assert(result != NULL);
    result->args = g_queue_new();
    result->redir_input = NULL;
    result->redir_output = NULL;

    assert(result != NULL && scommand_is_empty(result) &&
           scommand_get_redir_in(result) == NULL &&
           scommand_get_redir_out(result) == NULL);
    return result;
}

scommand scommand_destroy(scommand self) {
    assert(self != NULL);

    g_queue_free_full(self->args, free);
    g_free(self->redir_input);
    g_free(self->redir_output);
    g_free(self);
    self = NULL;

    return NULL;
}

/* Modificadores */

void scommand_push_back(scommand self, char *argument) {
    assert(self != NULL && argument != NULL);

    g_queue_push_tail(self->args, argument);

    assert(!scommand_is_empty(self));
}

void scommand_pop_front(scommand self) {
    assert(self != NULL && !scommand_is_empty(self));

    char *killme = g_queue_pop_head(self->args);
    g_free(killme);

    assert(scommand_is_empty(self) || scommand_length(self) > 0);
}

void scommand_set_redir_in(scommand self, char *filename) {
    assert(self != NULL);

    free(self->redir_input); /* en caso de que hubiese otra anteriormente */

    if (filename != NULL) {
        self->redir_input = filename;
    } else {
        self->redir_input = NULL;
    }
}

void scommand_set_redir_out(scommand self, char *filename) {
    assert(self != NULL);

    free(self->redir_output);

    if (filename != NULL) {
        self->redir_output = filename;
    } else {
        self->redir_output = NULL;
    }
}

/* Proyectores */

bool scommand_is_empty(const scommand self) {
    assert(self != NULL);
    return g_queue_is_empty(self->args);
}

unsigned int scommand_length(const scommand self) {
    assert(self != NULL);
    return g_queue_get_length(self->args);
}

char *scommand_front(const scommand self) {
    assert(self != NULL && !scommand_is_empty(self));
    char *result = (char *)g_queue_peek_head(self->args);
    assert(result != NULL);
    return result;
}

char *scommand_get_redir_in(const scommand self) {
    assert(self != NULL);
    return self->redir_input;
}

char *scommand_get_redir_out(const scommand self) {
    assert(self != NULL);
    return self->redir_output;
}

char *scommand_to_string(const scommand self) {
    assert(self != NULL);

    GString *str = g_string_new(NULL);

    /* añado los argumentos */
    for (GList *l = g_queue_peek_head_link(self->args); l; l = l->next) {
        if (l != g_queue_peek_head_link(self->args)) {
            g_string_append_c(str, ' ');
        }
        g_string_append(str, (char *)l->data);
    }

    /* redirec de entrada */
    if (self->redir_input) {
        g_string_append_printf(str, " < %s", self->redir_input);
    }

    /* redirec de salida */
    if (self->redir_output) {
        g_string_append_printf(str, " > %s", self->redir_output);
    }

    return g_string_free(str, FALSE);
}

struct pipeline_s {
    GList *commands; // no aseguro que esto sea asi
    bool wait; // booleano para verificar si tengo que esperar o no (a que no
               // se)
    unsigned int length; // cantidad de comandos en tuberia
};

bool invrep(pipeline p);
void scommand_destroy_wrapper(gpointer data);

// faltaria hacer el invariante mas completo
bool invrep(pipeline p) { return (p != NULL); }

pipeline pipeline_new(void) {
    pipeline p = g_malloc(
        sizeof(struct pipeline_s)); // reservo memoria para la estructura
    p->commands = NULL;             // apunto la lista a NULL
    p->wait = true;                 // por defecto pongo la espera en true
    p->length = 0; // inicio el largo de la cantidad de comandos en 0
    g_assert(invrep(p));
    return p;
}
void scommand_destroy_wrapper(gpointer data) {
    scommand_destroy(
        (struct scommand_s *)data); // tuve que castearlo en una funcion aparte
}

pipeline pipeline_destroy(pipeline self) {
    assert(invrep(self));
    g_list_free_full(
        self->commands,
        scommand_destroy_wrapper); // desrtuyo cada elemento de la lista
                                   // utilizando la funcion scommand_destroy
    free(self);                    // destruyo el resto de la estructura
    return NULL;
}

void pipeline_push_back(pipeline self, scommand sc) {
    g_assert(invrep(self));
    self->commands =
        g_list_append(self->commands, sc); // añado sc al final de la lista
    self->length++; // como añadi un comando agrando el campo length
}

void pipeline_pop_front(pipeline self) {
    g_assert(invrep(self) && !pipeline_is_empty(self));
    GList *first = g_list_first(self->commands);
    if (first != NULL) {
        scommand sc = g_list_nth_data(self->commands, 0);
        g_list_nth(self->commands, 0); // busco el primer nodo de la lista
        self->commands =
            g_list_delete_link(self->commands, first); // destruyo la lista
        scommand_destroy(sc); // desrtuyo el contenido de la lista
        self->length--;       // disminuyo el campo length
    }
}

void pipeline_set_wait(pipeline self, const bool w) {
    g_assert(invrep(self));
    self->wait = w; // cambio la condicion de el campo wait
}

bool pipeline_is_empty(const pipeline self) {
    g_assert(invrep(self));
    return self->length == 0;
}

unsigned int pipeline_length(const pipeline self) {
    g_assert(invrep(self));
    return self->length;
}

scommand pipeline_front(const pipeline self) {
    g_assert(invrep(self) && !pipeline_is_empty(self));
    GList *first = g_list_first(self->commands);
    return first->data;
}

bool pipeline_get_wait(const pipeline self) {
    g_assert(invrep(self));
    return self->wait;
}

char *pipeline_to_string(const pipeline self) {
    g_assert(invrep(self));
    char *pipe_str = g_strdup(""); // inicio nueva cadena
    GList *last =
        g_list_last(self->commands); // veo cual es el ultimo comando para poder
                                     // poner los | dentro de el bucle
    GList *current_command = g_list_first(self->commands);
    while (current_command != NULL) {
        char *scomm_str = scommand_to_string(
            current_command->data); // hago un string con el scommand actual
        char *temp = g_strconcat(
            pipe_str, scomm_str,
            NULL); // concateno, en un string temporal, lo que ya tenia con el
                   // nuevo scommand (temp para no tener memory leaks(me dijo un
                   // pajarito (chatgpt)))
        g_free(pipe_str); // libero la cadena anterior
        pipe_str = temp;  // apunto a la nueva cadena concatenada

        if (current_command != last) {
            temp = g_strconcat(pipe_str, " | ",
                               NULL); // el palo entre los scommand
            g_free(pipe_str);
            pipe_str = temp;
        } else if (current_command == last && !self->wait) {
            temp = g_strconcat(pipe_str, " & ",
                               NULL); // el &  al final de los scommand
            g_free(pipe_str);
            pipe_str = temp;
        }
        g_free(scomm_str); // libero la cadena temporal de scommand
        current_command = g_list_next(current_command);
    }

    g_assert(invrep(self) || strlen(pipe_str) > 0 || pipeline_get_wait(self));

    return pipe_str;
}
