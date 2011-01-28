#include "../hd44780_lcd/hd44780_lcd.h"

#include <avr/interrupt.h>

#define BUTTONS_DDR DDRB
#define BUTTONS_PORT PORTB
#define BUTTONS_PIN PINB
#define BUTTONS_LEFT_PIN PB1 
#define BUTTONS_RIGHT_PIN PB2
#define BUTTONS_UP_PIN PB3
#define BUTTONS_DOWN_PIN PB4
#define BUTTONS_SELECT_PIN PB5

#define BUTTON_PIN_CHANGE_INTERRUPT_ROUTINE_VECTOR SIG_PIN_CHANGE0
#define BUTTON_PIN_CHANGE_INTERRUPT_ENABLE_FLAG PCIE0
#define BUTTON_PIN_CHANGE_INTERRUPT_MASK_REGISTER PCMSK0
#define BUTTON_LEFT_PIN_CHANGE_INTERRUPT_FLAG PCINT1
#define BUTTON_RIGHT_PIN_CHANGE_INTERRUPT_FLAG PCINT2
#define BUTTON_UP_PIN_CHANGE_INTERRUPT_FLAG PCINT3
#define BUTTON_DOWN_PIN_CHANGE_INTERRUPT_FLAG PCINT4
#define BUTTON_SELECT_PIN_CHANGE_INTERRUPT_FLAG PCINT5

volatile int is_player_ones_turn = 0;
volatile int player_ones_moves = 0;
volatile int player_twos_moves = 0;
volatile int can_move_board_position = 0;
volatile int current_board_position_column = 0;
volatile int current_board_position_row = 0;

void write_board_position_value(int bit) {
  if(player_ones_moves & (1<<bit)) 
    lcd_write_string(PSTR("X"));
  else if(player_twos_moves & (1<<bit)) 
    lcd_write_string(PSTR("O"));
  else 
    lcd_write_string(PSTR("_"));
}

void refresh_board() {
  lcd_goto_position(1, 9);
  write_board_position_value(0);
  lcd_goto_position(1, 10);
  write_board_position_value(1);
  lcd_goto_position(1, 11);
  write_board_position_value(2);
  lcd_goto_position(2, 9);
  write_board_position_value(3);
  lcd_goto_position(2, 10);
  write_board_position_value(4);
  lcd_goto_position(2, 11);
  write_board_position_value(5);
  lcd_goto_position(3, 9);
  write_board_position_value(6);
  lcd_goto_position(3, 10);
  write_board_position_value(7);
  lcd_goto_position(3, 11);
  write_board_position_value(8);
}

void move_cursor_to_board_position(int row, int column){
  int lcd_column = 9;
  if(column == 2) lcd_column = 10;
  if(column == 3) lcd_column = 11;
  lcd_goto_position(row, lcd_column);
  current_board_position_row = row;
  current_board_position_column = column;
}

int move_cursor_to_boards_next_free_position() {
  int both_players_moves = player_ones_moves | player_twos_moves;
  if(!(both_players_moves & (1<<0))) { move_cursor_to_board_position(1, 1); return 1; }
  else if(!(both_players_moves & (1<<1))) { move_cursor_to_board_position(1, 2); return 1; }
  else if(!(both_players_moves & (1<<2))) { move_cursor_to_board_position(1, 3); return 1; }
  else if(!(both_players_moves & (1<<3))) { move_cursor_to_board_position(2, 1); return 1; }
  else if(!(both_players_moves & (1<<4))) { move_cursor_to_board_position(2, 2); return 1; }
  else if(!(both_players_moves & (1<<5))) { move_cursor_to_board_position(2, 3); return 1; }
  else if(!(both_players_moves & (1<<6))) { move_cursor_to_board_position(3, 1); return 1; }
  else if(!(both_players_moves & (1<<7))) { move_cursor_to_board_position(3, 2); return 1; }
  else if(!(both_players_moves & (1<<8))) { move_cursor_to_board_position(3, 3); return 1; }
  return 0;
}

int check_if_player_has_won(int players_moves){
  if((players_moves & 0b111) == 0b111) return 1;
  if((players_moves & 0b111000) == 0b111000) return 1;
  if((players_moves & 0b111000000) == 0b111000000) return 1;
  if((players_moves & 0b001001001) == 0b001001001) return 1;
  if((players_moves & 0b010010010) == 0b010010010) return 1;
  if((players_moves & 0b100100100) == 0b100100100) return 1;
  if((players_moves & 0b100010001) == 0b100010001) return 1;
  if((players_moves & 0b001010100) == 0b001010100) return 1;
  return 0;
}

