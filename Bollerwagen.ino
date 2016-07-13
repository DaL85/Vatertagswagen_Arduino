#include <SoftwareSerial.h>

//pin declaration
//seriell
int BT_pin1 = 10;
int BT_pin2 = 11;
//pwm
int out_pwm =5;
//digital
int out_motorbremse = 13;
//analog
int in_sensor_current = 2;
int in_sensor_throttle = 4;
int in_sensor_battery = 5;
//end pin declaration

//mesurement variables
double value_current=0.0;
int value_output_pwm=0;
int value_bat_now=0;
int value_bat_rd_before=0;
double thresholdValue = 30.0;
//end mesurement variables

//helper variables
boolean throttle_phone_active=false;
int loopcounter = 0;
//end helper variables

//serial read variables
String cmd;
String temp_cmd;
String value;
String temp_value;
boolean commandComplete=false;
boolean Kommandostart=false;
boolean Valuestart=false;
//end serial read variables

SoftwareSerial BT(BT_pin1, BT_pin2); 

void setup()  
{
  //the baud rate
   BT.begin(9600);

  pinMode(in_sensor_current, INPUT);
  pinMode(in_sensor_battery, INPUT);
  pinMode(in_sensor_throttle, INPUT);
  pinMode(out_pwm,OUTPUT);
  pinMode(out_motorbremse, OUTPUT);
  
  digitalWrite(out_motorbremse, HIGH);
}

void loop()
{
  pollingPhoneData();
  
  engineCurrentMonitoring();

  batterieVoltageMonitoring();
  
  bollerwagenTrottleControl();
}

void pollingPhoneData()
{
  //read data from phone
  read_serial_data();
  if(commandComplete) //transmission complete and command are valid
  {
    if(cmd == "b")
    {
      setBrake(value);
    }
    else if(cmd == "a")
    {
      setThrottlePhone(value);
    }
    else if(cmd == "t")
    {
      if(throttle_phone_active==true)
      {
        setThrottle(value.toInt());
      }
    }      
  }
}

void engineCurrentMonitoring()
{
  //read the engine current
  int in_value_currentsensor = analogRead(in_sensor_current);
   //0-5V => 0 - 1023 Bit
  //Vout = 2.5 ± (0.625· IP/IPN)V    IPN=400
  //=> IP = (Vout-2.5)*(±IPN/0.625)   400/0.625 = 640
  value_current = ((in_value_currentsensor/1023.0)*5.0)-2.5)*640.0;
  //loopcounter ++;
  //if(loopcounter >= 10 || value_current > thresholdValue)
  //{
   //  loopcounter = 0;
     //send it subsequently to the phone           
     write_serial_data("i",String(value_current));    
  //}
}

void batterieVoltageMonitoring()
{
  //read the battery voltage and send it subsequently to the phone
  //0-5V => 0 - 1023 Bit
    value_bat_now=analogRead(in_sensor_battery);
    value_bat_now=value_bat_now/10;  //to set a lower accuracy
    value_bat_now=value_bat_now*10;
    //String data='z'+(String)value_bat_now;
    //char d[5];
    //data.toCharArray(d,5);
    if(value_bat_rd_before!=value_bat_now)  //otherwise the phone is pretty busy
    {
       //meetAndroid.send(d);
       write_serial_data("u",String(value_bat_now));
       value_bat_rd_before=value_bat_now;
    }
  //end read the battery voltage and send it subsequently to the phone
}

void bollerwagenTrottleControl()
{
   //read and calculate the throttle sensor, send the calculated signal subsequently to the pwm pin
  //if(throttle_phone_active==false)
  //{
    value_output_pwm=(analogRead(in_sensor_throttle)-184)/2.73;
    setThrottle(value_output_pwm);    
  //}
}

void write_serial_data(String flag, String data)
{
  String sendValue = '#'+flag+'_'+data+';';
  BT.println(sendValue);
}

void read_serial_data()
{
    
  //Beispielstring: #red_255;
    commandComplete = false;
    char nextChar;  
      
    if (BT.available()) 
    {            
        nextChar = BT.read();
        if( nextChar == '#' ) //Kommando start
        {
          Kommandostart=true;
          Valuestart = false;
          commandComplete = true;
          temp_cmd = "";
          temp_value = "";
        }
        else
        {    
          if( Kommandostart )
          {            
            if( nextChar == ';' ) //Kommando vollständig
            { 
              cmd = temp_cmd;
              value = temp_value;
              commandComplete = true;              
            }
            else 
            {
              if( nextChar == '_' )
              {
                Valuestart=true;
              }
              else if ( !Valuestart )  //Teilstring noch im Kommandobereich
              {
                temp_cmd += nextChar;
              } 
              else                      //Teilstring im Valuebereich
              {
                temp_value += nextChar;              
              }
            } 
          }
        }        
    }
}

void setBrake(String v)
{
   if(v=="0")
   {
     digitalWrite(out_motorbremse, HIGH);//bremse deaktivieren    
   }
   else if(v=="1")
   {
     digitalWrite(out_motorbremse, LOW);//bremse aktivieren    
   }
}

void setThrottle(int v)
{
  value_current = (double)analogRead(in_sensor_current);
  if(value_current < 750.0)
  {
    int pwm_value = v;
    if(pwm_value<0)
    {
      pwm_value=0;
    }
    else if(pwm_value>190)
    {
      pwm_value=190;
    }
    analogWrite(out_pwm,pwm_value);
  }
  else
  {
    analogWrite(out_pwm,0);
    delay(2000);                  // waits for a 2 seconds
  }
}

void setThrottlePhone(String v)
{  
  if(v=="0")throttle_phone_active=false;
  else if(v=="1")throttle_phone_active=true;    
}

