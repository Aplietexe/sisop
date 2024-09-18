#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "command.h"
#include "parser.h"
#include "parsing.h"

static scommand parse_scommand(Parser p) {
    assert(p != NULL);

    scommand result = scommand_new();
    arg_kind_t type;
    char *arg = NULL;

    while (!parser_at_eof(p) &&
           (arg = parser_next_argument(p, &type)) != NULL) {
        switch (type) {
        case ARG_NORMAL:
            scommand_push_back(result, arg);
            break;
        case ARG_INPUT:
            scommand_set_redir_in(result, arg);
            break;
        case ARG_OUTPUT:
            scommand_set_redir_out(result, arg);
            break;
        }
    }

    if (scommand_is_empty(result)) {
        result = scommand_destroy(result);
    }

    return result;
}

static bool move_parser(Parser p) { // true si !parser_at_eof(p)
    if (parser_at_eof(p)) {
        return false;
    }
    parser_skip_blanks(p);
    return !parser_at_eof(p);
}

pipeline parse_pipeline(Parser p) {
    assert(p != NULL);
    assert(!parser_at_eof(p));

    pipeline result = pipeline_new();

    bool error = false;
    char last = '0';
    while (!error && move_parser(p)) {
        bool pipe;
        parser_op_pipe(p, &pipe);
        if (pipe) {
            if (last != 'C') { // un | debe seguir a un comando
                error = true;
            }
            last = '|';
            continue;
        }

        bool background;
        parser_op_background(p, &background);
        if (background) {
            if (last != 'C') { // un & debe seguir a un comando
                error = true;
            }
            last = '&';
            pipeline_set_wait(result, false);
            break;
        }

        if (last == 'C') {
            break; // si no hay | ni & y ya hubo un comando, no hay nada más
        }

        scommand sc = parse_scommand(p);
        if (sc == NULL) {
            error = true;
            break;
        }
        pipeline_push_back(result, sc);
        last = 'C';
    }

    error = error || last == '|';

    if (move_parser(p)) {
        bool garbage;
        parser_garbage(p, &garbage);
        error = error || garbage;
    }

    if (error) {
        result = pipeline_destroy(result);
    }

    return result;
}
