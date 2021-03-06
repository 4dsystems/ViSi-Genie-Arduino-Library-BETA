#include <genieArduino.h>

// This Demo extends the genieArduino_Demo, by showing how to use more than 1 screen at a time, attached to an Arduino with 2+ Serial Ports.
// This Demo uses the same WS4 Genie program on both displays, in this case, 2x uLCD-32PTU's, and an Arduino Mega.
// NOTE: This demo will work even if only one display is connected. The library will automatically detect if a display is disconnected or reconnected.

// This Demo communicates with 2 4D Systems Displays, configured with ViSi-Genie, utilising the Genie Arduino Library - https://github.com/4dsystems/ViSi-Genie-Arduino-Library-BETA.
// The display demo has a slider, a cool gauge, an LED Digits, a string box and a User LED.
// The program receives messages from the Slider0 object on each display using the Reported Events. This is triggered each time the Slider changes on the display, and an event
// is genereated and sent automatically. Reported Events originate from the On-Changed event from the slider itself, set in the Workshop4 software.
// Coolgauge is written to using Write Object, and the String is updated using the Write String command, showing the version of the library.
// The User LED is updated by the Arduino, by first doing a manual read of the User LED and then toggling it based on the state received back.

// As the slider changes, it sends its value to the Arduino, and the Arduino then tells the LED Digit to update its value using genie.WriteObject,
// but of the other displays LED Digit! So the Slider message goes via the Arduino to the LED Digit of the other display.
// Coolgauge is updated via simple timer in the Arduino code, and updates the display with its value.
// The User LED is read using genie.ReadObject, and then updated using genie.WriteObject. It is manually read, it does not use an Event.

// The library also automatically detects if the display is disconnected and reconnected. With these features, the program can now restore the values of objects after a reconnect.

// This demo illustrates how to use genie.ReadObject, genie.WriteObject, Reported Messages (Events), genie.WriteStr, genie.WriteContrast, plus supporting functions.

// Application Notes on the 4D Systems Website that are useful to understand this library are found: http://www.4dsystems.com.au/appnotes
// Good App Notes to read are:
// ViSi-Genie Connecting a 4D Display to an Arduino Host - http://www.4dsystems.com.au/appnote/4D-AN-00017/
// ViSi-Genie Writing to Genie Objects Using an Arduino Host - http://www.4dsystems.com.au/appnote/4D-AN-00018/
// ViSi-Genie A Simple Digital Voltmeter Application using an Arduino Host - http://www.4dsystems.com.au/appnote/4D-AN-00019/
// ViSi-Genie Connection to an Arduino Host with RGB LED Control - http://www.4dsystems.com.au/appnote/4D-AN-00010/
// ViSi-Genie Displaying Temperature values from an Arduino Host - http://www.4dsystems.com.au/appnote/4D-AN-00015/
// ViSi-Genie Arduino Danger Shield - http://www.4dsystems.com.au/appnote/4D-AN-00025/

Genie display1; // Genie Display 1
Genie display2; // Genie Display 2


