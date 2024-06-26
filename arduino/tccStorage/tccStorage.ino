#include <Ethernet.h>
#include <PubSubClient.h>

//---------W5500 RELATED---------
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};  // MAC address
IPAddress ip(10, 9, 0, 36);  // IP address that Arduino will assume
EthernetClient ethClient;



//---------MQTT RELATED---------
const char* mqtt_server = "10.9.0.114"; // MQTT broker/server IP address
const char* mqttTopic = "storageInfo";
const int mqttPort = 1883;
PubSubClient client(ethClient);

//---------Storage RELATED---------
int numFileiras = 5;
int numColunas = 5;
int fileira1[] = {0, 0, 0, 0, 0};
int fileira2[] = {0, 0, 0, 0, 0};
int fileira3[] = {0, 0, 0, 0, 0};
int fileira4[] = {0, 0, 0, 0, 0};
int fileira5[] = {0, 0, 0, 0, 0};

void setup()
{
  Serial.begin(115200);

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
  { // Dynamic IP setup
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
        delay(1); // do nothing, no point running without Ethernet hardware
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
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ArduinoClient"))
    {
      Serial.println("connected");
    } else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

String readStorageUsage()
{
  // mocked storageResponse
  String result = "";
  String mockedStorageInfo = "0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0";
  return mockedStorageInfo;
}

void loop()
{

  if (!client.connected())
  {
    reconnect();
  }
  String result = readStorageUsage();
  Serial.print("Message that will go to the topic [ ");
  Serial.print(mqttTopic);
  Serial.print("]: ");
  Serial.println(result);

  client.publish(mqttTopic, result.c_str());
  Serial.println("sent");
  delay(10000);

}