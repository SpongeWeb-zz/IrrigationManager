//************************************************
//Melhorias e idéias...
//************************************************
//Vincular a ação a válvula/PIN
//Temporizar o pulso
//Criar interface para rodar no raspberry
// exemplo de data 110012014102022

//http://stackoverflow.com/questions/11395280/c-error-expected-primary-expression-before-token
#include <TimeLib.h>
#include <EEPROM.h>

//Comandos;
//1 = Setar Hora
//2 = Escrever acao
//3 = Carregar acao

int const TOTAL_ITEMS = 3;
int const VALVE_PIN = 3;               
int i;
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

typedef enum {
  direct, //Direta
  pulse  //Pulsada  
} execution_mode ;
//
typedef struct 
{
   int hour;
   int minute;
   int second;
   long duration; //ms
   execution_mode mode;
} action_type_item;
//
typedef struct 
{
  action_type_item items[TOTAL_ITEMS];
} action_type;

//execution_mode_t mode1 = direct;
//execution_mode_t mode2 = pulse;

//action_type_t actions[] = { {10, 20, 23, 500, direct }, {10, 20, 25, 600, pulse }, {10, 20, 29, 600, pulse }};
//action_type action;

action_type action = { 
                       {
                          {19, 00, 10, 30, direct}, 
                          {07, 30, 00, 30, pulse}//,  
                          //{21, 00, 00, 20000, pulse }                        
                       }
                     };
void setup()
{
  //Carregando as acoes salvas na eeprom
  //action = LoadAction_EEPROM();
  pinMode(VALVE_PIN, OUTPUT);      // sets the digital pin as output
  digitalWrite(VALVE_PIN, HIGH);
  //setTime(iHour, iMinute, iSecond, iDay, iMonth, iYear);                
   
  // initialize serial:
  Serial.begin(9600);
  while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB port only
    }  
  Serial.println("Serial aberta");
}

void loop()
{ 
  if (stringComplete) {
    Serial.print("Recebido:");
    Serial.println(inputString);
    switch(inputString[0])
    {
      //Setar hora
      case '1':  
      {
        char day[] = { inputString[1], inputString[2], '\0' };
        int iDay = atoi( day );

        char month[] = { inputString[3], inputString[4], '\0' };
        int iMonth = atoi( month );        

        char year[] = { inputString[5], inputString[6], inputString[7], inputString[8], '\0' };
        int iYear = atoi( year );        

        char hour[] = { inputString[9], inputString[10], '\0' };
        int iHour = atoi( hour );        

        char minute[] = { inputString[11], inputString[12], '\0'};
        int iMinute = atoi( minute );                

        char second[] = { inputString[13], inputString[14], '\0'};
        int iSecond = atoi( second );                       

        setTime(iHour, iMinute, iSecond, iDay, iMonth, iYear);                
        Serial.println("Data atualizada");
        break;      
      }   
     case '2':
     {
        Serial.println("Salvando acoes");
        SaveAction_EEPROM(action);
       break;
     }  
    case '3':
     {
        Serial.println("carregando acoes");
        action = LoadAction_EEPROM();
       break;
     }       
    case '4':
      Serial.println("executando pulse 20 s");
      ExecuteAction({19, 00, 00, 20, pulse });
    }
        
    // clear the string:
    inputString = "";
    stringComplete = false;
  }
     
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year()); 
  Serial.println(); 

  for(i = 0; i < sizeof(action.items); i++)
  {
    action_type_item item = action.items[i];
    if(item.hour == hour() && item.minute == minute() && item.second == second() /*&& item.hour > 0  && item.minute > 0 && item.second > 0*/)
    {
      Serial.println("**********Executando Acao**********");      
      Serial.print("Hour: ");
      Serial.println(item.hour);
      
      Serial.print("Minute: ");
      Serial.println(item.minute);
      
      Serial.print("Second: ");
      Serial.println(item.second);
      
      Serial.print("Duration: ");
      Serial.println(item.duration);
      
      Serial.print("Mode: ");
      Serial.println(item.mode);               

      ExecuteAction(item);
    }  
  }
  delay(1000);                  // waits for a second
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') 
      stringComplete = true;
  }
}

action_type LoadAction_EEPROM()
{
  action_type result;
  int bufferSize = EEPROM.read(0); //tamanho do buffer
  byte buff[bufferSize];   
  if(bufferSize > 0)
  {
    for( i = 0; i < bufferSize; i++)
    {
      buff[i] = EEPROM.read(i+1);
    }  
    //result = (action_type*)buff; //Re-Make the struct    
    memcpy(&result, buff, sizeof(result)); 
    Serial.println("acoes carregadas");
    for(i = 0; i < TOTAL_ITEMS; i++)
    {
        action_type_item actionDItem = result.items[i];
        Serial.print("Hour: ");
        Serial.println(actionDItem.hour);
      
        Serial.print("Minute: ");
        Serial.println(actionDItem.minute);
      
        Serial.print("Second: ");
        Serial.println(actionDItem.second);
        
        Serial.print("Duration: ");
        Serial.println(actionDItem.duration);
        
        Serial.print("Mode: ");
        Serial.println(actionDItem.mode);           
    }       
  } 
  return result;
}
//
void SaveAction_EEPROM(action_type action)
{
  int bufferSize = sizeof(action);
  byte buff[bufferSize];   
  memcpy(buff, &action,  bufferSize);  
  //Salvando no end 0x00 o tamanho do buffer;
  EEPROM.write(0, bufferSize);  
  for( i = 0; i < bufferSize; i++)
  {
    EEPROM.write( i + 1, buff[i]);  
  }
  Serial.println("acoes salva");  
  //action_type* actionD = (action_type*)buff; //Re-Make the struct
}

void ExecuteAction(action_type_item action_item)
{
  if(action_item.mode == direct)
  {
    delay(200);
    digitalWrite(VALVE_PIN, LOW);    // sets the LED off
    delay(action_item.duration * 1000);                  // waits for a second
    digitalWrite(VALVE_PIN, HIGH);   // sets the LED on
  }
  else if(action_item.mode == pulse)
  {
    int count = 0;  
    do 
    {
      Serial.print("duration: ");
      Serial.println(action_item.duration);
      
      delay(200);
      digitalWrite(VALVE_PIN, LOW);    // sets the LED off
      delay(1000);                  // waits for a second
      digitalWrite(VALVE_PIN, HIGH);   // sets the LED on
      count++;
    }
    while(count<=action_item.duration) ;
  }
  //delay(1000);                  // waits for a second
}



