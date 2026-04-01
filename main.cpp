#include "mbed.h" 
#include "DHT11.h" 
DigitalOut Leds(LED1);      //Lights
DigitalIn LightSwitch(p13); //Light switch
AnalogOut Fan(p11);         //Fan motor
AnalogOut Door(p12);        //Door motor
DHT11 TempHumid(p7);        //Temperature and humidity sensor 
DigitalIn PIR(p5);          //IR sensor 
Serial device(p9, p10);     //Bluetooth device 
Serial pc(USBTX,USBRX);     //PC connection
const int lowdepth = 480;
int depth =0;
DigitalIn DepthSensor(p15); //Set depthSensor input pin to Analog 0. 
int Rest=0;                 //Wait time for IR sensor
int Placeholder=1774882800;
float Hour=0;
float Day=0;

int main() { 
    pc.printf("starting \n\r"); 
    device.baud(9600);
    set_time(1774854000);    //Mon Mar 30 2026 00:00:00 GMT-0700 (Pacific Daylight Time)
    char BTstate = 'S'; //Bluetooth. S for start, D for door, W for workhours, F for fan.
    int Tempstatus;
    Fan=0;
    Door=0;

    while(1) {
        //Bluetooth device 
        if(device.readable()) { // Get state for bluetooth. 
            BTstate=device.getc();
            pc.printf("State = %c \n \r",BTstate);
        }
        switch (BTstate){
            case 'D':
            pc.printf("Door Opened \n \r");
            break;
            case 'd':
            pc.printf("Door Opened \n \r");
            break;
        }
        //Temp and Humidity sensor 
        Tempstatus = TempHumid.readData(); // Read the status of DHT11 humidity sensor 
        if (Tempstatus != DHT11::OK) { // If not okay 
            pc.printf("Device not ready\r\n"); 
        }
        else { 
            pc.printf("T = %2.1f\n\r",TempHumid.readTemperature());
            // device.write("25");
            pc.printf("H = %2.2f %%\n\r",TempHumid.readHumidity());
            // device.write("10");
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
        char buffer [32];
        strftime(buffer, 32, "%I:%M %p\n\r", localtime(&seconds));
        Hour=((seconds-1774854000)/3600)%24;   //Hour of the day
        Day=((seconds-1774854000)/86400)%7;  //Day of the week
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
        pc.printf("Water ADC%d level is %d\n\r", DepthSensor.read(), depth); 
        if(depth>lowdepth){
            //todo implement alarm
            continue;
        }
        BTstate='S';
        wait(1);
    } 
}
