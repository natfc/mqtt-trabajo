
#ifndef PARSER_H
#define PARSER_H

#include <mosquitto.h>

// Function prototypes for command handling
void parse_command(struct mosquitto *mosq, const char *input);
void handle_exit(struct mosquitto *mosq);
void handle_private(struct mosquitto *mosq, const char *username, const char *message);
void handle_list();

#endif // PARSER_H