void update_game_status() {
  is_player_ones_turn = 1 - is_player_ones_turn;
  
  if(is_player_ones_turn) {
    lcd_goto_position(4, 1);
    lcd_write_string(PSTR("> Player one's turn ")); 
  }
  else {
    lcd_goto_position(4, 1);
    lcd_write_string(PSTR("> Player two's turn "));
  }
  
  if(check_if_player_has_won(player_ones_moves)){
    lcd_goto_position(4, 1);
    lcd_write_string(PSTR("> Player one WON!   "));
    lcd_turn_blinking_cursor_off();
  }
  else if(check_if_player_has_won(player_twos_moves)){
    lcd_goto_position(4, 1);
    lcd_write_string(PSTR("> Player two WON!   "));
    lcd_turn_blinking_cursor_off();
  }
  else {
    if(!move_cursor_to_boards_next_free_position()){
      lcd_goto_position(4, 1);
      lcd_write_string(PSTR("> Its a draw!       "));
      lcd_turn_blinking_cursor_off();
    }
    else {
     lcd_turn_blinking_cursor_on();
     can_move_board_position = 1;
    }
  }
}

void display_initial_message() {
  lcd_goto_position(1, 1);
  lcd_write_string(PSTR("        TIC        "));
  _delay_ms(400);
  lcd_goto_position(2, 1);
  lcd_write_string(PSTR("        TAC        "));
  _delay_ms(400);
  lcd_goto_position(3, 1);
  lcd_write_string(PSTR("        TOE        "));
}

void move_down(){
  int temp_row = current_board_position_row + 1;
  if(temp_row > 3) temp_row = 1;
  move_cursor_to_board_position(temp_row, current_board_position_column);
}

void move_up(){
  int temp_row = current_board_position_row - 1;
  if(temp_row < 1) temp_row = 3;
  move_cursor_to_board_position(temp_row, current_board_position_column);
}

void move_right(){
  int temp_column = current_board_position_column + 1;
  if(temp_column > 3) temp_column = 1;
  move_cursor_to_board_position(current_board_position_row, temp_column);
}

void move_left(){
  int temp_column = current_board_position_column - 1;
  if(temp_column < 1) temp_column = 3;
  move_cursor_to_board_position(current_board_position_row, temp_column);
}

void select_position(){
  int current_pos = current_board_position_column + ((current_board_position_row - 1) * 3) - 1;
  if(is_player_ones_turn) player_ones_moves |= (1<<current_pos);
  else  player_twos_moves |= (1<<current_pos);
  refresh_board();
  update_game_status();
}

ISR(BUTTON_PIN_CHANGE_INTERRUPT_ROUTINE_VECTOR) {
  cli(); // disable interrupts so only one button press is active at one time
  if(can_move_board_position){
    can_move_board_position = 0;
    if(!(BUTTONS_PIN & (1<<BUTTONS_LEFT_PIN))) {
      move_left();
    } else if(!(BUTTONS_PIN & (1<<BUTTONS_RIGHT_PIN))) { 
      move_right();
    } else if(!(BUTTONS_PIN & (1<<BUTTONS_UP_PIN))) {
      move_up(); 
    } else if(!(BUTTONS_PIN & (1<<BUTTONS_DOWN_PIN))) {
      move_down();  
    } else if(!(BUTTONS_PIN & (1<<BUTTONS_SELECT_PIN))) {
      int current_pos = current_board_position_column + ((current_board_position_row - 1) * 3) - 1;
      if(!(player_ones_moves & (1<<current_pos)) && !(player_twos_moves & (1<<current_pos))) {
        select_position(); 
      }
    }
    _delay_ms(100);
    can_move_board_position = 1;
  }
  sei(); //reenable interrupts
}

void initialize_pins(){
  //set button pins as output port and turn on pull up resistor
  BUTTONS_DDR |= (1<<BUTTONS_LEFT_PIN) | (1<<BUTTONS_RIGHT_PIN) | (1<<BUTTONS_UP_PIN) | (1<<BUTTONS_DOWN_PIN) | (1<<BUTTONS_SELECT_PIN);
  BUTTONS_PORT |= (1<<BUTTONS_LEFT_PIN) | (1<<BUTTONS_RIGHT_PIN) | (1<<BUTTONS_UP_PIN) | (1<<BUTTONS_DOWN_PIN) | (1<<BUTTONS_SELECT_PIN);

  //turn on interrupts for buttons pins and allow interrupts for the specific pins
  PCICR |= (1<<BUTTON_PIN_CHANGE_INTERRUPT_ENABLE_FLAG);
  BUTTON_PIN_CHANGE_INTERRUPT_MASK_REGISTER |= (1<<BUTTON_LEFT_PIN_CHANGE_INTERRUPT_FLAG) | (1<<BUTTON_RIGHT_PIN_CHANGE_INTERRUPT_FLAG) | (1<<BUTTON_UP_PIN_CHANGE_INTERRUPT_FLAG) | (1<<BUTTON_DOWN_PIN_CHANGE_INTERRUPT_FLAG) | (1<<BUTTON_SELECT_PIN_CHANGE_INTERRUPT_FLAG);
}

int main() {
  initialize_pins();
  lcd_initialize();
  display_initial_message();
  _delay_ms(500);
  lcd_clear_screen();
  refresh_board();
  update_game_status();
  sei(); //turn on interrupts
  
  while(1) {}
  
  return 0;
}
