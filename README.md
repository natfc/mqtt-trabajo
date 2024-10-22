# mqtt-trabajo

every message is sent to the upv/SCI/martinnathan topic with qos 2
regular messages will have retain=true
connection/disconnection messages will have retain=false
last will messages (for unexpected disconnections) will have retain=false

to implement commands, we will need to implement a parser

the /salir command will use the mosquitto_disconnect method to disconnect gracefully

to implement the /privado command, we will make every user subscribe to the upv/SCI/martinnathan/username topic, using the /privado username command will send a message to that topic

to implement to /lista command, we will need to make every user send a heartbeat message to the upv/SCI/martinnathan/heartbeat topic every x seconds. Every client is subscribed to this topic, and updates their own hashmap (we use uthash) of connected users

TODO: use an encrypted/with auth server
TODO: add an extra feature