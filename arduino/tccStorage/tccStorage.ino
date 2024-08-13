#include <Ethernet.h>
#include <PubSubClient.h>
#include <Keypad_Matrix.h>

//----------------W5500 RELATED----------------
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  // MAC address
IPAddress ip(10, 9, 0, 36);                           // IP address that Arduino will assume
EthernetClient ethClient;

//-----------------MQTT RELATED----------------
const char* mqtt_server = "10.9.0.114";  // MQTT broker/server IP address
const char* mqttTopic = "storageInfo";
const int mqttPort = 1883;
PubSubClient client(ethClient);

//---------------Storage RELATED---------------
const byte ROWS = 5;
const byte COLS = 5;
const byte rowPins[ROWS] = {42, 38, 34, 30, 26}; //connect to the row pinouts of the keypad
const byte colPins[COLS] = {44, 40, 36, 32, 28}; //connect to the column pinouts of the keypad
// Create the storage model
const char keys[ROWS][COLS] = {
  {'1', '2', '3', '4', '5'},
  {'6', '7', '8', '9', 'a'},
  {'b', 'c', 'd', 'e', 'f'},
  {'g', 'h', 'i', 'j', 'k'},
  {'l', 'm', 'n', 'o', 'p'}
};
Keypad_Matrix storageModel = Keypad_Matrix(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
byte keyStates[ROWS * COLS] = {0}; // Array to store the state of each key
byte previousKeyStates[ROWS * COLS] = {0}; // Array to store the previous state of each key

// Using averaging
const int numReadings = 5; // Number of readings to average
int readings[ROWS * COLS][numReadings]; // Array to store the readings
int currentReadNumber = 0; // Index of the current reading

void setup()
{
  Serial.begin(115200);

  // ethernet module setup
  setup_ethernet();

  // mqtt communication setup
  client.setServer(mqtt_server, 1883);

  //storage setup
  storageModel.begin();

  // Initialize the readings array
  for (int i = 0; i < ROWS * COLS; i++) {
    for (int j = 0; j < numReadings; j++) {
      readings[i][j] = 0;
    }
  }
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

String readStorageUsage()
{
  // Check the state of each key and update the array
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLS; j++) {
      char key = keys[i][j];
      int keyIndex = i * COLS + j;
      readings[keyIndex][currentReadNumber] = storageModel.isKeyDown(key) ? 1 : 0;
    }
  }

  currentReadNumber++;
  if (currentReadNumber >= numReadings) {
    currentReadNumber = 0;
  }

  // Average the readings and update keyStates
  for (int i = 0; i < ROWS * COLS; i++) {
    int sum = 0;
    for (int j = 0; j < numReadings; j++) {
      sum += readings[i][j];
    }
    keyStates[i] = (sum > numReadings / 2) ? 1 : 0;
  }

  // Convert the array to a string and return it
  String keyStateStr = "";
  for (int i = 0; i < 25; i++) {
    keyStateStr += String(keyStates[i]);
    if (i < 24) {
      keyStateStr += ",";
    }
  }
  return keyStateStr;
}

bool stateChanged() {
  for (int i = 0; i < 25; i++) {
    if (keyStates[i] != previousKeyStates[i]) {
      return true;
    }
  }
  return false;
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  else {
    storageModel.scan();
    String result = readStorageUsage();
    if (stateChanged()) {
      Serial.print("Message that will go to the topic [ ");
      Serial.print(mqttTopic);
      Serial.print("]: ");
      Serial.println(result);

      client.publish(mqttTopic, result.c_str());
      Serial.println("sent");

      // Update previousKeyStates to current keyStates
      for (int i = 0; i < 25; i++) {
        previousKeyStates[i] = keyStates[i];
      }
    }
  delay(150);
  }
  delay(150);
}