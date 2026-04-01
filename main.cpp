#include "mbed.h" 
#include "DHT11.h" 
#include <ctime>
DigitalOut Leds(LED1);      //Lights
DigitalIn LightSwitch(p12); //Light switch
PwmOut Fan(p21);         //Fan motor
PwmOut Door(p22);        //Door motor
DHT11 TempHumid(p16);        //Temperature and humidity sensor 
DigitalIn PIR(p6);          //IR sensor 
AnalogIn DepthSensor(p15); //Depth Sensor
Serial device(p9, p10);     //Bluetooth device. 9tx, 10rx
Serial pc(USBTX,USBRX);     //PC connection
int status;
const int lowdepth = 480;
int depth =0;
int Rest=0;                 //Wait time for IR sensor
int Placeholder=1774882800;
float Hour=0;
float Day=0;

int main() { 
    pc.printf("starting \r\n"); 
    device.baud(9600);
    // set_time(localtime());
    set_time(1774854000);    //Mon Mar 30 2026 00:00:00 GMT-0700 (Pacific Daylight Time)
    char BTstate = 'S'; //Bluetooth. S for start, D for door, W for workhours, F for fan.
    Fan=0;
    Door=0;

    while(1) {
        //Bluetooth device 
        if(device.readable()) { // Get state for bluetooth. 
            BTstate=device.getc();
            pc.printf("BlueTooth state = %c \r\n",BTstate);
        }
        else{
             pc.printf("BlueTooth device not readable \r\n");
        }
        switch (BTstate){
            case 'D':
            pc.printf("Door Opened \r\n");
            break;
            case 'd':
            pc.printf("Door Opened \r\n");
            break;
        }
        //Temp and Humidity sensor 
        status = TempHumid.readData(); // Read the status of sensor
        if (status != DHT11::OK) { // If not okay
            pc.printf("Device not ready\r\n");
        } else { // If the status is okay, read the values
            pc.printf("Temperature: %d C\r\n", TempHumid.readTemperature());
            pc.printf("Humidity: %d %%\r\n", TempHumid.readHumidity());
        }
        //Fan and Door motors
        // Fan=0.5;
        // if(){    //Command to open door
        //     Door=0.5;
        //     wait(1);
        //     Door=0;
        //     //Send info to device that door is open
        // }
        // if(){   //Command to close door
        //     Door=-0.5;
        //     wait(1);
        //     Door=0;
        //     //Send info to device that door is closed
        // }
        //RTC
        time_t seconds = time(NULL);  
        printf ("Time as a basic string = %s\r\n", ctime(&seconds)); 
        Hour=((seconds-1774854000)/3600)%24;   //Hour of the day
        Day=((seconds-1774854000)/86400)%8;  //Day of the week
        if(LightSwitch.read()==1){
            Leds=1;
        }
        if(LightSwitch.read()==0){  //Checks if lights are not manually on
            if(Day>5){    //Check if Day is a weekend
                if((Hour>16) || (Hour<8)){  //Check if Hour is not during work hours
                    //If all of the abovve are true then turn on motion sensors.
                    //IR sensor 
                    if (PIR){       //If it senses movement turn on lights and reset rest counter
                        Leds=1;
                        Rest=0;
                    }
                    else if (!PIR){ //If it doesn't sense movement inscrease rest counter
                        Rest++;
                    }
                    if (Rest==6){   //If no movement is detected for 6s and is not work hours, turn off lights
                        Leds=0;
                    }
                }
            }
        }
        //Water level sensor 
        depth=DepthSensor.read(); // Read the sensor values. 
        pc.printf("Water level is %d\r\n", DepthSensor.read(), depth); 
        if(depth>lowdepth){
            //todo implement alarm
            continue;
        }
        BTstate='S';
        wait(10);
    } 
}