void setup()
{
  // Use a Serial Begin and serial port of your choice in your code and use the genie.Begin function to send
  // it to the Genie library (see this example below)
  // 200K Baud is good for most Arduinos. Galileo should use 115200.

  pinMode(12, OUTPUT); // indicator led for display #1
  pinMode(13, OUTPUT); // indicator led for display #2

  Serial.begin(115200);  // UART Debug for terminal

  Serial1.begin(200000);   // Serial0 @ 200000 (200K) Baud
  display1.Begin(Serial1);   // Use Serial1 for talking to the Genie Library, and to the 4D Systems display #1

  Serial2.begin(200000);  // Serial1 @ 200000 (200K) Baud
  display2.Begin(Serial2);   // Use Serial2 for talking to the Genie Library, and to the 4D Systems display #2

  display1.AttachEventHandler(myGenieEventHandler1); // Attach the user function Event Handler for processing events for display 1
  display2.AttachEventHandler(myGenieEventHandler2); // Attach the user function Event Handler for processing events for display 2

  /*   *** Debugging Levels ***
      0) Minimal
      1) WriteObjects
      2) Report Events
      3) Report Objects
      4) Reserved
      5) Magic Events
      6) All messages
      All levels broadcast critical problems. */
  display1.debug(Serial, 3); // prints debug information to the serial monitor port, level 3 reports & critical info
  display2.debug(Serial, 6); // prints debug information to the serial monitor port, all reports & critical info

  /* Changes timeout flag (ms) to force disconnection,
    Don't go too low,if you see it connecting and disconnecting, increase the value
    Higher value leads to longer time before disconnection */
  //  display1.timeout(1250); // default: 1250ms
  //  display2.timeout(1250); // default: 1250ms

  /* offline/sync recovery
     too low a value can flood the uart of the display during a connection/sync
     too high a value *might* cause lcd to go in limp mode
     most results seem to work at 50ms, if you have issues with slow response or
     have issues with events not running properly after a sync, adjust this. */
  //  display1.recover(50); // default 50ms
  //  display2.recover(50); // default 50ms

  /* This changes the form to another. genie.form(2) changes to form 2... */
  // display1.form(0); // go to form 0
  // display2.form(0); // go to form 0

  /* returns true (1) if the display is online or false (0) if out of sync/disconnected */
  if (display1.online()) {
    /* Set the brightness/Contrast of the Display - (Not needed but illustrates how)
      Most Displays, 1 = Display ON, 0 = Display OFF. See below for exceptions and for DIABLO16 displays.
      For uLCD-43, uLCD-220RD, uLCD-70DT, and uLCD-35DT, use 0-15 for Brightness Control, where 0 = Display OFF, though to 15 = Max Brightness ON. */
    display1.WriteContrast(15);
  }
  else {
    digitalWrite(12, HIGH); // turn on indicator led (display 1 is not connected)
    Serial.println("Display 1 is not connected!");
  }

  if (display2.online()) {
    display2.WriteContrast(15);
  }
  else {
    digitalWrite(13, HIGH); // turn on indicator led (display 1 is not connected)
    Serial.println("Display 2 is not connected!");
  }

  /* returns the current form the lcd is on */
  if ( display1.form() == 0 ) {
    // if display 1 is on form 0, do something.
  }

  if ( display2.form() == 0 ) {
    // if display 2 is on form 0, do something.
  }

  /* This returns true (1) if the lcd is online, and on form 0,only then will this statement be operational. */
  if ( display1.online(0) == 1 ) {// if display 1 is online and on form 0, then do something.
    /* Write a string to the Display to show the version of the library used */
    display1.WriteStr(0, GENIE_VERSION);
  }

  if ( display2.online(0) == 1 ) {// if display 2 is online and on form 0, then do something.
    /* Write a string to the Display to show the version of the library used */
    display2.WriteStr(0, GENIE_VERSION);
  }
}

