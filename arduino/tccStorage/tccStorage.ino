#include <Ethernet.h>
#include <PubSubClient.h>

//---------W5500 RELATED---------
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  // MAC address
IPAddress ip(10, 9, 0, 36);                           // IP address that Arduino will assume
EthernetClient ethClient;

//---------MQTT RELATED---------
const char* mqtt_server = "10.9.0.114";  // MQTT broker/server IP address
const char* mqttTopic = "storageInfo";
const int mqttPort = 1883;
PubSubClient client(ethClient);

//---------Storage RELATED---------
const int switchsPin[] = { 36, 37, 38, 39 };
int state[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
const int switchNum = sizeof(switchsPin) / sizeof(switchsPin[0]);


void setup() 
{
  Serial.begin(115200);

  // sensors setup
  // Configures the switch pins as inputs and enables the internal pull-up resistors
  for (int i = 0; i < switchNum; i++) 
  {
    pinMode(switchsPin[i], INPUT_PULLUP);
  }

  // ethernet module setup
  setup_ethernet();

  // mqtt communication setup
  client.setServer(mqtt_server, 1883);
}

void setup_ethernet() 
{
  Serial.println("Begin Ethernet");

  Ethernet.init(10);
  if (Ethernet.begin(mac)) 
  {  // Dynamic IP setup
    Serial.println("DHCP OK!");
  } else 
  {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) 
    {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      while (true) 
      {
        delay(1);  // do nothing, no point running without Ethernet hardware
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) 
    {
      Serial.println("Ethernet cable is not connected.");
    }
  }
  delay(5000);


  // show the Ethernet configs of the arduino
  Serial.print("Local IP : ");
  Serial.println(Ethernet.localIP());
  Serial.print("Subnet Mask : ");
  Serial.println(Ethernet.subnetMask());
  Serial.print("Gateway IP : ");
  Serial.println(Ethernet.gatewayIP());
  Serial.print("DNS Server : ");
  Serial.println(Ethernet.dnsServerIP());

  Serial.println("Ethernet Successfully Initialized");
}

// function to reconnect to the topic
void reconnect() 
{
  while (!client.connected()) 
  {
    Serial.print("Attempting MQTT connection...");

    if (client.connect("ArduinoClient")) 
    {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

String readStorageUsage(int* sensorState, int switchNum) 
{
  // mocked storageResponse
  String result = "";
  for (int i = 0; i < switchNum; i++) 
  {
    result += String(sensorState[i]);
    if (i < switchNum - 1) 
    {
      result += ", ";
    }
  }

  // REMOVE WHEN PUT ALL SENSORS
  result += ", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0";
  return result;
}

void loop() 
{

  if (!client.connected()) 
  {
    reconnect();
  }

  bool stateChanged = false;
  for (int i = 0; i < switchNum; i++) 
  {
    int currentState = digitalRead(switchsPin[i]) == LOW ? 1 : 0;
    if (currentState != state[i]) 
    {
      stateChanged = true;
    }
    state[i] = currentState;
  }

  if (stateChanged) 
  {
    String result = readStorageUsage(state, switchNum);
    Serial.print("Message that will go to the topic [ ");
    Serial.print(mqttTopic);
    Serial.print("]: ");
    Serial.println(result);

    client.publish(mqttTopic, result.c_str());
    Serial.println("sent");
  }

  delay(150);
}