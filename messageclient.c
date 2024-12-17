#include "mpc.h"
#include "parser.h"
#include "uthash.h"
#include <mosquitto.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

const int MIN_TIME_BETWEEN_HASHTABLE_ITERATIONS = 20;
#define TABLE_SIZE 10
char username[50];
int heartbeat_interval = 5;

// Define the User structure
typedef struct {
  char username[50];  // Key: unique username
  time_t last_active; // Value: timestamp of last activity
  UT_hash_handle hh;  // Makes this structure hashable
} User;

User *users = NULL, *user, *current_user; // Initialize the hash table for users
time_t timeout = 10;                      // Set the timeout for 10 seconds
time_t current_time;                      // Set the current time

void displayConnectedUsers() {
  User *current_user, *tmp;
  int user_count = 0;

  printf("Currently connected users:\n");
  HASH_ITER(hh, users, current_user, tmp) {
    user_count++;
    printf("Username: %s, Last Active: %ld seconds ago\n", current_user->username,
           (long)(time(NULL) - current_user->last_active));
  }

  if (user_count == 0) {
    printf("No active users.\n");
  }
}

// Add or update a user in the hash table
void addOrUpdateUser(const char *username) {
  User *user;

  // Try to find an existing user
  HASH_FIND_STR(users, username, user);
  if (user == NULL) {
    // User not found, create a new user
    user = (User *)malloc(sizeof(User));
    strncpy(user->username, username, sizeof(user->username));
    user->last_active = time(NULL);      // Set current timestamp
    HASH_ADD_STR(users, username, user); // Add user to hash table
  } else {
    // User found, update last active time
    user->last_active = time(NULL);
  }
}

// Function to send a heartbeat message every x seconds
void *heartbeat_thread_func(void *mosq_ptr) {
  struct mosquitto *mosq = (struct mosquitto *)mosq_ptr;
  char heartbeat_message[100];

  while (1) {
    snprintf(heartbeat_message, sizeof(heartbeat_message), "Heartbeat from %s",
             username);

    // Publish the heartbeat message
    mosquitto_publish(mosq, NULL, "upv/SCI/nathanmartin/heartbeat",
                      strlen(heartbeat_message), heartbeat_message, 0, false);

    // Sleep for the heartbeat interval (in seconds)
    sleep(heartbeat_interval);
  }

  return NULL; // Thread exit
}

void on_connect(struct mosquitto *mosq, void *obj, int reason_code) {
  if (reason_code != 0) {
    fprintf(stderr, "Failed to connect, reason code: %s\n",
            mosquitto_connack_string(reason_code));
    return;
  }


  // Add or update the user in the hash table with their username
  addOrUpdateUser(username);

  printf("Welcome, %s! You are now connected.\n", username);
  char connection_notice[100];
  sprintf(connection_notice, "%s has connected.", username);
  int rc = mosquitto_publish(mosq, NULL, "upv/SCI/nathanmartin/main",
                               strlen(connection_notice), connection_notice, 2, false);
    if (rc != MOSQ_ERR_SUCCESS) {
      fprintf(stderr, "Error publishing: %s\n", mosquitto_strerror(rc));
    }
}
void on_disconnect(struct mosquitto *mosq, void *obj, int reason_code) {
}
void on_subscribe(struct mosquitto *mosq, void *obj, int mid, int qos_count,
                  const int *granted_qos) {
  for (int i = 0; i < qos_count; i++) {

  }
}
void on_regular_message(struct mosquitto *mosq, void *obj,
                        const struct mosquitto_message *msg) {
  printf("%s\n", (char *)msg->payload);
}

void on_heartbeat_message(struct mosquitto *mosq, void *obj,
                          const struct mosquitto_message *msg) {
  // update the hash table
  char user_id[50];
  sscanf(
      msg->payload, "Heartbeat from %49s",
      user_id); // Finds the username from the message contents and extracts it
  addOrUpdateUser(
      user_id); // Extracts the user id from the topic using sscanf, and then
                // updates the hash table using addOrUpdateUser
  // printf("Heartbeat received from %s and processed by on_heartbeat_message
  // \n", user_id);
}

void on_message(struct mosquitto *mosq, void *obj,
                const struct mosquitto_message *msg) {
  // filter whether it is a regular message or a heartbeat message
  // checks if the topic of the message is the same as the topic for a heartbeat
  // message
  if (strcmp(msg->topic, "upv/SCI/nathanmartin/heartbeat") == 0) {
    on_heartbeat_message(mosq, obj, msg);
  } else
  // if it is not a heartbeat message it is a regular message
  {
    on_regular_message(mosq, obj, msg);
  }
}

