import paho.mqtt.client as mqtt
import requests
import ssl

# Define MQTT and Authentication Server settings
BROKER = "yourbroker.com"
PORT = 8883
AUTH_SERVER = "https://authserver.com/api/login"  # Endpoint to authenticate users

# User credentials (replace with actual credentials)
USERNAME = "user"
PASSWORD = "password"

def authenticate_with_server(username, password):
    """Authenticate with the authentication server and get a token."""
    payload = {'username': username, 'password': password}
    response = requests.post(AUTH_SERVER, json=payload)
    
    # Check if the request was successful and return the token
    if response.status_code == 200:
        token = response.json().get("token")  # Assuming token is in response JSON
        return token
    else:
        print("Authentication failed:", response.status_code, response.text)
        return None

def on_connect(client, userdata, flags, rc):
    """Callback when connected to MQTT broker."""
    if rc == 0:
        print("Connected successfully to broker.")
        client.subscribe("chat/public/#")
    else:
        print("Failed to connect, return code %d\n", rc)

# Authenticate with the server and get a token
auth_token = authenticate_with_server(USERNAME, PASSWORD)

if auth_token:
    # Initialize MQTT client
    client = mqtt.Client()

    # Set TLS settings
    client.tls_set(ca_certs="/opt/homebrew/etc/mosquitto/certs/ca.crt", 
                   certfile="/opt/homebrew/etc/mosquitto/certs/server.crt", 
                   keyfile="/opt/homebrew/etc/mosquitto/certs/server.key", 
                   tls_version=ssl.PROTOCOL_TLSv1_2)
    client.tls_insecure_set(False)  # Set to True only for testing self-signed certs

    # Set credentials for MQTT (use the token as password)
    client.username_pw_set(USERNAME, auth_token)

    # Define callback functions
    client.on_connect = on_connect

    # Connect to the MQTT broker
    client.connect(BROKER, PORT, 60)

    # Start MQTT loop to process messages
    client.loop_forever()
else:
    print("Failed to authenticate with the server. Exiting.")
