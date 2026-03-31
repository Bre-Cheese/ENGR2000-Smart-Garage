#include "mbed.h" 
#include "DHT11.h" 
DigitalOut led(LED1); 
DHT11 TempHumid(p7);//Temperature and humidity sensor 
DigitalIn PIR(p5);//IR sensor 
Serial device(p9, p10); //Bluetooth device 
Serial pc(USBTX,USBRX); 
const int lowdepth = 480; 
DigitalIn DepthSensor(p15); //Set depthSensor input pin to Analog 0. 
int Rest=0; //Wait time for IR sensor

int main() { 
    pc.printf("starting \n\r"); 
    device.baud(9600); 
    char BTstate = 'S'; //Bluetooth. S for start, D for door, W for workhours, F for fan.
    int Tempstatus; 

    while(1) {
        // set_time(localtime());  //Set timestamp
        //Bluetooth device 
        if(device.readable()) { // Get state for bluetooth. 
            BTstate=device.getc();
            pc.printf("State = %c \n \r",BTstate);
        }
        // if (BTstate=='D'){

        // }
        //Temp and Humidity sensor 
        Tempstatus = TempHumid.readData(); // Read the status of DHT11 humidity sensor 
        if (Tempstatus != DHT11::OK) { // If not okay 
            pc.printf("Device not ready\r\n"); 
        } 
        else { 
            pc.printf("T = %2.1f\n\r",TempHumid.readTemperature()); 
            pc.printf("H = %2.2f %%\n\r",TempHumid.readHumidity()); 
        } 
        //RTC 
        time_t seconds = time(NULL);  
        printf ("Time as a basic string = %s\r\n", ctime(&seconds)); 
        char buffer [32]; 
        strftime(buffer, 32, "%I:%M %p\n\r", localtime(&seconds)); 
        //IR sensor 
        if (PIR){       //If it senses movement turn on lights and reset rest counter
            led=1;
            Rest=0;
        }
        else if (!PIR){ //If it doesn't sense movement inscrease rest counter
            Rest++;
        }
        if (Rest==6){   //If no movement is detected for 6s and is not work hours, turn off lights
            led=0;
        }
        //Water level sensor 
        int depth = DepthSensor.read(); // Read the sensor values. 
        pc.printf("Water ADC%d level is %d\n\r", DepthSensor.read(), depth); 
        if(depth>lowdepth){
            //todo implement alarm
            continue;
        }
        BTstate='S';
        wait(1);
    } 
}
