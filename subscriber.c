#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
void on_connect(struct mosquitto *mosq, void *obj, int reason_code)
{
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
void on_message(struct mosquitto *mosq, void *obj,
                const struct mosquitto_message *msg)
{
    printf("%s %d %s\n", msg->topic, msg->qos, (char *)msg->payload);
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
    rc = mosquitto_subscribe(mosq, NULL, "upv/SCI/prof", 0);
    if (rc != MOSQ_ERR_SUCCESS)
    {
        fprintf(stderr, "Error subscribing: %s\n", mosquitto_strerror(rc));
    }
    /* main loop */
    time_t last_time = 0;
    while (1)
    {
        time_t t = time(NULL);
        if (t > last_time + 10)
        {
            printf("Haciendo cosas...\n");
            last_time = t;
        }
    }
    mosquitto_disconnect(mosq);
    mosquitto_lib_cleanup();
    return 0;
}