#ifndef PARSER_H
#define PARSER_H

#include "mpc.h"  // Include the mpc library header

// Function to initialize the parser
mpc_parser_t *initParser();

// Function to search for a specific tag in the AST
int searchAst(mpc_ast_t *node, const char *tag_to_find);

// Function to parse the input
int parseInput(char *input, mpc_parser_t *parser, mpc_result_t *r);

// // Function to handle commands based on the parsed output
// void handleCommand(mpc_ast_t *output);

// // Function to handle input and process commands
// int handleInput(char *input, mpc_parser_t *parser);

#endif // PARSER_H
