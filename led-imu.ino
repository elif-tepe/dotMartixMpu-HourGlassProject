#include <LedControl.h>
#include <Wire.h>


#define DEVICE (0x53)   //ADXL345 device address
#define TO_READ (6)     //num of bytes we are going to read (two bytes for each axis)

byte buff[TO_READ] ;    //6 bytes buffer for saving data read from the device
char str[512];          //string buffer to transform data before sending it

int MATRIX_WIDTH = 8;
LedControl lc = LedControl(12, 11, 10, 1); // DIN, CLK, CS, NRDEV
unsigned long delaytime = 50;
int x_key = A1;
int y_key = A0;
int x_pos;
int y_pos;


// object that represents a single light location
// future update with gravity
class Grain
{
  public:
    int x = 0;
    int y = 0;
    int mass = 1;
};
Grain *g;



void setup()
{
  // set up a grain object
  g = new Grain();

  ClearDisplay();

  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output

  //Turning on the ADXL345
  writeTo(DEVICE, 0x2D, 0);
  writeTo(DEVICE, 0x2D, 16);
  writeTo(DEVICE, 0x2D, 8);
}



void loop()
{
  // The first axis-acceleration-data register
  int regAddress = 0x32;
  int x, y, z;

  readFrom(DEVICE, regAddress, TO_READ, buff); //read the acceleration data from ADXL345

  // Combine the two bytes of each direction
  // Least significant bit first
  x = (((int)buff[1]) << 8) | buff[0];
  y = (((int)buff[3]) << 8) | buff[2];
  z = (((int)buff[5]) << 8) | buff[4];

  // Convert the values into values that can be represented on the matrix
  x = map(x, -300, 300, 0, 8);
  y = map(y, -300, 300, 0, 8);
  z = map(z, -300, 300, 0, 8);

  //we send the x y z values as a string to the serial port
  Serial.print("X: ");
  Serial.print(x);
  Serial.print("   Y: ");
  Serial.print(y);
  Serial.print("   Z: ");
  Serial.print(z);
  Serial.print("\n");

  ClearDisplay();
  // assign the grain to this location
  g->x = x;
  g->y = y;
  lc.setLed(0, g->x, g->y, true);


  //add some delay between each update
  delay(10);
}



void ClearDisplay()
{
  // sets up the lcd display
  int devices = lc.getDeviceCount();

  for (int address = 0; address < devices; address++)
  {
    lc.shutdown(address, false);
    lc.setIntensity(address, 1);
    lc.clearDisplay(address);
  }
}



//Writes val to address register on device
void writeTo(int device, byte address, byte val) 
{
  Wire.beginTransmission(device); //start transmission to device
  Wire.write(address);        // send register address
  Wire.write(val);        // send value to write
  Wire.endTransmission(); //end transmission
}



//reads num bytes starting from address register on device in to buff array
void readFrom(int device, byte address, int num, byte buff[]) 
{
  Wire.beginTransmission(device); //start transmission to device
  Wire.write(address);        //sends address to read from
  Wire.endTransmission(); //end transmission

  Wire.beginTransmission(device); //start transmission to device
  Wire.requestFrom(device, num);    // request 6 bytes from device

  int i = 0;
  while (Wire.available())   //device may send less than requested (abnormal)
  {
    buff[i] = Wire.read(); // receive a byte
    i++;
  }
  Wire.endTransmission(); //end transmission
}
