#include <genieArduino.h>

// This Demo communicates with a 4D Systems Display, configured with ViSi-Genie, utilising the Genie Arduino Library - https://github.com/4dsystems/ViSi-Genie-Arduino-Library.
// The ViSi Genie project contains multiple forms containing different 4D Graphics objects.

// The program checks if the display is connected to the Arduino before writing new values to the ViSi Genie Objects.
// The first and second form contains a Coolgauge and Spectrum objects respectively. If the active form is at any of these two forms, the program will update the present widget.
// The third form contains a Scope which is always updated as long as the display is online regardless of what form is currently active.
// The fourth form contains a Slider, 8 LEDs and an LedDigits object. The Slider updates the LedDigits while the LedDigits forwards the changes to the Arduino.
// The Arduino receives these messages and writes the binary equivalent to the 8 LEDs. Such that if the bit is set or 1, the LED is turned ON.
// The last form contains a LedDigits and a Knob which reports its new value to the Arduino whenever it changes position.
// The Arduino receives these new values and writes it to the LedDigits present on the same form.

// The library also automatically detects if the display is disconnected and reconnected.

// This demo illustrates how to use genie.online, genie.form, genie.WriteObject and setting up an event handler.

// Application Notes on the 4D Systems Website that are useful to understand this library are found: http://www.4dsystems.com.au/appnotes
// Good App Notes to read are:
// ViSi-Genie Connecting a 4D Display to an Arduino Host - http://www.4dsystems.com.au/appnote/4D-AN-00017/
// ViSi-Genie Writing to Genie Objects Using an Arduino Host - http://www.4dsystems.com.au/appnote/4D-AN-00018/
// ViSi-Genie A Simple Digital Voltmeter Application using an Arduino Host - http://www.4dsystems.com.au/appnote/4D-AN-00019/
// ViSi-Genie Connection to an Arduino Host with RGB LED Control - http://www.4dsystems.com.au/appnote/4D-AN-00010/
// ViSi-Genie Displaying Temperature values from an Arduino Host - http://www.4dsystems.com.au/appnote/4D-AN-00015/
// ViSi-Genie Arduino Danger Shield - http://www.4dsystems.com.au/appnote/4D-AN-00025/

Genie genie;

#define COOLGAUGE_FORM  0
#define SPECTRUM_FORM   1
#define SCOPE_FORM      2
#define SLIDER_FORM     3
#define KNOB_FORM       4

char *formTitles[] = {"Coolgauge", "Spectrum", "Scope", "Slider", "Knob"};

void setup() {

  Serial.begin(115200);  // UART Debug for terminal
  Serial1.begin(115200); // UART for Genie Display

  pinMode(13, OUTPUT);

  genie.Begin(Serial1);
  genie.AttachEventHandler(myGenieEventHandler);

  // Set the brightness/Contrast of the Display - (Not needed but illustrates how)
  // Most Displays, 1 = Display ON, 0 = Display OFF. See below for exceptions and for DIABLO16 displays.
  // For uLCD-43, uLCD-220RD, uLCD-70DT, and uLCD-35DT, use 0-15 for Brightness Control, where 0 = Display OFF, though to 15 = Max Brightness ON.
  genie.WriteContrast(15);

  /* Changes timeout flag (ms) to force disconnection,
     Don't go too low,if you see it connecting and disconnecting, increase the value
     Higher value leads to longer time before disconnection */
  genie.timeout(150); // Default: 1250ms
}