void loop()
{
  static long waitPeriod = millis(); // Time now

  static int gaugeAddVal1 = 1; // Set the value at which the Gauge on Display 1 increases by initially
  static int gaugeVal1 = 10; // Starting Value for Gauge on Display 1
  static int gaugeAddVal2 = 2; // Set the value at which the Gauge on Display 2 increases by initially
  static int gaugeVal2 = 50; // Starting Value for Gauge on Display 2

  display1.DoEvents(); // This calls the library each loop to process the queued responses from display 1
  display2.DoEvents(); // This calls the library each loop to process the queued responses from display 2

  if (millis() >= waitPeriod)
  {

    if (display1.online(0)) { // if display 1 is online and on form 0
      // Simulation code for Gauge on Display 1, just to increment and decrement gauge value each loop, for animation
      gaugeVal1 += gaugeAddVal1;
      if (gaugeVal1 >= 99) gaugeAddVal1 = -1; // If the value is > or = to 99, make gauge decrease in value by 1
      if (gaugeVal1 <= 0) gaugeAddVal1 = 1; // If the value is < or = to 0, make gauge increase in value by 1

      // Write to CoolGauge0 with the value in the gaugeVal variable on Display 1
      display1.WriteObject(GENIE_OBJ_COOL_GAUGE, 0, gaugeVal1);

      // The results of this call will be available to myGenieEventHandler() after the display has responded
      // Do a manual read from the UserLEd0 object on Display 1
      display1.ReadObject(GENIE_OBJ_USER_LED, 0);
    }

    if (display2.online(0)) { // if display 2 is online and on form 0
      // Simulation code for Gauge on Display 2, just to increment and decrement gauge value each loop, for animation
      gaugeVal2 += gaugeAddVal2;
      if (gaugeVal2 >= 99) gaugeAddVal2 = -2; // If the value is > or = to 99, make gauge decrease in value by 2
      if (gaugeVal2 <= 0) gaugeAddVal2 = 2; // If the value is < or = to 0, make gauge increase in value by 2

      // Write to CoolGauge0 with the value in the gaugeVal variable on Display 2
      display2.WriteObject(GENIE_OBJ_COOL_GAUGE, 0, gaugeVal2);

      // The results of this call will be available to myGenieEventHandler() after the display has responded
      // Do a manual read from the UserLed0 object on Display 2
      display2.ReadObject(GENIE_OBJ_USER_LED, 0);
    }

    waitPeriod = millis() + 50; // rerun this code to update Cool Gauge and Slider in another 50ms time.
  }
}

/////////////////////////////////////////////////////////////////////
//
// This is the user's event handler. It is called by the DoEvents()
// when the following conditions are true
//
//		The link is in an IDLE state, and
//		There is an event to handle
//
// The event can be either a REPORT_EVENT frame sent asynchronously
// from the display or a REPORT_OBJ frame sent by the display in
// response to a READ_OBJ (genie.ReadObject) request.
//

int slider_val1 = 0;
int slider_val2 = 0;


// Event Handler Function for Display 1
void myGenieEventHandler1(void)
{
  genieFrame Event;
  display1.DequeueEvent(&Event); // Remove the next queued event from the buffer, and process it below

  //If the cmd received is from a Reported Event (Events triggered from the Events tab of Workshop4 objects)
  if (Event.reportObject.cmd == GENIE_REPORT_EVENT)
  {
    if (Event.reportObject.object == GENIE_OBJ_SLIDER)                   // If the Reported Message was from a Slider
    {
      if (Event.reportObject.index == 0)                                 // If Slider0 (Index = 0)
      {
        slider_val1 = display1.GetEventData(&Event);                  // Receive the event data from the Slider0
        display2.WriteObject(GENIE_OBJ_LED_DIGITS, 0, slider_val1);       // Write Slider0 value of Display 1 to to LED Digits 0 of Display 2 !
      }
    }
  }

  //If the cmd received is from a Reported Object, which occurs if a Read Object (genie.ReadOject) is requested in the main code, reply processed here.
  if (Event.reportObject.cmd == GENIE_REPORT_OBJ)
  {
    if (Event.reportObject.object == GENIE_OBJ_USER_LED)                 // If the Reported Message was from a User LED
    {
      if (Event.reportObject.index == 0)                                 // If UserLed0 (Index = 0)
      {
        bool UserLed0_val = display1.GetEventData(&Event);               // Receive the event data from the UserLed0
        UserLed0_val = !UserLed0_val;                                    // Toggle the state of the User LED Variable
        display1.WriteObject(GENIE_OBJ_USER_LED, 0, UserLed0_val);       // Write UserLed0_val value back to to UserLed0
      }
    }
  }

  else if (Event.reportObject.cmd == GENIE_PING) {
    if (Event.reportObject.object == GENIE_DISCONNECTED) {
      /* This function runs once, when the LCD is disconnected, because it was turned off or out of sync.
           You may use this to process necessary code. */
      digitalWrite(12, HIGH);
    }

    else if (Event.reportObject.object == GENIE_READY) {
      /* This function runs once, when the LCD is connected and synchronized.
           You may use this to restore screen widgets, or process other code. */
      digitalWrite(12, LOW);
      display2.WriteObject(GENIE_OBJ_LED_DIGITS, 0, slider_val1); // Restore Leddigits0 on display 2

      display1.WriteObject(GENIE_OBJ_SLIDER, 0, slider_val1); // Restore Slider0
      display1.WriteObject(GENIE_OBJ_LED_DIGITS, 0, slider_val2); // Restore Leddigits0  on display 1
      static int recover_times = -1; // how many times did the display recover?
      recover_times++;
      display1.WriteStr(0, (String)GENIE_VERSION + "\n\n\tRecovered " + recover_times + " Time(s)!"); // Restore text in Strings0
    }
  }
}

