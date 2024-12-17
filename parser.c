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
        /* On success print and delete the AST */
        // mpc_ast_t *ast = r->output;  // Cast the void* to mpc_ast_t*
        // mpc_ast_print(r->output);
        // // printf("Printing output->tag: %s\n", ast->children[0]->tag);
        // char *tags[] = {"salir", "privado", "lista"};
        // for (int i = 0; i < 3; i++){
        //     int found = searchAst(ast, tags[i]);
        //     if (found){printf("%s found", tags[i]);}
        //     // printf("%d", found);
        // }
        // mpc_ast_delete(r->output);
        // printf("1");
        return 1;
    } else {
        /* Otherwise print and delete the Error */
        printf("0");
        mpc_err_print(r->error);
        mpc_err_delete(r->error);
        return 0;
    }
}

// void handleCommand(mpc_ast_t *output) {
//     if (searchAst(output, "salir") || strstr(output->tag, "salir") != NULL) {
//         printf("Command recognized: /salir\n");
//         // Handle exit logic here, e.g., break the loop or set a flag to exit.
//     } else if (searchAst(output, "privado") || strstr(output->tag, "privado") != NULL) {
//         // Extract the ID and message
//         // char *id = output->children[2]->contents; // ID is the third child
//         // char *message = output->children[3]->contents; // Message is the fourth child
//         printf("Private message \n");
//         // Handle private message logic here.
//     } else if (searchAst(output, "lista") || strstr(output->tag, "lista") != NULL) {
//         printf("Command recognized: /lista\n");
//         // Handle listing logic here.
//     } else {
//         // Handle general message
//         printf("Message received: %s\n", output->contents);
//     }
//     // mpc_ast_delete(output);
// }

// int handleInput(char *input, mpc_parser_t *parser) {
//     mpc_result_t r;
//     int result = parseInput(input, parser, &r);
//     if (result) {
//         handleCommand(r.output);
//         mpc_ast_delete(r.output);
//     }
// }

#ifdef TEST_PARSER
int main() {
    // char input[] = "/salir";
    mpc_parser_t *parser;

    /* Initialize parser */
    parser = initParser();
    if (!parser) {
        return 1;  // Exit if parser failed to initialize
    }

    /* Parse the input */
    // int success = parse_input(input, parser);
    
    // if (!success) {
    // }

    /* Clean up all parsers after parsing: DO NOT, gives a segmentation error */
    // mpc_cleanup(7, parser);

    // Test inputs
    char *tests[] = {
        "/salir",            // Test for exit command
        "Hello, World!",    // Test for a simple message
        "/privado john Hi there!", // Test for private message command
        "/lista",           // Test for list command
    };

    // Execute tests
    for (int i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
        printf("Testing input: '%s'\n", tests[i]);
        mpc_result_t r;
        // parseInput(tests[i], parser, r);  // Parse the test input
        handleInput(tests[i], parser);
        printf("\n");  // Newline for better readability
    }

    return 0;
}
#endif
