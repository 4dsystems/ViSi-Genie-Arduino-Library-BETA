#include <genieArduino.h>
Genie genie;

void setup() 
{
  Serial.begin(115200);  // UART Debug for terminal
  Serial1.begin(200000); // UART for Genie Display
  
  genie.Begin(Serial1);
  genie.AttachEventHandler(myGenieEventHandler);
  
  genie.WriteContrast(15);
  pinMode(13, OUTPUT);
}

void loop() 
{
  genie.DoEvents();
  //genie.Ping(); // Manual Ping to check if display is connected
  genie.WriteObject(GENIE_OBJ_LED_DIGITS, 0, random(111, 9000));
  genie.WriteObject(GENIE_OBJ_LED_DIGITS, 1, random(111, 9000));
  genie.WriteObject(GENIE_OBJ_LED_DIGITS, 2, random(111, 9000));

  static int coolgaugeNeedle;
  coolgaugeNeedle++;
  if ( coolgaugeNeedle > 100 ) 
  {
    coolgaugeNeedle = 0;
  }
  genie.WriteObject(GENIE_OBJ_COOL_GAUGE, 0, coolgaugeNeedle);
}

void myGenieEventHandler() 
{
  genieFrame Event;
  genie.DequeueEvent(&Event);
  
  // For debugging, to show the events that come in to be processed
  Serial.print(Event.reportObject.cmd); Serial.print(" : ");
  Serial.print(Event.reportObject.object); Serial.print(" : ");
  Serial.print(Event.reportObject.index); Serial.print(" : ");
  Serial.println(genie.GetEventData(&Event));

  if (Event.reportObject.cmd == GENIE_REPORT_EVENT)
  {
    if (Event.reportObject.object == GENIE_OBJ_KEYBOARD) 
    {
      if (Event.reportObject.index == 0) //Keyboard0
      {
        Serial.println((char)genie.GetEventData(&Event));
      }
    }
    if (Event.reportObject.object == GENIE_OBJ_4DBUTTON) 
    {
      Serial.print("4D Button #");
      Serial.print(Event.reportObject.index);
      Serial.println(" Pressed.");
    }
  }

  if (Event.reportObject.cmd == GENIE_PING) 
  {
    if (Event.reportObject.object == GENIE_DISCONNECTED) 
      digitalWrite(13, HIGH);
    if (Event.reportObject.object == GENIE_READY) 
      digitalWrite(13, LOW);
    if (Event.reportObject.object == GENIE_ACK)
    {
      // manual ping response - Display is Connected - Do something if required
    }
    if (Event.reportObject.object == GENIE_NAK)
    {
      // manually ping response - Display is NOT Connected - Do something if required
    }
  }
}

