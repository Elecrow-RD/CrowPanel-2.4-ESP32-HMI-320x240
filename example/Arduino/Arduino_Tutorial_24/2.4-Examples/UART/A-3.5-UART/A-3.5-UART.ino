
unsigned char buffer[256]; // buffer array for data recieve over serial port
int count = 0;   // counter for buffer array
void clearBufferArray()              // function to clear buffer array
{
  for (int i = 0; i < count; i++)
  {
    buffer[i] = NULL; // clear all index of array with command NULL
  }
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin( 9600 ); /*初始化串口*/
  Serial2.begin( 9600 ); /*初始化串口2*/
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial2.available())              // if date is comming from softwareserial port ==> data is comming from SoftSerial shield
  {
    while (Serial2.available())         // reading data into char array
    {
      buffer[count++] = Serial2.read();   // writing data into array
      if (count == 256)break;
    }
    Serial.write(buffer, count);           // if no data transmission ends, write buffer to hardware serial port
    clearBufferArray();              // call clearBufferArray function to clear the storaged data from the array
    count = 0;                       // set counter of while loop to zero
  }
  if (Serial.available())            // if data is available on hardwareserial port ==> data is comming from PC or notebook
    Serial2.write(Serial.read());       // write it to the SoftSerial shield
}
