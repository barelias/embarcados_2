#include <LiquidCrystal_I2C.h>
#include <Keypad.h> // Library for keypad
#include <Servo.h>

/* Locking mechanism definitions */
#define SERVO_PIN 6          // Define the pin for the servo motor
#define SERVO_LOCK_POS   0   // Pose 0 degree 
#define SERVO_UNLOCK_POS 90  // Pose 90 degree
Servo servoMotor; // Create a servo object

const byte lines = 4; // Number of keypad rows
const byte columns = 4; // Number of keypad columns

const char KEYPAD_MATRIX[lines][columns] = { // Keypad character matrix (keypad mapping)
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

const byte ROW_PINS[lines] = {10, 9, 8, 7}; // Connection pins for keypad rows
const byte COLUMN_PINS[columns] = {5, 4, 3, 2}; // Connection pins for keypad columns

Keypad custom_keypad = Keypad(makeKeymap(KEYPAD_MATRIX), ROW_PINS, COLUMN_PINS, lines, columns); // Initialize keypad

// LCD Config
#define I2C_ADDR    0x27
#define LCD_COLUMNS 16
#define LCD_LINES   2

LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLUMNS, LCD_LINES);

// state config
int state = 0; /*
  0=message to define password (waits for key) (unlocked) (message=write_pass)
  1=pass definition (unlocked) (message=password)
  2=pass defined (locked) (message=write lock) (waits for key)
  3=pass defined (locked) (message=pass)
  4=pass right (unlocked) (message=press anything to lock->2)
  5=pass wrong (locked) (message=wrong, press anything to try again->2)
*/
char written_password[] = "****";
char choosed_password[] = "****";
int password_message_count = 0;
int is_backlit = 1;
unsigned long int time_last_lcd_change_millis = millis();

void setup() {
  // Initialize the LCD display and turn on backlight
  lcd.init();
  lcd.backlight();
  is_backlit = 1;

  Serial.begin(9600); // Initialize serial port (keypad)

  servoMotor.attach(SERVO_PIN); // Attach servo to the defined pin
  servoMotor.write(SERVO_UNLOCK_POS); // Set initial position of the servo

}

void loop() {
  // put your main code here, to run repeatedly:
  char key_pressed = custom_keypad.getKey();
  switch (state) {
    case 0:
      lcd.setCursor(1, 0);
      lcd.print("DEFINA A SENHA");
      lcd.setCursor(1, 1);
      lcd.print("                  ");
      lcd.setCursor(1, 2);
      if (key_pressed) {
        time_last_lcd_change_millis = millis();
        if (is_backlit == 0) {
          lcd.backlight();
        }
        is_backlit = 1;
        written_password[password_message_count] = key_pressed;
        password_message_count++;
        state = 1;
        strcpy(choosed_password, "****");
      }
      break;
    case 1:
      lcd.setCursor(1, 0);
      lcd.print("SENHA ESCOLHIDA:");
      lcd.setCursor(1, 1);
      lcd.print(written_password);
      lcd.setCursor(1, 2);
      if (key_pressed) {
        written_password[password_message_count] = key_pressed;
        password_message_count++;
        time_last_lcd_change_millis = millis();
        if (is_backlit == 0) {
          lcd.backlight();
        }
        is_backlit = 1;
        if (password_message_count > 3) {
          state = 2;
          strcpy(choosed_password, written_password);
          strcpy(written_password, "****");
          password_message_count = 0;
          servoMotor.write(SERVO_LOCK_POS);
          lcd.clear();
        }
      }
      break;
    case 2:
      lcd.setCursor(1, 0);
      lcd.print("DIGITE A SENHA:");
      lcd.setCursor(1, 1);
      switch (password_message_count) {
        case 0:
          lcd.print("     ");
          break;
        case 1:
          lcd.print("*    ");
          break;
        case 2:
          lcd.print("**    ");
          break;
        case 3:
          lcd.print("***   ");
          break;
      }
      lcd.print("");
      lcd.setCursor(1, 2);
      if (key_pressed) {
        time_last_lcd_change_millis = millis();
        if (is_backlit == 0) {
          lcd.backlight();
        }
        is_backlit = 1;
        written_password[password_message_count] = key_pressed;
        password_message_count++;
        if (password_message_count > 3) {
          if (!strcmp(written_password, choosed_password)) {
            strcpy(written_password, "****");
            servoMotor.write(SERVO_UNLOCK_POS);
            state = 3;
          } else {
            strcpy(written_password, "****");
            state = 4;
          }
        }
        lcd.clear();
      }
      break;
    case 3:
      lcd.setCursor(0, 0);
      lcd.print("SENHA CORRETA!");
      lcd.setCursor(0, 1);
      lcd.print("1-LOCK 2-RESET");
      lcd.setCursor(1, 2);
      if (key_pressed) {
        time_last_lcd_change_millis = millis();
        if (is_backlit == 0) {
          lcd.backlight();
        }
        is_backlit = 1;
        if (key_pressed == '1') {
          password_message_count = 0;
          servoMotor.write(SERVO_LOCK_POS);
          state = 2;
          lcd.clear();
        } else if (key_pressed == '2') {
          strcpy(choosed_password, "****");
          strcpy(written_password, "****");
          password_message_count = 0;
          servoMotor.write(SERVO_LOCK_POS);
          state = 0;
          lcd.clear();
        }
      }
      break;
    case 4:
      lcd.setCursor(1, 0);
      lcd.print("SENHA INCORRETA!");
      lcd.setCursor(1, 1);
      lcd.print("TENTE NOVAMENTE");
      lcd.setCursor(1, 2);
      if (key_pressed) {
        time_last_lcd_change_millis = millis();
        if (is_backlit == 0) {
          lcd.backlight();
        }
        is_backlit = 1;
        password_message_count = 0;
        state = 2;
        lcd.clear();
      }
      break;
  }
  if (millis() - time_last_lcd_change_millis > 5000) {
    if (is_backlit != 0) {
      Serial.println(time_last_lcd_change_millis, millis());
      lcd.noBacklight();
      is_backlit = 0;
    }
  }
}
