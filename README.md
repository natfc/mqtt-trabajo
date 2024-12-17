# mqtt-trabajo

Every message is sent to the upv/SCI/nathanmartin topic with qos 2

To run, run messageclient

the /salir command will use the mosquitto_disconnect method to disconnect gracefully

to implement the /privado command, we will make every user subscribe to the upv/SCI/martinnathan/username topic, using the /privado username command will send a message to that topic

to implement to /lista command, we will need to make every user send a heartbeat message to the upv/SCI/martinnathan/heartbeat topic every x seconds. Every client is subscribed to this topic, and updates their own hashmap (we use uthash) of connected users

TODO: use an encrypted/with auth server
TODO: add an extra feature

## Credits

This project uses code licensed under the BSD license by [Daniel Holden](https://github.com/orangeduck) (the mpc parser).  
Copyright (c) 2013, Daniel Holden. All rights reserved.

This project uses code licensed under a custom MIT-style license by [Troy D. Hanson](https://troydhanson.github.io/uthash/).  
Copyright (c) 2005-2022, Troy D. Hanson. All rights reserved.
