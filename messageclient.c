#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "uthash.h"

const int MIN_TIME_BETWEEN_HASHTABLE_ITERATIONS = 20;
#define TABLE_SIZE 10

void on_connect(struct mosquitto *mosq, void *obj, int reason_code)
{
    //
    printf("on_connect: %s\n", mosquitto_connack_string(reason_code));
}
void on_disconnect(struct mosquitto *mosq, void *obj, int reason_code)
{
    printf("on_disconnect: %s\n", mosquitto_connack_string(reason_code));
}
void on_subscribe(struct mosquitto *mosq, void *obj,
                  int mid, int qos_count, const int *granted_qos)
{
    for (int i = 0; i < qos_count; i++)
    {
        printf("on_subscribe: %d:granted qos = %d\n", i, granted_qos[i]);
    }
}
void on_regular_message(struct mosquitto *mosq, void *obj,
                        const struct mosquitto_message *msg)
{
    printf("%s %d %s\n", msg->topic, msg->qos, (char *)msg->payload);
}

void on_heartbeat_message(struct mosquitto *mosq, void *obj,
                          const struct mosquitto_message *msg)
{
    // TODO update the hash table

}


void on_message(struct mosquitto *mosq, void *obj,
                const struct mosquitto_message *msg)
{
    // TODO filter whether it is a regular message or a heartbeat message
    if (1)
    {
        on_regular_message(mosq, obj, msg);
    }
    else
    {
        on_heartbeat_message(mosq, obj, msg);
    }
}

// Define the User structure
typedef struct
{
    char username[50];  // Key: unique username
    time_t last_active; // Value: timestamp of last activity
    UT_hash_handle hh;  // Makes this structure hashable
} User;

User *users = NULL, *user, *current_user; // Initialize the hash table for users
time_t timeout = 10; // Set the timeout for 10 seconds
time_t current_time; // Set the current time

// Add or update a user in the hash table
void addOrUpdateUser(const char *username)
{
    User *user;

    // Try to find an existing user
    HASH_FIND_STR(users, username, user);
    if (user == NULL)
    {   
        // User not found, create a new user
        user = (User *)malloc(sizeof(User));
        strncpy(user->username, username, sizeof(user->username));
        user->last_active = time(NULL);      // Set current timestamp
        HASH_ADD_STR(users, username, user); // Add user to hash table
    }
    else
    {
        // User found, update last active time
        user->last_active = time(NULL);
    }
}

// Retrieve a user's last active time
time_t getLastActive(const char *username)
{
    User *user;
    HASH_FIND_STR(users, username, user); // Find user by username

    if (user != NULL)
    {
        return user->last_active; // Return the last active timestamp
    }
    else
    {
        return -1; // User not found
    }
}

// Delete a user from the hash table
void deleteUser(const char *username)
{
    User *user;
    HASH_FIND_STR(users, username, user); // Find user by username
    if (user)
    {
        HASH_DEL(users, user); // Delete the user from the hash table
        free(user);            // Free the memory allocated for the user
    }
}

// Iterate through the hash table, deleting users that haven't sent a heartbeat for more than the set delay
void iterateHashTable(hh, users, user, current_user)
{   // Check if the user has exceeded the timeout
    if (difftime(current_time, getLastActive(user))>timeout){
        printf("Removing user: %s (inactive)\n", user->username);
        deleteUser(user) // Delete the entry from the hash table
        free((char *) user->username); // Free the key string, the entry is freed in deleteUser()
    } else {
        // User is still active, print their details
        printf("Active user: %s, Last Heartbeat: %ld\n", user->username, user->last_active)
    }
}

int main(int argc, char *argv[])
{
    struct mosquitto *mosq;
    int rc;
    mosquitto_lib_init();
    mosq = mosquitto_new(NULL, true, NULL);
    if (mosq == NULL)
    {
        fprintf(stderr, "Error: Out of memory.\n");
        return 1;
    }
    mosquitto_connect_callback_set(mosq, on_connect);
    mosquitto_disconnect_callback_set(mosq, on_disconnect);
    mosquitto_subscribe_callback_set(mosq, on_subscribe);
    mosquitto_message_callback_set(mosq, on_message);
    /* Run the network loop in a background thread, this call returns quickly. */
    rc = mosquitto_loop_start(mosq);
    if (rc != MOSQ_ERR_SUCCESS)
    {
        mosquitto_destroy(mosq);
        fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
        return 1;
    }
    /* Connect to test.mosquitto.org on port 1883, with a keepalive of 60 seconds. */
    rc = mosquitto_connect(mosq, "test.mosquitto.org", 1883, 60);
    if (rc != MOSQ_ERR_SUCCESS)
    {
        mosquitto_destroy(mosq);
        fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
        return 1;
    }
    /* Subscribe to the main topic */
    rc = mosquitto_subscribe(mosq, NULL, "upv/SCI/martinnathan/main", 0);
    if (rc != MOSQ_ERR_SUCCESS)
    {
        fprintf(stderr, "Error subscribing: %s\n", mosquitto_strerror(rc));
    }
    /* Subscribe to the heartbeat topic*/
    rc = mosquitto_subscribe(mosq, NULL, "upv/SCI/martinnathan/heartbeat", 0);
    if (rc != MOSQ_ERR_SUCCESS)
    {
        fprintf(stderr, "Error subscribing: %s\n", mosquitto_strerror(rc));
    }

    char line[2048];
    time_t last_hashtable_iteration = 0;
    /* main loop */
    while (1)
    {
        // get input from user
        if (fgets(line, 100, stdin) == NULL)
            break;
        else
        {
            // TODO parse the commands from the user
            int rc = mosquitto_publish(mosq, NULL, "upv/SCI/martinnathan/main",
                                       strlen(line), line, 0, false);
            if (rc != MOSQ_ERR_SUCCESS)
            {
                fprintf(stderr, "Error publishing: %s\n", mosquitto_strerror(rc));
            }
        }

        // every y seconds, iterate through the hash table, and deleteUser() if the last_active time_t is too old.

        if (difftime(time(NULL), last_hashtable_iteration) > MIN_TIME_BETWEEN_HASHTABLE_ITERATIONS)
        {
            // iterate through the hash table
            iterateHashTable();
        }
    }
}