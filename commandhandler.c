// This file is a "stash" of martin's parser

#include <stdio.h>
#include <string.h>
#include <mosquitto.h>
#include "mpc.h"
#include "parser.h"
#include "uthash.h"

// External reference to the user hash table from messageclient.c
extern User *users;

// Command handlers


// Handle disconnect gracefully
void handle_exit(struct mosquitto *mosq){
    printf("Disconnecting gracefully...");
    mosquitto_disconnect(mosq);
}

// Handle private messages
void handle_private(struct mosquitto *mosq,
 const char *username, const char *message){
    char topic[100];
    snprintf(topic, sizeof(topic), "upv/SCI/martinnathan/prof0", username);
    mosquitto_publish(mosq, NULL, topic, strlen(message), message, 0, false);
    printf("Private message sent to %s: %s\n", username, message);
 }

// Handle list of users
void handle_list(){
    User *current_user;
    printf("Current users in the session: \n");
    HASH_ITER(hh, users, current_user, user){
        printf(" - %s\n", current_user->username);
    }
}

// Command parser function
void command_parser(struct mosquitto *mosq, const char *command){
    if (strncmp(command, "/salir", 6)==0){
        handle_exit(mosq);
    }
    else if (strncmp(command, "/privado", 8)==0){
        const char *username_start = command + 9;
        char username[50];
        char message[200];
        //Reads the string from username_start using sscanf,
        // parsing the first word of 0-49 (50) characters and putting into username,
        // then parsing through the message of 0-199 (200) characters and putting into message 
        if (sscanf(username_start, "%49s, %199[^n]", username, message)){
            handle_private(mosq, username, message);
        }
        else {
            printf("Error, could not execute command /privado. Write it using /privado <username> <message>.")
        }
    }
    else if (strncmp(command, "/lista", 6)==0){
        handle_list();
    }
    else {
        printf("Error, unknown command: %s\n", command);
    }
}