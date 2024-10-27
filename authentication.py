import paho.mqtt.client as mqtt
import ssl

# Define the broker settings
BROKER = "yourbroker.com"
PORT = 8883

# Define credentials
USERNAME = "user"
PASSWORD = "password"  # Or a JWT

def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    client.subscribe("chat/public/#")

# Initialize client
client = mqtt.Client()

# Set TLS settings
client.tls_set(ca_certs="/path/to/ca.crt", certfile="/path/to/client.crt",
               keyfile="/path/to/client.key", tls_version=ssl.PROTOCOL_TLSv1_2)
client.tls_insecure_set(False)  # True for self-signed certs, False for production

# Set credentials
client.username_pw_set(USERNAME, PASSWORD)

client.on_connect = on_connect

# Connect and loop forever
client.connect(BROKER, PORT, 60)
client.loop_forever()
