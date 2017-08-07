#include <genieArduino.h>

// This Demo communicates with a 4D Systems Display, configured with ViSi-Genie, utilising the Genie Arduino Library - https://github.com/4dsystems/ViSi-Genie-Arduino-Library-BETA.
// The display has a strings object that can be written to. It also has a MagicObject0 that receives WRITE_MAGIC_BYTES and WRITE_MAGIC_DBYTES messages from the host
// and sends back REPORT_MAGIC_EVENT_BYTES and REPORT_MAGIC_EVENT_DBYTES messages back to the host.

// This demo generates a random set of bytes and sends them to the display module in the form of either a WRITE_MAGIC_BYTES message or a WRITE_MAGIC_DBYTES message.
// If the host sends a WRITE_MAGIC_BYTES message containing a random set of magic bytes, the display replies with a REPORT_MAGIC_EVENT_BYTES message which contains 
// exactly the same magic bytes.
// If the host sends a WRITE_MAGIC_DBYTES message containing a random set of magic double bytes, the display replies with a REPORT_MAGIC_EVENT_DBYTES message which 
// contains exactly the same magic double bytes.
// The program alternates between sending a WRITE_MAGIC_BYTES message and a WRITE_MAGIC_DBYTES message. 
// For this demo, the maximum length of magic bytes message sent is set to 50 bytes. Similarly, the maximum length of magic double bytes sent is set to 25 double 
// bytes or 50 bytes.
// Note also, that the host has to receive and process the same amount of bytes, since the display module sends these bytes back to the host.
// It is important to note that, ideally, the host has to be able to process the bytes in the serial receive buffer in a timely manner to avoid overflow. Otherwise, 
// the library might behave in a strange way. 
// Note also that the DoEvents() method in the main loop calls on the event handlers, so it (DoEvents()) should execute as frequent as possible to process the bytes 
// and hence to avoid overflow. Therefore, any additional instructions in the code which will delay the execution of DoEvents() can cause a buffer overflow.
// If a longer magic bytes or magic double bytes message is desired, the user can manually increase the size of the serial receive buffer of the Arduino host. 
// Another option is to insert delays into the MagicObject0 code of the display module. This will give the host some time to process the messages, hence avoiding 
// overflow.

// The library also automatically detects if the display is disconnected and reconnected. With these features, the program can now restore the values of objects after a reconnect.

// This demo illustrates how to use genie.WriteMagicBytes, genie.WriteMagicDBytes, genie.GetNextByte, genie.GetNextDoubleByte, Reported Messages (Events), genie.WriteStr, 
// genie.WriteContrast, plus supporting functions.

// Application Notes on the 4D Systems Website that are useful to understand this library are found: http://www.4dsystems.com.au/appnotes
// Good App Notes to read are:
// ViSi-Genie Connecting a 4D Display to an Arduino Host - http://www.4dsystems.com.au/appnote/4D-AN-00017/
// ViSi-Genie Writing to Genie Objects Using an Arduino Host - http://www.4dsystems.com.au/appnote/4D-AN-00018/
// ViSi-Genie A Simple Digital Voltmeter Application using an Arduino Host - http://www.4dsystems.com.au/appnote/4D-AN-00019/
// ViSi-Genie Connection to an Arduino Host with RGB LED Control - http://www.4dsystems.com.au/appnote/4D-AN-00010/
// ViSi-Genie Displaying Temperature values from an Arduino Host - http://www.4dsystems.com.au/appnote/4D-AN-00015/
// ViSi-Genie Arduino Danger Shield - http://www.4dsystems.com.au/appnote/4D-AN-00025/

Genie genie;

#define CW 0
#define CCW 1

#define SEND_BYTES 0
#define SEND_DBYTES 1

uint8_t  magicBytes[100];
uint16_t magicDBytes[100];

uint8_t alternateFlag = SEND_BYTES;

void setup() {
  pinMode(13, OUTPUT); // indicator led

  genie.AttachEventHandler(myGenieEventHandler);
  genie.AttachMagicByteReader(myGenieMagicBHandler); // Attach the user function Magic Handler for processing REPORT_MAGIC_EVENT_BYTES events
  genie.AttachMagicDoubleByteReader(myGenieMagicDBHandler); // Attach the user function Magic Handler for processing REPORT_MAGIC_EVENT_DBYTES events


  Serial.begin(115200);  // UART Debug for terminal
  Serial1.begin(200000); // UART for Genie Display

  while (!genie.Begin(Serial1)) {
    Serial.println("display offline!");
  }

  if (genie.online()) Serial.println("display online!");

  Serial.print("SERIAL_RX_BUFFER_SIZE: "); Serial.println(SERIAL_RX_BUFFER_SIZE);
  /*   *** Debugging Levels ***
      0) Minimal
      1) WriteObjects
      2) Report Events
      3) Report Objects
      4) Reserved
      5) Magic Events
      6) All messages
      All levels broadcast critical problems. */
  genie.debug(Serial, 5); // prints debug information to the serial monitor port, level 2 reports & critical info



  /* Changes timeout flag (ms) to force disconnection,
    Don't go too low,if you see it connecting and disconnecting, increase the value
    Higher value leads to longer time before disconnection */
  //  genie.timeout(1250); // default: 1250ms

  /* offline/sync recovery
     too low a value can flood the uart of the display during a connection/sync
     too high a value *might* cause lcd to go in limp mode
     most results seem to work at 50ms, if you have issues with slow response or
     have issues with events not running properly after a sync, adjust this. */
  //  genie.recover(50); // default 50ms

  /* This changes the form to another. genie.form(2) changes to form 2... */
  // genie.form(0); // go to form 0

  /* returns true (1) if the display is online or false (0) if out of sync/disconnected */
  if (genie.online()) {
    /* Set the brightness/Contrast of the Display - (Not needed but illustrates how)
      Most Displays, 1 = Display ON, 0 = Display OFF. See below for exceptions and for DIABLO16 displays.
      For uLCD-43, uLCD-220RD, uLCD-70DT, and uLCD-35DT, use 0-15 for Brightness Control, where 0 = Display OFF, though to 15 = Max Brightness ON. */
    genie.WriteContrast(15);
  }
  else
    digitalWrite(13, HIGH); // turn on indicator led (display is not connected)

  /* returns the current form the lcd is on */
  if ( genie.form() == 0 ) {
    // if lcd is on form 0, do something.
  }

  /* This returns true (1) if the lcd is online, and on form 0,only then will this statement be operational. */
  if ( genie.online(0) == 1 ) {// if display is online and on form 0, then do something.
    /* Write a string to the Display to show the version of the library used */
    genie.WriteStr(0, GENIE_VERSION);
  }
}