// Retrieve a user's last active time
time_t getLastActive(const char *username) {
  User *user;
  HASH_FIND_STR(users, username, user); // Find user by username

  if (user != NULL) {
    return user->last_active; // Return the last active timestamp
  } else {
    return -1; // User not found
  }
}

// Delete a user from the hash table
void deleteUser(const char *username) {
  User *user;
  HASH_FIND_STR(users, username, user); // Find user by username
  if (user) {
    HASH_DEL(users, user); // Delete the user from the hash table
    free(user);            // Free the memory allocated for the user
  }
}

void iterateHashTable() {
  User *current_user, *tmp;
  current_time = time(NULL); // Update the current time
  // Iterate over the hash table safely
  HASH_ITER(hh, users, current_user, tmp) {
    // Check if the user has exceeded the timeout
    if (difftime(current_time, current_user->last_active) > timeout) {
      // printf("Removing user: %s (inactive)\n", current_user->username);
      HASH_DEL(users, current_user); // Delete the entry from the hash table
      free(current_user);            // Free the memory allocated for the user
    } else {
      // User is still active, print their details
            //  current_user->last_active);
    }
  }
}

void handleCommand(mpc_ast_t *output, struct mosquitto *mosq, char *line) {
  if (searchAst(output, "salir") || strstr(output->tag, "salir") != NULL) {
    // printf("Command recognized: /salir\n");
    char disconnection_notice[100];
    sprintf(disconnection_notice, "%s has disconnected.", username);
    int rc = mosquitto_publish(mosq, NULL, "upv/SCI/nathanmartin/main",
                               strlen(disconnection_notice), disconnection_notice, 2, false);
    sleep(1);
    if (rc != MOSQ_ERR_SUCCESS) {
      fprintf(stderr, "Error publishing: %s\n", mosquitto_strerror(rc));
    }
    rc = mosquitto_disconnect(mosq);
    if (rc != MOSQ_ERR_SUCCESS) {
      // mosquitto_destroy(mosq);
      fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
    } else {
      mosquitto_destroy(mosq);
      mosquitto_lib_cleanup();
      exit(EXIT_SUCCESS);
    }
    

    // mpc_ast_print(output);
    // Handle exit logic here, e.g., break the loop or set a flag to exit.
  } else if (searchAst(output, "privado") ||
             strstr(output->tag, "privado") != NULL) {
    mpc_ast_t *command_node = output->children[1];


    // mpc_ast_print(command_node);

    if (command_node) {
      // Extracting the id field from the "privado" command

      mpc_ast_t *id_node = command_node->children[2];
      mpc_ast_t *message_node = command_node->children[3];
      if (id_node && message_node) {
        // The "contents" field contains the actual text value of the node
        char *id = id_node->contents;
        char *message = message_node->contents;

        // publish the message to the right channel
        char privado_topic[100];
        char private_message_prefix[100];
        char formatted_message[512];
        snprintf(private_message_prefix, 100,"%s (private): ", username);
        sprintf(formatted_message, "%s%s", private_message_prefix, message);
        snprintf(privado_topic, sizeof(privado_topic),
                 "upv/SCI/nathanmartin/%s", id);
        int rc = mosquitto_publish(mosq, NULL, privado_topic, strlen(formatted_message),
                                   formatted_message, 2, false);
        if (rc != MOSQ_ERR_SUCCESS) {
          fprintf(stderr, "Error publishing: %s\n", mosquitto_strerror(rc));
        } 
      }
    }
    // Handle private message logic here.
  } else if (searchAst(output, "lista") ||
             strstr(output->tag, "lista") != NULL) {

    // mpc_ast_print(output);
    displayConnectedUsers();
    // Handle listing logic here.
  } else {
    // Handle general message

    // char* message = output->children[1]->children[1]->contents;
    // Remove newline character at the end of the line
    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n') {
      line[len - 1] = '\0';
    }
    char message_prefix[100];
    char formatted_message[512];
    snprintf(message_prefix, 100,"%s: ", username);
    sprintf(formatted_message, "%s%s", message_prefix, line);
    int rc = mosquitto_publish(mosq, NULL, "upv/SCI/nathanmartin/main",
                               strlen(formatted_message), formatted_message, 2, false);
    if (rc != MOSQ_ERR_SUCCESS) {
      fprintf(stderr, "Error publishing: %s\n", mosquitto_strerror(rc));
    }
  }
}