void loop() {

  static int coolgaugeVal = 0;
  static int8_t gaugeRotation = -1;
  static uint8_t spectrumVal[10];
  static uint16_t scopeVal[4];
  unsigned long f = 1000;
  static int scopeCtr = 0;
  static unsigned long waitPeriod = millis();
  static unsigned long waitScope = millis();
  static int activeForm = -1;

  // Check if the display is connected
  genie.DoEvents();
  if (genie.online()) {

    // If connected, do the following:
    Serial.print("Active Form: "); Serial.println(formTitles[genie.form()]);

    if (millis() >= waitPeriod) {
      // It is recommended to decrease the frequency of writing to the display using a non-blocking delay

      switch (genie.form()) {
        // It is ideal to only update 4D Graphics Objects when the form containing it is active
        case COOLGAUGE_FORM:
          // If the form containing the Coolgauge is active, update the Coolgauge
          /* Note that it is best to set the form OnActivate event to Report the Event to the Host 
           for forms that are only updated when active. For this example, the Coolgauge form is not set
           to Report OnActivate events by default so you'll notice a short delay when moving to this form.
           Try to set OnActivate event in the ViSi Genie project to see the difference. */
          if (coolgaugeVal == 0 || coolgaugeVal == 100) gaugeRotation *= -1;
          coolgaugeVal += gaugeRotation;
          genie.WriteObject(GENIE_OBJ_COOL_GAUGE, 0, coolgaugeVal);
          break;
        case SPECTRUM_FORM:
          // If the form containing the Spectrum is active, update the Spectrum
          /* Note that it is best to set the form OnActivate event to Report the Event to the Host 
           for forms that are only updated when active. For this example, the Spectrum form is not set
           to Report OnActivate events by default so you'll notice a short delay when moving to this form.
           Try to set OnActivate event in the ViSi Genie project to see the difference. */
          for (int i = 0; i < 10; i++) {
            spectrumVal[i] = 50 * sin(2 * PI * f * (millis() + i)) + 50;
            genie.WriteObject(GENIE_OBJ_SPECTRUM, 0, i << 8 | spectrumVal[i]);
            genie.DoEvents();
          }
          break;
        default:
          break;
      }
      waitPeriod = millis() + 50; // rerun this code to update Cool Gauge and Slider in another 50ms time.
    }

    // Objects not included in the active form can still be updated every loop if desired.
    // If needed, users can also use non-blocking delays like the above example to decrease the number of times that
    // the Arduino is writing this data to the display.
    scopeCtr ++;
    if (scopeCtr >= 200) scopeCtr = 0;
    scopeVal[0] = 127 * sin(2 * PI * scopeCtr / 200) + 127;
    scopeVal[1] = 127 * cos(PI / 2 + 2 * PI * scopeCtr / 200) + 127;
    //    scopeVal[2] = 127 * sin(PI + 2 * PI * scopeCtr / 200) + 127;
    //    scopeVal[3] = 127 * cos(PI + 2 * PI * scopeCtr / 200) + 127;
    scopeVal[2] = 63 * cos(2 * PI * scopeCtr / 200) + 63;
    scopeVal[3] = 63 * cos(2 * PI * scopeCtr / 200) + 190;
    for (int i = 0; i < 4; i++) {
      genie.WriteObject(GENIE_OBJ_SCOPE, 0, scopeVal[i]);
    }

  } else { // If display is not connected
    Serial.println("Display isn't online. Please check connections.");
  }
}

bool ledState[8];

void myGenieEventHandler() {
  genieFrame Event;
  genie.DequeueEvent(&Event);

  if (Event.reportObject.cmd == GENIE_REPORT_EVENT) {
    if (Event.reportObject.object == GENIE_OBJ_SLIDER) {
      if (Event.reportObject.index == 0) {
        // If the REPORT_EVENT was sent by Slider0, update LEDs
        // NOTE: If you check the ViSi Genie project included with this example, you will notice that
        //       the Slider OnChanging event is to update the LedDigits and the LedDigits OnChanged
        //       event is to Report an Event. However, the event reported is still coming from Slider0
        //       since Slider0 is the one that actually initialized the events.
        for (int i = 0; i < 8; i++) {
          ledState[i] = !!(genie.GetEventData(&Event) & (0x01 << (7 - i)));
          genie.WriteObject(GENIE_OBJ_USER_LED, i, ledState[i]);
        }
      }
    } else if (Event.reportObject.object == GENIE_OBJ_KNOB) {
      if (Event.reportObject.index == 0) {
        // If the REPORT_EVENT was sent by Slider0, update ledDigits
        genie.WriteObject(GENIE_OBJ_LED_DIGITS, 1, genie.GetEventData(&Event));
      }
    }

  } else if (Event.reportObject.cmd == GENIE_PING) {

    if (Event.reportObject.object == GENIE_DISCONNECTED) {
      // If the display gets disconnected, turn on LED
      digitalWrite(13, HIGH);
      Serial.println("Display got disconnected");

    } else if (Event.reportObject.object == GENIE_READY) {
      // If the display gets disconnected, turn off LED
      digitalWrite(13, LOW);
      Serial.println("Display is ready");

    } else if (Event.reportObject.object == GENIE_ACK) {
      // manual ping response - Display is Connected - Do something if required

    } else if (Event.reportObject.object == GENIE_NAK) {
      // manually ping response - Display is NOT Connected - Do something if required

    }

  }

}