void loop() {
  static long waitPeriod = millis(); // timer to repeat task
  static int coolgaugeVal;
  static boolean gaugeRotation = CW;
  static int messageLength;
  genie.DoEvents();

  /* Ping request at an interval chosen by the user.
    Each interval queues an event, which can be retrieved in the handler.
    This can be ran in your loop and will follow its interval */
  //genie.Ping(1000);

  /* Uptime status, to see how long your display has been online this session. */
  //Serial.println( genie.uptime() ); // prints uptime in millisec to the serial monitor (if used)

  if (millis() >= waitPeriod) {
    if (genie.online(0)) { // check if the display is online (connected) and if on Form0

      if (alternateFlag == SEND_BYTES) {
        // generate magic bytes
        messageLength = random(10, 50);
        Serial.print("sending random magic bytes: ");
        for (int i = 0; i < messageLength; i++) {
          magicBytes[i] = random(0, 255);
          Serial.print( magicBytes[i], HEX);
          Serial.print(" ");
        }
        Serial.println();
        genie.WriteMagicBytes(0, magicBytes, messageLength, 1);
        alternateFlag = SEND_DBYTES;
      }

      else if (alternateFlag == SEND_DBYTES) {
        // generate magic double bytes
        messageLength = random(10, 25);        
        Serial.print("sending random magic double bytes: ");
        for (int i = 0; i < messageLength; i++) {
          magicDBytes[i] = random(0x41, 0x5A) << 8 | random(0x41, 0x5A);
          Serial.print( (char) (magicDBytes[i] >> 8));
          Serial.print(" ");
          Serial.print( (char) (magicDBytes[i] & 0xFF));
          Serial.print(" ");
        }
        Serial.println();
        genie.WriteMagicDBytes(0, magicDBytes, messageLength, 1);
        alternateFlag = SEND_BYTES;
      }
    }
    waitPeriod = millis() + 50; /* rerun this code to update Cool Gauge and Slider in another 50ms time. */
  }
}


/////////////////////////////////////////////////////////////////////
//
// This is the user's event handler.

void myGenieEventHandler() {
  genieFrame Event;
  genie.DequeueEvent(&Event);

  if (Event.reportObject.cmd == GENIE_PING) {
    if (Event.reportObject.object == GENIE_DISCONNECTED) {
      /* This function runs once, when the LCD is disconnected, because it was turned off or out of sync.
           You may use this to process necessary code. */
      digitalWrite(13, HIGH);
    }

    else if (Event.reportObject.object == GENIE_READY) {
      /* This function runs once, when the LCD is connected and synchronized.
           You may use this to restore screen widgets, or process other code. */
      digitalWrite(13, LOW);
      static int recover_times = -1; // how many times did the display recover?
      recover_times++;
      genie.WriteStr(0, (String)GENIE_VERSION + "\n\n\tRecovered " + recover_times + " Time(s)!"); // Restore text in Strings0

    }

    else if (Event.reportObject.object == GENIE_ACK) {
      /* If a user issues a genie.Ping(interval) request and it passes,
         this function will happen every 'interval' times chosen by the user. */
      //digitalWrite(13, HIGH); // here we toggle the led on and off after every successful ping interval.
      //delay(20);
      //digitalWrite(13, LOW);
    }

    else if (Event.reportObject.object == GENIE_NAK) {
      /* If a user issues a genie.Ping(interval) request and it fails,
         this function will happen every 'interval' times chosen by the user. */
    }
  }
}

void myGenieMagicBHandler(uint8_t index, uint8_t length) {
  if (index == 0) // If the data is coming from MagicObject0
  {
    Serial.print("Received ");
    Serial.print(length, DEC);
    Serial.print(" bytes from magic object ");
    Serial.println(index, DEC);

    Serial.print("received magic bytes      : ");
    for (int i = 0; i < length; i++ ) {
      Serial.print(genie.GetNextByte(), HEX);
      Serial.print(" ");
    }
    Serial.println();
    Serial.println();
  }
}

void myGenieMagicDBHandler(uint8_t index, uint8_t length) {
  if (index == 0) // If the data is coming from MagicObject0
  {
    Serial.print("Received ");
    Serial.print(length, DEC);
    Serial.print(" double bytes from magic object ");
    Serial.println(index, DEC);

    Serial.print("received magic double bytes      : ");
    for (int i = 0; i < length; i++ ) {
      int doubleByte = genie.GetNextDoubleByte();
      Serial.print((char)(doubleByte >> 8));
      Serial.print(" ");
      Serial.print((char)(doubleByte & 0xFF));
      Serial.print(" ");
    }
    Serial.println();
    Serial.println();
  }
}