// Event Handler Function for Display 2
void myGenieEventHandler2(void)
{
  genieFrame Event;
  display2.DequeueEvent(&Event); // Remove the next queued event from the buffer, and process it below

  //If the cmd received is from a Reported Event (Events triggered from the Events tab of Workshop4 objects)
  if (Event.reportObject.cmd == GENIE_REPORT_EVENT)
  {
    if (Event.reportObject.object == GENIE_OBJ_SLIDER)                   // If the Reported Message was from a Slider
    {
      if (Event.reportObject.index == 0)                                 // If Slider0 (Index = 0)
      {
        slider_val2 = display2.GetEventData(&Event);                  // Receive the event data from the Slider0
        display1.WriteObject(GENIE_OBJ_LED_DIGITS, 0, slider_val2);       // Write Slider0 value of Display 2 to to LED Digits 0 of Display 1
      }
    }
  }

  //If the cmd received is from a Reported Object, which occurs if a Read Object (genie.ReadOject) is requested in the main code, reply processed here.
  else if (Event.reportObject.cmd == GENIE_REPORT_OBJ)
  {
    if (Event.reportObject.object == GENIE_OBJ_USER_LED)                 // If the Reported Message was from a User LED
    {
      if (Event.reportObject.index == 0)                                 // If UserLed0 (Index = 0)
      {
        bool UserLed0_val = display2.GetEventData(&Event);               // Receive the event data from the UserLed0
        UserLed0_val = !UserLed0_val;                                    // Toggle the state of the User LED Variable
        display2.WriteObject(GENIE_OBJ_USER_LED, 0, UserLed0_val);       // Write UserLed0_val value back to to UserLed0
      }
    }
  }

  /********** This can be expanded as more objects are added that need to be captured *************
  *************************************************************************************************
    Event.reportObject.cmd is used to determine the command of that event, such as an reported event
    Event.reportObject.object is used to determine the object type, such as a Slider
    Event.reportObject.index is used to determine the index of the object, such as Slider0
    genie.GetEventData(&Event) us used to save the data from the Event, into a variable.
  *************************************************************************************************/

  else if (Event.reportObject.cmd == GENIE_PING) {
    if (Event.reportObject.object == GENIE_DISCONNECTED) {
      /* This function runs once, when the LCD is disconnected, because it was turned off or out of sync.
           You may use this to process necessary code. */
      digitalWrite(13, HIGH);
    }

    else if (Event.reportObject.object == GENIE_READY) {
      /* This function runs once, when the LCD is connected and synchronized.
           You may use this to restore screen widgets, or process other code. */
      digitalWrite(13, LOW);
      display1.WriteObject(GENIE_OBJ_LED_DIGITS, 0, slider_val2); // Restore Leddigits0

      display2.WriteObject(GENIE_OBJ_SLIDER, 0, slider_val2); // Restore Slider0
      display2.WriteObject(GENIE_OBJ_LED_DIGITS, 0, slider_val1); // Restore Leddigits0

      static int recover_times = -1; // how many times did the display recover?
      recover_times++;
      display2.WriteStr(0, (String)GENIE_VERSION + "\n\n\tRecovered " + recover_times + " Time(s)!"); // Restore text in Strings0
    }
  }
}
