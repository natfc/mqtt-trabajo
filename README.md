# mqtt-trabajo

This is a messaging application using the Mosquitto message broker. At the moment, it uses the Mosquitto public test server (test.mosquitto.org) on the port 1883 (uncrypted, unauthenticated).

To compile, run `gcc -std=c99 -Wall messageclient.c parser.c mpc.c -ledit -lmosquitto -lm -o messageclient`.

To run, run `./messageclient`

### Commands

- `/lista` prints a list of all connected users
- `/privado id message` sends a private message to a user whose id is `id`
- `/salir` disconnects you gracefully

### WIP

Add a self-hosted server with encryption and authentication.

## Credits

This project uses code licensed under the BSD license by [Daniel Holden](https://github.com/orangeduck) (the mpc parser).  
Copyright (c) 2013, Daniel Holden. All rights reserved.

This project uses code licensed under a custom MIT-style license by [Troy D. Hanson](https://troydhanson.github.io/uthash/).  
Copyright (c) 2005-2022, Troy D. Hanson. All rights reserved.
