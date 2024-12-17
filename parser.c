/* Parses commands from raw input */
#include "mpc.h"
#include <editline/readline.h>
#include <stdio.h>
#include "parser.h"


mpc_parser_t *initParser() {
    /* Create parsers */
    mpc_parser_t *Input = mpc_new("input");
    mpc_parser_t *Slash = mpc_new("slash");
    mpc_parser_t *Salir = mpc_new("salir");
    mpc_parser_t *Privado = mpc_new("privado");
    mpc_parser_t *Lista = mpc_new("lista");
    mpc_parser_t *Id = mpc_new("id");
    mpc_parser_t *Message = mpc_new("message");
    mpc_parser_t *Command = mpc_new("command");

    /* Check if all parsers are created */
    if (!Input || !Slash || !Salir || !Privado || !Lista || !Id || !Message) {
        fprintf(stderr, "Error initializing parsers!\n");
        return NULL;
    }

    /* Define grammar with mpca_lang */
    mpc_err_t* err = mpca_lang(MPCA_LANG_DEFAULT, "                               \
    slash   : '/'; \
    message : /[a-zA-Z0-9.,!?;: '(){}<>-]+/ ; \
    salir   : \"salir\"; \
    privado : \"privado\"; \
    lista   : \"lista\"; \
    id      : /[a-zA-Z0-9]+/ ; \
    command : <slash> <salir> | \
              <slash> <privado> <id> <message> | \
              <slash> <lista> ; \
    input   : /^/ (<command> | <message>) ;   \
    ",
    Input, Slash, Salir, Privado, Lista, Message, Id, Command);

    if (err != NULL) {
        /* Print error if grammar fails */
        mpc_err_print(err);
        mpc_err_delete(err);
        mpc_cleanup(7, Input, Slash, Salir, Privado, Lista, Message, Id);
        return NULL;
    }

    return Input;  // Return the input parser
}

// Function to search for a specific tag in the AST
int searchAst(mpc_ast_t *node, const char *tag_to_find) {
    // Check if the current node's tag matches the tag we're looking for
    if (strstr(node->tag, tag_to_find) != NULL) {
        return 1; // Found the tag, return success
    }

    // Recursively search through all child nodes
    for (int i = 0; i < node->children_num; i++) {
        if (searchAst(node->children[i], tag_to_find)) {
            return 1; // Found the tag in a child node
        }
    }

    return 0; // Tag not found in this node or any children
}

int parseInput(char *input, mpc_parser_t *parser, mpc_result_t *r) {
    if (mpc_parse("<stdin>", input, parser, r)) {
        return 1;
    } else {
        /* Otherwise print and delete the Error */
        printf("0");
        mpc_err_print(r->error);
        mpc_err_delete(r->error);
        return 0;
    }
}


