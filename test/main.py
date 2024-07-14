import paho.mqtt.client as mqtt
import time

client_id = 'python-mqtt-1'

def on_connect(client, userdata, flags, reason_code, properties):
    if reason_code == 0:
        print('Connected to MQTT Broker!')
    else:
        print('Failed to connect, return code %d\n', reason_code)

def on_message(client, userdata, msg):
    print(f"Received `{msg.payload.decode()}` from `{msg.topic}` topic")

client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
client.on_connect = on_connect
client.connect('192.168.0.121', 1883)
client.on_message = on_message

print('Connecting to the broker')

client.loop_start()
client.subscribe('retransmitter/temperature')

while(client.is_connected() == False):
    time.sleep(0.1)
print('Connected to the broker')

while(True):
    data_to_send = input('Enter data to send:')
    client.publish('retransmitter/temperature', data_to_send)