import uasyncio as asyncio
import network
import json
import random
from mqtt import MQTTClient
from machine import Pin, I2C
import DHT20

# MQTT 配置
SERVER = "192.168.50.177"
PORT = 1883
CLIENT_ID = 'micropython-client-{id}'.format(id=random.getrandbits(8))
USERNAME = 'sea'
PASSWORD = 'lisihai525'
TOPIC_LED_STATE = "esp32/led/state"
TOPIC_LED_COMMAND = "esp32/led/command"
TOPIC_TEMPERATURE = "esp32/temperature"
TOPIC_HUMIDITY = "esp32/humidity"
topic = "esp32/led/command"

# Wi-Fi 
async def wifi_connect():
    ssid = 'yanfa_software'
    password = 'yanfa-123456'
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    wlan.scan()
    if not wlan.isconnected():
        print('connecting to network...')
        wlan.connect(ssid, password)
        while not wlan.isconnected():
            await asyncio.sleep(1)
    print('network config:', wlan.ifconfig())

# 初始化
pin25 = Pin(25, Pin.OUT)
i2c = I2C(scl=Pin(21), sda=Pin(22))
sensor = DHT20.DHT20(i2c)

# MQTT 回调函数
def mqtt_callback(topic, msg):
    print("R")
    #print("Received message on topic {}: {}".format(topic.decode(), msg.decode()))
    #print('received message %s on topic %s' % (msg, topic))
    print("Received '{payload}' from topic '{topic}'\n".format(
        payload = msg.decode(), topic = topic.decode()))
    if topic.decode() == TOPIC_LED_COMMAND:
        print("Handling LED command:", msg.decode())
        if msg.decode() == "ON":
            pin25.value(1)
            print("LED turned ON")
        elif msg.decode() == "OFF":
            pin25.value(0)
            print("LED turned OFF")
        else:
            print("Unknown command received:", msg.decode())
    else:
        print("Received message on topic {}, but no action taken.".format(topic.decode()))
 


# MQTT 连接
def connect_mqtt():
    client = MQTTClient(CLIENT_ID, SERVER, PORT, USERNAME, PASSWORD)
    client.set_callback(mqtt_callback)
    client.connect()  # 不是异步函数，所以不需要 await
    client.subscribe(TOPIC_LED_COMMAND)  # 订阅LED控制主题
    print('Connected to MQTT Broker "{server}"'.format(server=SERVER))
    return client

async def publish_messages(client):
    msg_count = 0
    while True:
        # 读取传感器数据
        sensor.read_dht20()
        temperature = sensor.dht20_temperature()
        humidity = sensor.dht20_humidity()

        # 发布传感器数据到 MQTT 主题
        client.publish(TOPIC_TEMPERATURE, str(temperature))
        client.publish(TOPIC_HUMIDITY, str(humidity))
        # 发布 LED 状态到 MQTT 主题
        #client.publish(TOPIC_LED_STATE, "ON" if pin25.value() else "OFF")

        print("Published temperature: {} °C, humidity: {} %".format(temperature, humidity))

        await asyncio.sleep(5)  # 每5秒发布一次数据

# 启动主循环
async def main():
    await wifi_connect()
    client = connect_mqtt()
    await asyncio.gather(
        publish_messages(client)
        
    )

# asyncio 运行
asyncio.run(main())
 