int handleInput(char *input, mpc_parser_t *parser, struct mosquitto *mosq) {
  mpc_result_t r;
  int result = parseInput(input, parser, &r);
  if (result) {
    handleCommand(r.output, mosq, input);
    mpc_ast_delete(r.output);
  }
}

int main(int argc, char *argv[]) {
  struct mosquitto *mosq;
  int rc;
  pthread_t heartbeat_thread;

  mosquitto_lib_init();

  // Prompt for username before proceeding to the MQTT loop
  printf("Please enter your username (only letters or numbers): ");
  fgets(username, sizeof(username), stdin);

  // Remove newline character if present
  size_t len = strlen(username);
  if (len > 0 && username[len - 1] == '\n') {
    username[len - 1] = '\0';
  }

  if (strlen(username) == 0) {
    fprintf(stderr, "Error: Username cannot be empty.\n");
    return 1; // Exit if no valid username is provided
  }

  mosq = mosquitto_new(username, false, NULL);
  if (mosq == NULL) {
    fprintf(stderr, "Error: Out of memory.\n");
    return 1;
  }
  mosquitto_connect_callback_set(mosq, on_connect);
  mosquitto_disconnect_callback_set(mosq, on_disconnect);
  mosquitto_subscribe_callback_set(mosq, on_subscribe);
  mosquitto_message_callback_set(mosq, on_message);
  /* Run the network loop in a background thread, this call returns quickly. */
  rc = mosquitto_loop_start(mosq);
  if (rc != MOSQ_ERR_SUCCESS) {
    mosquitto_destroy(mosq);
    fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
    return 1;
  }

  char will[128];
  /* Set a will, before calling mosquitto_connect. */
    snprintf(will, sizeof(will), "%s has disconnected unexpectedly.",
           username);
    rc = mosquitto_will_set(mosq, "upv/SCI/nathanmartin/main", strlen(will), will, 2, false);

  /* Connect to test.mosquitto.org on port 1883, with a keepalive of 60 seconds.
   */
  rc = mosquitto_connect(mosq, "test.mosquitto.org", 1883, 60);
  if (rc != MOSQ_ERR_SUCCESS) {
    mosquitto_destroy(mosq);
    fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
    return 1;
  }
  /* Subscribe to the main topic */
  rc = mosquitto_subscribe(mosq, NULL, "upv/SCI/nathanmartin/main", 2);
  if (rc != MOSQ_ERR_SUCCESS) {
    fprintf(stderr, "Error subscribing: %s\n", mosquitto_strerror(rc));
  }
  /* Subscribe to the heartbeat topic */
  rc = mosquitto_subscribe(mosq, NULL, "upv/SCI/nathanmartin/heartbeat", 1);
  if (rc != MOSQ_ERR_SUCCESS) {
    fprintf(stderr, "Error subscribing: %s\n", mosquitto_strerror(rc));
  }

  /* Subscribe to the personal topic */
  char privado_topic[100];
  snprintf(privado_topic, sizeof(privado_topic), "upv/SCI/nathanmartin/%s",
           username);
  rc = mosquitto_subscribe(mosq, NULL, privado_topic, 2);
  if (rc != MOSQ_ERR_SUCCESS) {
    fprintf(stderr, "Error subscribing: %s\n", mosquitto_strerror(rc));
  }

  if (pthread_create(&heartbeat_thread, NULL, heartbeat_thread_func,
                     (void *)mosq) != 0) {
    fprintf(stderr, "Error: Failed to create heartbeat thread.\n");
    return 1;
  }

  char line[2048];
  time_t last_hashtable_iteration = 0;

  mpc_parser_t *parser;
  /* Initialize parser */
  parser = initParser();
  if (!parser) {
    return 1; // Exit if parser failed to initialize
  }
  /* main loop */
  while (1) {
    // get input from user
    if (fgets(line, 100, stdin) == NULL)
      break;
    else {
      // parse the commands from the user
      handleInput(line, parser, mosq);
      if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Error publishing: %s\n", mosquitto_strerror(rc));
      }
    }
    // every y seconds, iterate through the hash table, and deleteUser() if the
    // last_active time_t is too old.

    if (difftime(time(NULL), last_hashtable_iteration) >
        MIN_TIME_BETWEEN_HASHTABLE_ITERATIONS) {
      // iterate through the hash table
      iterateHashTable();
    }
  }
}