#include <SPI.h> //allows an SPI communication and lets the ethernet shield and arduino board communicate 
#include <Ethernet.h> //internet connection 
#include "DHT.h" // reads the infro from the temp sensor 
#define DHTPIN 2 // pin 2 connection 
#define DHTTYPE DHT11 //sensor model 
DHT dht(DHTPIN, DHTTYPE); //this is what links sensor and pin 
String apiKey = "N3NBDXBWMLVNH82U"; // allows the data from the adruino board to be uploaded onto the cloud (thingspeak) 
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; //Default MAC address used 

EthernetClient client; // this is what sends the data over the internet 
void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; //waits for the serial connection 
  }
  dht.begin();// starts the temp sensor 

  Serial.println("Currently trying to connect to network..."); // prints my message showing the connection has started 
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to aquire the IP address"); // error message if connection fails 
    while (true);
  }
  delay(1000);
}
void loop() {
  float venuetemp = dht.readTemperature(); // reads temp from sensor and stores the value as a decimal 

  if (isnan(venuetemp)) {
    Serial.println("There is a sensor error"); //error is sensor does fail 
    delay(20000);// this is how long the sytem waits its set to 20 seconds 
    return;
  }

  int tempstatus; // this part of code is where the edge AI component is added 

  if (venuetemp < 10) {
    tempstatus = 1; // the temp is currently Cold
  } else if (venuetemp <= 20) {
    tempstatus = 2; // the temp is currently Moderate 
  } else {
    tempstatus = 3; // the temp is currently Hot
  }
  Serial.print("Current Temperature: ");
  Serial.print(venuetemp);            //shows messages regarding temp 
  Serial.print(" Degrees Celcius and the Status: ");
  Serial.println(tempstatus);

  if (client.connect("api.thingspeak.com", 80)) { // connection to thingspeak
    String postStr = apiKey;
    postStr += "&field1="; // sends the temp reading to field 1 in thingspeak
    postStr += String(venuetemp);
    postStr += "&field2="; // sends temp reading to field 2 in thingspeak 
    postStr += String(tempstatus);
    postStr += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n"); // tells cloud im sending data
    client.print("Host: api.thingspeak.com\n");// destination 
    client.print("Connection: close\n"); // closes connection after the temp reading is sent 
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);

    Serial.println("Data sent to ThingSpeak"); // is shown the confirm the succesful upload 
  }

  client.stop(); // stops the internet connection
  delay(20000); // Data is sent to ThingSpeak every 20 seconds 
}


