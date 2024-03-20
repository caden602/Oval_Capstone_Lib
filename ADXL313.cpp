/******************************************************************************
  Example1_BasicReadings.ino
  Read values of x/y/z axis of the ADXL313 (via I2C), print them to terminal.
  This uses default configuration (1G range, full resolution, 100Hz datarate).

  SparkFun ADXL313 Arduino Library
  Pete Lewis @ SparkFun Electronics
  Original Creation Date: September 19, 2020
  https://github.com/sparkfun/SparkFun_ADXL313_Arduino_Library

  Do you like this library? Help support SparkFun. Buy a board!

    SparkFun 3-Axis Digital Accelerometer Breakout - ADXL313 (Qwiic)
    https://www.sparkfun.com/products/17241  

  Development environment specifics:

	IDE: Arduino 1.8.13
	Hardware Platform: SparkFun Redboard Qwiic
	SparkFun 3-Axis Digital Accelerometer Breakout - ADXL313 (Qwiic) Version: 1.0

  Hardware Connections:
  Use a qwiic cable to connect from the Redboard Qwiic to the ADXL313 breakout (QWIIC).
  You can also choose to wire up the connections using the header pins like so:

  ARDUINO --> ADXL313
  SDA (A4) --> SDA
  SCL (A5) --> SCL
  3.3V --> 3.3V
  GND --> GND

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/
#include <ADXL313.h>

void adxl_setup(ADXL313 *myAdxl)
{

    if (!myAdxl->begin()) //Begin communication over I2C
    {
        Serial.println("Failed to find AXDL313");
        while(1); 
    }
    Serial.println("ADXL313 Found!");
    
    myAdxl->measureModeOn(); // wakes up the sensor from standby and puts it into measurement mode
}

bool adxl_get_data(ADXL313 *myAdxl, adxl_data_t*adxl_data)
{
  if(myAdxl->dataReady()) // check data ready interrupt, note, this clears all other int bits in INT_SOURCE reg
  {
    myAdxl->readAccel(); // read all 3 axis, they are stored in class variables: myAdxl.x, myAdxl.y and myAdxl.z
    adxl_data->x = myAdxl->x;
    adxl_data->y = myAdxl->y;
    adxl_data->z = myAdxl->z;

    // set error bit to 0 (false)
    return false;
  }
  return true;
}