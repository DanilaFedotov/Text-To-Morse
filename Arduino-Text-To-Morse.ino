// The following code accepts strings (ended by CR or NL) and prints the equivalent morse code of the string.
// The printed morse code is also sent on a pin (default: LED_BUILTIN) to display it with correct time units
// Danila Fedotov; last edited: 11/03/2022

#define DEBUG_MODE
#define DEBUG_PRINT_WORD_SPACES
#define DEBUG_LETTER_SPACE_CHAR ' ' //Char that represents space between letter sequences
#define DEBUG_WORD_SPACE_CHAR '\n'  //Char that represents space between word sequences
#define DEBUG_DASH_CHAR '-'         //Char that represents dash
#define DEBUG_DOT_CHAR '.'          //Char that represents dot

// #define TIMED_MODE //if defined, executes delays for morse code, otherwise instantenous prints

#define LED_OUT LED_BUILTIN //LED used to display morse code

#define TIME_UNIT               1000  //Used to change time constant (default: 1000ms)
#define DOT_LENGTH              1     //Length of dot in number of time units
#define DASH_LENGTH             3     //Length of dash in number of time units
#define LETTER_SPACE_LENGTH     3     //Length of space between dots and dashed in time units
#define WORD_SPACE_LENGTH       7     //Length of space between words in time units
#define INTER_LENGTH            1     //Length between symbools

#define DOT_TIME            (TIME_UNIT * DOT_LENGTH) //Dot time after time unit applied
#define DASH_TIME           (TIME_UNIT * DASH_LENGTH) //Dash time after time unit applied
#define LETTER_SPACE_TIME   (TIME_UNIT * LETTER_SPACE_LENGTH)//Time for space between dots and dashes after time units applied
#define WORD_SPACE_TIME     (TIME_UNIT * (WORD_SPACE_LENGTH - LETTER_SPACE_LENGTH)) //Time for space between words after time units applied
#define INTER_TIME          (TIME_UNIT * INTER_LENGTH) //Time between symbols

//Dynamic character array structure
typedef struct {
  char *array;
  uint8_t size;
} char_array;

//Function definitons
void send_morse_letter(uint8_t enc);
uint8_t encode_to_morse_seq(char ch);
void insert_array(char_array *a, char element);
void flush_array(char_array *a);
void send_morse_string();
char to_upper_case(char ch);
boolean is_morse_char(char ch);

void send_code(uint16_t time_delay, char ch, uint8_t pin_state);
void send_letter_space();
void send_word_space();
void send_dash();
void send_dot();
void send_inter();

//Global Variables
char_array serial_buffer; // Serial buffer to store all characters sent via UART

const PROGMEM uint8_t morse_enc[37] = { //Morse sequences each encoded in 1 byte; encoding: 0b[XXXXX][XXX] / 0b[Sequence][Sequence Length]
  0b01000010, //A
  0b10000100, //B
  0b10100100, //C
  0b10000011, //D
  0b00000001, //E
  0b00100100, //F
  0b11000011, //G
  0b00000100, //H
  0b00000010, //I
  0b01110100, //J
  0b10100011, //K
  0b01000100, //L
  0b11000010, //M
  0b10000010, //N
  0b11100011, //O
  0b01100100, //P
  0b11010100, //Q
  0b01000011, //R
  0b00000011, //S
  0b10000001, //T
  0b00100011, //U
  0b00010100, //V
  0b01100011, //W
  0b10010100, //X
  0b10110100, //Y
  0b11000100, //Z
  0b11111101, //0
  0b01111101, //1
  0b00111101, //2
  0b00011101, //3
  0b00001101, //4
  0b00000101, //5
  0b10000101, //6
  0b11000101, //7
  0b11100101, //8
  0b11110101, //9
  0b00000111  //SPACE
};

void setup() {
  Serial.begin(9600);
  pinMode(LED_OUT, OUTPUT);
  serial_buffer.size = 0;
}

void loop() {
  if (Serial.available()) {
    char ch = to_upper_case(Serial.read()); //Capitalises if letter
    if (is_morse_char(ch)) {
      if ((ch == 32) && (serial_buffer.array[serial_buffer.size - 1] == 32)) {
        return;
      }
      insert_array(&serial_buffer, ch);
    } else if ((ch == 13) || (ch == 10)) {
      send_morse_string();
    }
  }
}

void send_morse_letter(uint8_t enc) { // accepts encoded morse sequence byte
  uint8_t len = (enc & 0b00000111); //3 LSBs are bits to represent morse sequence length
  if (len == 7) { //Encoding for space
    send_word_space();
    return;
  }
  for (uint8_t i = 0; i < len; i++) { //Move through byte sequence
    if (((enc << i) & 0b10000000) == 0b10000000) { //Test if front bit is dot (0) or dash(1)
      send_dash();
    } else {
      send_dot();
    }
    if (i == len - 1) {
      send_letter_space();
    } else {
      send_inter();
    }
  }
}

// Only accepts capitalized alphanums
uint8_t encode_to_morse_seq(char ch) {
  uint8_t offset = 0;
  if ((ch > 64) && (ch < 91 )) {
    offset = ch - 'A';
  } else if ((ch > 47) && (ch < 58)) {
    offset = ch - '0' + 26;
  } else if (ch == 32) {
    offset = 36;
  }
  return pgm_read_byte_near(morse_enc + offset);
}

void insert_array(char_array * a, char element) {
  a->size++;
  a->array = (char*) realloc(a->array, a->size); //Increase array size
  a->array[a->size - 1] = element;
}

void flush_array(char_array * a) {
  free(a->array); //Clear array
  a->array = NULL;
  a->size = 0;
}

void send_morse_string() {
  //Print normal string
  for (int i = 0; i < serial_buffer.size; i++) {
#ifdef DEBUG_MODE
    Serial.print(serial_buffer.array[i]);
#endif
  }
#ifdef DEBUG_MODE
  Serial.println('\n');
#endif

  //Send/Print Morse Code
  for (int i = 0; i < serial_buffer.size; i++) {
    send_morse_letter(encode_to_morse_seq(serial_buffer.array[i]));
  }
#ifdef DEBUG_MODE
  Serial.print('\n');
#endif
  flush_array(&serial_buffer);
}

void send_letter_space() {
  send_code(LETTER_SPACE_TIME, DEBUG_LETTER_SPACE_CHAR, LOW);
}

void send_word_space() {
  send_code(WORD_SPACE_TIME, DEBUG_WORD_SPACE_CHAR, LOW);
}

void send_dash() {
  send_code(DASH_TIME, DEBUG_DASH_CHAR, HIGH);
}

void send_dot() {
  send_code(DOT_TIME, DEBUG_DOT_CHAR, HIGH);
}

void send_inter() {
  send_code(INTER_TIME, NULL, LOW);
}

void send_code(uint16_t time_delay, char ch, uint8_t pin_state) {
  if (ch != NULL) {
#ifdef DEBUG_MODE
    Serial.print(ch);
#endif
  }
#ifdef TIMED_MODE
  digitalWrite(LED_OUT, pin_state);
  delay(time_delay);
#endif
}

boolean is_morse_char(char ch) { //Checks if character is allowed for morse
  if ((ch > 64) && (ch < 91 )) {
    return true;
  } else if (ch > 47 && ch < 58) {
    return true;
  } else if (ch == 32) {
    return true;
  }
  return false;
}

char to_upper_case(char ch) {
  if ((ch > 96) && (ch < 123)) {
    return ch - 32;
  }
  return ch;
}
