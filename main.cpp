#include "mbed.h" 
#include "DHT11.h" 
#include <ctime>
DigitalOut Lights(LED1);    //Lights
DigitalOut AlertLight(p7);  //Red LED
DigitalOut Buzzer(p8);      //Buzzer
PwmOut Fan(p21);            //Fan motor
PwmOut Door(p22);           //Door motor
DigitalIn LightSwitch(p12); //Light switch
DigitalIn PIR(p6);          //IR sensor
DHT11 TempHumid(p16);       //Temperature and humidity sensor
AnalogIn DepthSensor(p15);  //Depth Sensor
Serial device(p9, p10);     //Bluetooth device. 9tx, 10rx
Serial pc(USBTX,USBRX);     //PC connection
int Alarm=1;                //Alarm system. Can be turned off with Bluetooth
int status;                 //Temp / Humidity sensor status
const int lowdepth = 480;   //Maximum water depth
int Depth=0;                //Depth sensor
int Auto=1;                 //Automatic fan state
int Rest=6;                 //Wait time for IR sensor
int StartTime=1774854000;   //Mon Mar 30 2026 00:00:00 GMT-0700 (Pacific Daylight Time)
int CurrentTime=1775372400; //
float Hour=0;               //Work week hours used for automatic shutoff
float Day=0;                //Work week days used for automatic shutoff

int main() { 
    pc.printf("starting \r\n"); 
    device.baud(9600);
    set_time(StartTime);    //Mon Mar 30 2026 00:00:00 GMT-0700 (Pacific Daylight Time)
    char BTstate = 'S';
    Fan=0;
    Door=0;

    while(1) {
        //RTC
        time_t seconds = time(NULL);  
        printf ("Time as a basic string = %s\r\n", ctime(&seconds)); 
        Hour=((CurrentTime-StartTime)/3600)%24;   //Hour of the day
        Day=((CurrentTime-StartTime)/86400)%7;  //Day of the week
        pc.printf("Hour = %f. Day = %f \r\n",Hour, Day);
        //Bluetooth device 
        if(device.readable()) { // Get state for bluetooth. 
            BTstate=device.getc();
            pc.printf("BlueTooth state = %c \r\n",BTstate);
            switch (BTstate){
                //Turn alarm on
                case 'A':
                Alarm=1;
                pc.printf("Alarm is On.\r\n");
                break;
                //Turn alarm off
                case 'a':
                Alarm=0;
                pc.printf("Alarm is Off.\r\n");
                break;
                //Open door
                case 'D':
                Door=0.5;
                wait(1);
                Door=0;
                pc.printf("Door Opened.\r\n");
                break;
                //Close door
                case 'd':
                Door=-0.5;
                wait(1);
                Door=0;
                pc.printf("Door Closed.\r\n");
                break;
                //Turn automatic fanning on
                case 'F':
                Auto=1;
                pc.printf("Automatic Fan On.\r\n");
                break;
                //Turn automatic fanning off
                case 'f':
                Auto=0;
                pc.printf("Automatic Fan Off.\r\n");
                break;
                //Turn fan on manually through BlueTooth
                case 'M':
                Fan=1;
                pc.printf("Manual Fan On.\r\n");
                break;
                //Turn fan off manually through BlueTooth
                case 'm':
                Fan=0;
                pc.printf("Manual Fan Off.\r\n");
                break;
                //Check sensors and status'
                // case 'C':
                // device.printf(TempHumid.readTemperature());
                // device.printf(TempHumid.readHumidity());
                // device.printf(DepthSensor.read());
                // if((Day<5) && (Hour>=8) && (Hour<=16)){
                //     device.printf("Working");
                // }
                // else if((Day>=5) || (Hour<8) || (Hour>16)){
                //     device.printf("Not Working");
                // }
                // if (Lights==1){
                //     device.printf("Activity");
                // }
                // else if (Lights==0){
                //     device.printf("No Activity");
                // }
                // break;
            }
        }
        else{
             pc.printf("BlueTooth device not readable \r\n");
        }
        if(LightSwitch.read()==1){      //Turns on lights and factory
            Lights=1;
            //Automatic turn off
            if(Day>=5){    //Check if Day is a weekend
                if((Hour<8) || (Hour>16)){  //Check if Hour is not during work hours
                    //If all of the above are true then turn on motion sensors.
                    //IR sensor 
                    if (PIR){       //If it senses movement turn on lights and sound alarm
                        Lights=1;
                        Rest=0;
                        if (Alarm==1){  //If alarm is enabled sound alarm
                            pc.printf("Motion detected after hours!\r\n");
                            AlertLight=1;
                            Buzzer=1;
                            wait(1);
                            AlertLight=0;
                            Buzzer=0;
                        }
                    }
                    else if (!PIR){ //If it doesn't sense movement inscrease rest counter
                        Rest++;
                        if (Rest>=6){   //If no movement is detected for 6s, turn off lights
                        Lights=0;
                        Rest=6;
                        }
                    }
                }
            }
        }
        else if(LightSwitch.read()==0){  //Checks if lights are not manually on
            Lights=0;
            if (PIR){       //If it senses movement turn on lights and sound alarm
                Lights=1;
                Rest=0;
                if (Alarm==1){  //If alarm is enabled sound alarm
                    pc.printf("Motion detected after hours!\r\n");
                    AlertLight=1;
                    Buzzer=1;
                    wait(1);
                    AlertLight=0;
                    Buzzer=0;
                }
            }
            else if (!PIR){ //If it doesn't sense movement inscrease rest counter
                Rest++;
                if (Rest>=6){   //If no movement is detected for 6s, turn off lights
                Lights=0;
                Rest=6;
                }
            }
        }
        if (Lights==1){     //Allows factory functions if lights is on
            //Temp / Humidity sensor and automatic Fan
            status = TempHumid.readData(); // Read the status of sensor
            if (status != DHT11::OK) { // If not okay
                pc.printf("Temp Sensor not ready\r\n");
            }
            else { // If the status is okay, read the values
                pc.printf("Temperature: %d C\r\n", TempHumid.readTemperature());
                pc.printf("Humidity: %d %%\r\n", TempHumid.readHumidity());
                if (TempHumid.readTemperature()>25){    //Turns on fan if temperature is above 25C
                    pc.printf("High Temperature!\r\n");
                    if (Auto==1){
                        Fan=1;
                    }
                }
                else if (TempHumid.readTemperature()<=25){
                    if (Auto==1){
                        Fan=0;
                    }
                }
                if (TempHumid.readHumidity()>25){    //Turns on fan if humidity is above 25%
                    pc.printf("High Humidity!\r\n");
                    if (Auto==1){
                        Fan=1;
                    }
                }
                else if (TempHumid.readHumidity()<=25){
                    if (Auto==1){
                        Fan=0;
                    }
                }
            }
            //Water level sensor 
            Depth=DepthSensor.read(); // Read the sensor values. 
            pc.printf("Water level is %d\r\n", Depth); 
            if(Depth>lowdepth){
                pc.printf("Water leak!\r\n");
                AlertLight=1;
                Buzzer=1;
                wait(1);
                AlertLight=0;
                Buzzer=0;
            }
        }
        //End loop functions
        BTstate='S';    //Resets BlueTooth state so commands are not repeated
        wait(2);
    } 
}
