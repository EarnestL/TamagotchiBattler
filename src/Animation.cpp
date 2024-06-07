#include "Animation.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

Animation::Animation(){
  scroll = 0;
  enter = false;
  scrolled = false;
  ate = false;
  petted = false;
  prev_select_toolitem = "pet";
  selected_menuscrollitem = "toolbox";
  curr_display = "home";
  selected_menuitem = "battle";
  dotcount = 0;
  battleSelectedItem = "fight";
  battleSelfIdle_frameState = 0;
  battleOppIdle_frameState = 0;
  battleSelfAttack_frameState = 0;
  battleOppAttack_frameState = 0;
}

void Animation::display_init(Pal* pal){
  this->pal = pal;
  randomSeed(esp_random());
  //set up display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    Serial.println(F("SSD136 allocation has failed.."));
  }

  display.clearDisplay();
  display.display();
  for (int i = 0; i < turning_on_frame; i++){
    display.clearDisplay();
    display.drawBitmap(0,0, turning_on_animation_sprite + (TURNING_ON_FRAME_WIDTH*TURNING_ON_FRAME_HEIGHT*i)/8, TURNING_ON_FRAME_WIDTH, TURNING_ON_FRAME_HEIGHT, WHITE);
    display.display();
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
void Animation::display_newgame(int option){
  String header = "Name Selection";
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(header, 0, 0, &x1, &y1, &w, &h);

  String names[4] = {"Carl", "Jake", "Jameson", "Yokota"};

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.clearDisplay();
  display.setCursor((SCREEN_WIDTH-w)/2, 5);
  display.println(header);
  display.drawLine(0, 16, SCREEN_WIDTH-1, 16, WHITE);
  display.setCursor(0, 22);
  if (option == 0){display.print("> ");};
  display.println(names[0]);
  display.setCursor(0, 32);
  if (option == 1){display.print("> ");};
  display.println(names[1]);
  display.setCursor(0, 42);
  if (option == 2){display.print("> ");};
  display.println(names[2]);
  display.setCursor(0, 52);
  if (option == 3){display.print("> ");};
  display.println(names[3]);
  display.display();
}

void Animation::print_dotcount(uint8_t count){
  display.setCursor(60,(SCREEN_HEIGHT/2)-1);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  for (int i = 0; i < count;i++){
    display.print(".");
  }
  display.println("");
}

void Animation::reset_loading_dotcount(){
  this->dotcount = 0;
}

void Animation::clearDisplay(){
  display.clearDisplay();
}

void Animation::display_stats(){
  display.fillRect(0,0,SCREEN_WIDTH-1,15,BLACK);
  int16_t x1, y1;
  uint16_t w, h;

  display.getTextBounds(String(pal->get_name().c_str()), 0, 0, &x1, &y1, &w, &h);

  //name tag
  display.fillRect(0, 0, 8 + w, 15, WHITE);
  display.drawPixel(0,0,BLACK);
  display.drawPixel(0,14,BLACK);
  display.drawPixel(7+w,0,BLACK);
  display.drawPixel(7+w,14,BLACK);
  display.setCursor(4,4);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.println(String(pal->get_name().c_str()));

  //hp bar
  display.drawLine(75, 1, 125, 1, WHITE);
  display.drawLine(74, 2, 126, 2, WHITE);
  display.drawLine(74, 3, 126, 3, WHITE);
  display.drawLine(74, 4, 126, 4, WHITE);
  display.drawLine(75, 5, 125, 5, WHITE);
  display.fillRect(75 + pal->get_hp()/2, 2, (max_health - pal->get_hp())/2, 3, BLACK);

  //hp icon
  display.drawLine(60+3, 0, 61+3, 0, WHITE);  
  display.drawLine(64+3, 0, 65+3, 0, WHITE);  
  display.drawLine(59+3, 1, 66+3, 1, WHITE);
  display.drawLine(59+3, 2, 66+3, 2, WHITE);
  display.drawLine(59+3, 3, 66+3, 3, WHITE);
  display.drawLine(60+3, 4, 65+3, 4, WHITE);
  display.drawLine(61+3, 5, 64+3, 5, WHITE);
  display.drawLine(62+3, 6, 63+3, 6, WHITE);

  //hunger bar
  display.drawLine(75, 1+9, 125, 1+9, WHITE);
  display.drawLine(74, 2+9, 126, 2+9, WHITE);
  display.drawLine(74, 3+9, 126, 3+9, WHITE);
  display.drawLine(74, 4+9, 126, 4+9, WHITE);
  display.drawLine(75, 5+9, 125, 5+9, WHITE);
  display.fillRect(75 + pal->get_hunger()/2, 2+9, (max_hunger - pal->get_hunger())/2, 3, BLACK);

  //hunger icon
  display.drawLine(65+3, 0+10, 65+3, 4+10, WHITE);  
  display.drawLine(64+3, 1+10, 64+3, 2+10, WHITE);  
  display.drawLine(60+3, 0+10, 60+3, 2+10, WHITE);  
  display.drawLine(62+3, 0+10, 62+3, 2+10, WHITE);  
  display.drawLine(61+3, 2+10, 61+3, 4+10, WHITE);

  display.display();
}

void Animation::pal_wiggle(){
  for (int i = 0; i < pal_wiggle_frame; i++){
    clear_pal();
    display.drawBitmap((SCREEN_WIDTH-PAL_FRAME_WIDTH)/2 - 1, 15, pal_wiggle_sprite + (PAL_FRAME_WIDTH*PAL_FRAME_HEIGHT*i)/8, 24, 48, WHITE);
    display.display();
    vTaskDelay(pdMS_TO_TICKS(100));
    if(curr_display == "menu" ){
      return;
    }
  }
}

void Animation::pal_jump(){
  for (int i = 0; i < pal_jump_frame; i++){

    clear_pal();
    display.drawBitmap((SCREEN_WIDTH-PAL_FRAME_WIDTH)/2 - 1, 15, pal_jump_sprite + (PAL_FRAME_WIDTH*PAL_FRAME_HEIGHT*i)/8, 24, 48, WHITE);
    display.display();
    vTaskDelay(pdMS_TO_TICKS(100));
    if(curr_display == "menu" ){
      return;
    }
  }
}

void Animation::pal_blink(){
  for (int i = 0; i < pal_blink_frame; i++){
    clear_pal();
    display.drawBitmap((SCREEN_WIDTH-PAL_FRAME_WIDTH)/2 - 1, 15, pal_blink_sprite + (PAL_FRAME_WIDTH*PAL_FRAME_HEIGHT*i)/8, 24, 48, WHITE);
    display.display();
    vTaskDelay(pdMS_TO_TICKS(100));
    if(curr_display == "menu" ){
      return;
    }
  }
}

void Animation::pal_eating(){
  for (int i = 0; i < pal_eating_frame; i++){
    clear_pal();
    display.drawBitmap((SCREEN_WIDTH-PAL_FRAME_WIDTH)/2 - 1, 15, pal_eating_sprite + (PAL_FRAME_WIDTH*PAL_FRAME_HEIGHT*i)/8, 24, 48, WHITE);
    display.display();
    vTaskDelay(pdMS_TO_TICKS(100));
    if(curr_display == "menu" ){
      return;
    }
  }
}

void Animation::pal_petting(){
  for (int i = 0; i < pal_squashed_frame; i++){
    clear_pal();
    display.drawBitmap((SCREEN_WIDTH-PAL_FRAME_WIDTH)/2 - 1, 15, pal_squashed_sprite + (PAL_FRAME_WIDTH*PAL_FRAME_HEIGHT*i)/8, 24, 48, WHITE);
    display.display();
    vTaskDelay(pdMS_TO_TICKS(100));
    if(curr_display == "menu" ){
      return;
    }
  }
}

void Animation::pal_refusing(){
  for (int i = 0; i < pal_refusing_frame; i++){
    clear_pal();
    display.drawBitmap((SCREEN_WIDTH-PAL_FRAME_WIDTH)/2 - 1, 15, pal_refusing_sprite + (PAL_FRAME_WIDTH*PAL_FRAME_HEIGHT*i)/8, 24, 48, WHITE);
    display.display();
    vTaskDelay(pdMS_TO_TICKS(100));
    if(curr_display == "menu" ){
      return;
    }
  }
}

void Animation::start_transition_animation(){
  display.clearDisplay();
  display.drawLine(0, 32 - 2, SCREEN_WIDTH - 1, 32 - 2, WHITE);
  display.display();
  delay(100);

  display.clearDisplay();
  display.drawBitmap((SCREEN_WIDTH-PAL_FRAME_WIDTH)/2 - 1, 15, pal_default_still, PAL_FRAME_WIDTH, PAL_FRAME_HEIGHT, WHITE);
  display.drawBitmap(menuscroll_x, menuscroll_y, menuscroll_off_still, MENUSCROLL_FRAME_WIDTH, MENUSCROLL_FRAME_HEIGHT, WHITE);
  display.fillRect(0, 0, SCREEN_WIDTH - 1, 32 - 7, BLACK);
  display.drawLine(0, 32 - 7, SCREEN_WIDTH - 1, 32 - 7, WHITE);
  display.fillRect(0, 32 + 7, SCREEN_WIDTH - 1, 64 - 32 + 7, BLACK);
  display.drawLine(0, 32 + 7, SCREEN_WIDTH - 1, 32 + 7, WHITE);
  display.display();
  delay(100);

  display.clearDisplay();
  display.drawBitmap((SCREEN_WIDTH-PAL_FRAME_WIDTH)/2 - 1, 15, pal_default_still, PAL_FRAME_WIDTH, PAL_FRAME_HEIGHT, WHITE);
  display.drawBitmap(menuscroll_x, menuscroll_y, menuscroll_off_still, MENUSCROLL_FRAME_WIDTH, MENUSCROLL_FRAME_HEIGHT, WHITE);
  display.fillRect(0, 0, SCREEN_WIDTH - 1, 32 - 7 - 10, BLACK);
  display.drawLine(0, 32 - 7 - 10, SCREEN_WIDTH - 1, 32 - 7 - 10, WHITE);
  display.fillRect(0, 32 + 7 + 10, SCREEN_WIDTH - 1, 64 - 32 + 7 + 10, BLACK);
  display.drawLine(0, 32 + 7 + 10, SCREEN_WIDTH - 1, 32 + 7 + 10, WHITE);
  display.display();
  delay(100);

  display.clearDisplay();
  display.drawBitmap((SCREEN_WIDTH-PAL_FRAME_WIDTH)/2 - 1, 15, pal_default_still, PAL_FRAME_WIDTH, PAL_FRAME_HEIGHT, WHITE);
  display.drawBitmap(menuscroll_x, menuscroll_y, menuscroll_off_still, MENUSCROLL_FRAME_WIDTH, MENUSCROLL_FRAME_HEIGHT, WHITE);

    display.fillRect(0,0,SCREEN_WIDTH-1,15,BLACK);
  int16_t x1, y1;
  uint16_t w, h;

  display.getTextBounds(String(pal->get_name().c_str()), 0, 0, &x1, &y1, &w, &h);

  //name tag
  display.fillRect(0, 0, 8 + w, 15, WHITE);
  display.drawPixel(0,0,BLACK);
  display.drawPixel(0,14,BLACK);
  display.drawPixel(7+w,0,BLACK);
  display.drawPixel(7+w,14,BLACK);
  display.setCursor(4,4);
  display.setTextColor(BLACK);
  display.println(String(pal->get_name().c_str()));

  //hp bar
  display.drawLine(75, 1, 125, 1, WHITE);
  display.drawLine(74, 2, 126, 2, WHITE);
  display.drawLine(74, 3, 126, 3, WHITE);
  display.drawLine(74, 4, 126, 4, WHITE);
  display.drawLine(75, 5, 125, 5, WHITE);
  display.fillRect(75 + pal->get_hp()/2, 2, (max_health - pal->get_hp())/2, 3, BLACK);

  //hp icon
  display.drawLine(60+3, 0, 61+3, 0, WHITE);  
  display.drawLine(64+3, 0, 65+3, 0, WHITE);  
  display.drawLine(59+3, 1, 66+3, 1, WHITE);
  display.drawLine(59+3, 2, 66+3, 2, WHITE);
  display.drawLine(59+3, 3, 66+3, 3, WHITE);
  display.drawLine(60+3, 4, 65+3, 4, WHITE);
  display.drawLine(61+3, 5, 64+3, 5, WHITE);
  display.drawLine(62+3, 6, 63+3, 6, WHITE);

  //hunger bar
  display.drawLine(75, 1+9, 125, 1+9, WHITE);
  display.drawLine(74, 2+9, 126, 2+9, WHITE);
  display.drawLine(74, 3+9, 126, 3+9, WHITE);
  display.drawLine(74, 4+9, 126, 4+9, WHITE);
  display.drawLine(75, 5+9, 125, 5+9, WHITE);
  display.fillRect(75 + pal->get_hunger()/2, 2+9, (max_hunger - pal->get_hunger())/2, 3, BLACK);

  //hunger icon
  display.drawLine(65+3, 0+10, 65+3, 4+10, WHITE);  
  display.drawLine(64+3, 1+10, 64+3, 2+10, WHITE);  
  display.drawLine(60+3, 0+10, 60+3, 2+10, WHITE);  
  display.drawLine(62+3, 0+10, 62+3, 2+10, WHITE);  
  display.drawLine(61+3, 2+10, 61+3, 4+10, WHITE);

  display.fillRect(0, 0, SCREEN_WIDTH - 1, 32 - 7 - 10 - 13, BLACK);
  display.drawLine(0, 32 - 7 - 10 - 13, SCREEN_WIDTH - 1, 32 - 7 - 10 - 13, WHITE);
  display.fillRect(0, 32 + 7 + 10 + 13, SCREEN_WIDTH - 1, 64 - (32 + 7 + 10 + 13), BLACK);
  display.drawLine(0, 32 + 7 + 10 + 13, SCREEN_WIDTH - 1, 32 + 7 + 10 + 13, WHITE);
  display.display();
  delay(100);
  display.clearDisplay();
}

void Animation::pal_display(){
  if (random(0,90) == 0){
    pal_jump();
  }
  else if (random(0,25) == 0){
    pal_wiggle();
  }
  else if (random(0,25) == 0){
    pal_blink();
  }
  else{
    clear_pal();
    display.drawBitmap((SCREEN_WIDTH-PAL_FRAME_WIDTH)/2 - 1, 15, pal_default_still, 24, 48, WHITE);
    display.display();
  }

  if(ate){
    if (pal->get_hunger() == 0){
      pal_refusing();
    }
    else{
      pal_eating();
    }
    ate = 0;
  }

  if(petted){
    pal_petting();
    petted = 0;
  }
}

void Animation::print_toolbox_item(int8_t x, int8_t y, string item){
  if (item == "pet"){
    display.drawBitmap(SCREEN_WIDTH - 1 - MENU_FRAME_WIDTH + x, 16 + y, pet_icon_still, 40, 40, WHITE);
  }
  else if (item == "food"){
    display.drawBitmap(SCREEN_WIDTH - 1 - MENU_FRAME_WIDTH + x, 16 + y, food_icon_still, 40, 40, WHITE);
  }
}

void Animation::print_menuscroll_toolbox(int8_t x, int8_t y){
  if (selected_menuscrollitem == "toolbox"){
    display.drawBitmap(menuscroll_x + x, menuscroll_y + y, toolIcon_selected, MENUSCROLL_FRAME_WIDTH, MENUSCROLL_FRAME_HEIGHT, WHITE);
  }
  else if (selected_menuscrollitem == "menu"){
    display.drawBitmap(menuscroll_x + x, menuscroll_y + y, toolIcon_still, MENUSCROLL_FRAME_WIDTH, MENUSCROLL_FRAME_HEIGHT, WHITE);
  }
}
void Animation::print_menuscroll_setting(int8_t x, int8_t y){
  if (selected_menuscrollitem == "toolbox"){
    display.drawBitmap(menuscroll_x + x, menuscroll_y + y, menuIcon_still, MENUSCROLL_FRAME_WIDTH, MENUSCROLL_FRAME_HEIGHT, WHITE);
  }
  else if (selected_menuscrollitem == "menu"){
    display.drawBitmap(menuscroll_x + x, menuscroll_y + y, menuIcon_selected, MENUSCROLL_FRAME_WIDTH, MENUSCROLL_FRAME_HEIGHT, WHITE);
  }
}

void Animation::toolbox_display(){

  string options[2] = {"pet", "food"};
  int8_t options_size = 2;

  if(curr_display == "toolbox"){
    if(scroll>=2){
      scroll = 0;
    }

    if(options[scroll] == "pet"){
      prev_select_toolitem = "pet";
      if(scrolled){
        toolbox_scrolled_animation();
        scrolled = false;
      }
      clear_toolbox();
      print_toolbox_item(0, 0, "pet");
      display.drawBitmap(SCREEN_WIDTH - 1 - MENU_FRAME_WIDTH, 16, toolbox_still, 40, 40, WHITE);
      display.display();
      if (enter){
        toolbox_entered_animation();
        petted = true;
        Serial.println("Petted!");
        enter = false;
      }
    }
    else if (options[scroll] == "food"){
      prev_select_toolitem = "food";
      if(scrolled){
        toolbox_scrolled_animation();
        scrolled = false;
      }
      clear_toolbox();
      print_toolbox_item(0, 0, "food");
      display.drawBitmap(SCREEN_WIDTH - 1 - MENU_FRAME_WIDTH, 16, toolbox_still, 40, 40, WHITE);
      display.display();
      if (enter){
        toolbox_entered_animation();
        ate = true;
        pal->change_hunger(-6);
        Serial.println("Fed!");
        enter = false;
      }
    }
  }

  vTaskDelay(pdMS_TO_TICKS(100));

}

void Animation::menuscroll_on_display(){
  clear_menuscroll();
  if (scrolled && curr_display == "menuscroll"){
    if (selected_menuscrollitem == "toolbox"){
      selected_menuscrollitem = "menu";
    }
    else{
      selected_menuscrollitem = "toolbox";
    }
    scrolled = false;
    scroll = 0;
  }

  if(enter && curr_display == "menuscroll"){
    if (selected_menuscrollitem == "toolbox"){
      curr_display = selected_menuscrollitem;
    }
    else if (selected_menuscrollitem == "menu"){
      curr_display = selected_menuscrollitem;
    }
    menuscroll_items_entered_animation();
    enter = false;
  }
  print_menuscroll_toolbox(0, 0);
  print_menuscroll_setting(0, 0);
  display.drawBitmap(menuscroll_x, menuscroll_y, menuscroll_on_still, MENUSCROLL_FRAME_WIDTH, MENUSCROLL_FRAME_HEIGHT, WHITE);
  display.display();
  vTaskDelay(pdMS_TO_TICKS(100));
}

void Animation::menuscroll_off_display(){
  clear_menuscroll();
  display.drawBitmap(menuscroll_x, menuscroll_y, menuscroll_off_still, MENUSCROLL_FRAME_WIDTH, MENUSCROLL_FRAME_HEIGHT, WHITE);
  display.display();
  vTaskDelay(pdMS_TO_TICKS(100));
}

void Animation::menuscroll_opening_animation(){
  int8_t toolbox_movement[menuscroll_opening_frame] = {-30, -12, 2, -1};
  int8_t setting_movement[menuscroll_opening_frame] = {-30, -12, 2, -1};
  for (int i = 0; i < menuscroll_opening_frame; i++){
    clear_menuscroll();
    print_menuscroll_toolbox(0, toolbox_movement[i]);
    print_menuscroll_setting(0, setting_movement[i]);
    display.drawBitmap(menuscroll_x, menuscroll_y, menuscroll_opening_sprite + (MENUSCROLL_FRAME_WIDTH*MENUSCROLL_FRAME_HEIGHT*i)/8, MENUSCROLL_FRAME_WIDTH, MENUSCROLL_FRAME_HEIGHT, WHITE);
    display.display();
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void Animation::set_selected_menuscrollitem(string item){
  this->selected_menuscrollitem = item;
}

string Animation::get_selected_menuscrollitem(){
  return this->selected_menuscrollitem;
}

void Animation::menuscroll_closing_animation(){
  int8_t toolbox_movement[menuscroll_closing_frame] = {1, 3, -64, -64, -64, -64};
  int8_t setting_movement[menuscroll_closing_frame] = {1, 3, -21, -64, -64, -64};
  for (int i = 0; i < menuscroll_closing_frame; i++){
    clear_menuscroll();
    print_menuscroll_toolbox(0, toolbox_movement[i]);
    print_menuscroll_setting(0, setting_movement[i]);
    display.drawBitmap(menuscroll_x, menuscroll_y, menuscroll_closing_sprite + (MENUSCROLL_FRAME_WIDTH*MENUSCROLL_FRAME_HEIGHT*i)/8, MENUSCROLL_FRAME_WIDTH, MENUSCROLL_FRAME_HEIGHT, WHITE);
    display.display();
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void Animation::clear_pal(){
  display.fillRect((SCREEN_WIDTH-PAL_FRAME_WIDTH)/2 - 1, 16, 24, 48, BLACK);
}

void Animation::clear_toolbox(){
  display.fillRect(SCREEN_WIDTH - 1 - MENU_FRAME_WIDTH, 16, 42, 40, BLACK);
}

void Animation::scrollexecute(){
  scroll++;
  scrolled = true;
}

void Animation::scrollreset(){
  scroll = 0;
}

int8_t Animation::getscroll(){
  return scroll;
}

void Animation::enterexecute(){
  enter = true;
}

void Animation::toolbox_scrolled_animation(){
  display.drawBitmap(SCREEN_WIDTH - 1 - MENU_FRAME_WIDTH, 16, toolbox_still, 40, 40, BLACK);
  display.drawBitmap(SCREEN_WIDTH - 1 - MENU_FRAME_WIDTH, 16, toolbox_scrolled, 40, 40, WHITE);
  display.display();
}

void Animation::toolbox_entered_animation(){
  display.drawBitmap(SCREEN_WIDTH - 1 - MENU_FRAME_WIDTH, 16, toolbox_still, 40, 40, BLACK);
  display.drawBitmap(SCREEN_WIDTH - 1 - MENU_FRAME_WIDTH, 16, toolbox_entered, 40, 40, WHITE);
  display.display();
  vTaskDelay(pdMS_TO_TICKS(50));
  display.drawBitmap(SCREEN_WIDTH - 1 - MENU_FRAME_WIDTH, 16, toolbox_entered, 40, 40, BLACK);
  display.drawBitmap(SCREEN_WIDTH - 1 - MENU_FRAME_WIDTH, 16, toolbox_still, 40, 40, WHITE);
  display.display();
}

void Animation::toolbox_emerging_animation(){
  int8_t menu_movement[menu_emerging_frame] = {45,18,-3,2,-1};
  for (int i = 0; i < menu_emerging_frame; i++){
    clear_toolbox();
    print_toolbox_item(menu_movement[i], 0, prev_select_toolitem);
    display.drawBitmap(SCREEN_WIDTH - 1 - MENU_FRAME_WIDTH, 16, toolbox_emerging_sprite + (MENU_FRAME_WIDTH*MENU_FRAME_HEIGHT*i)/8, 40, 40, WHITE);
    display.display();
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void Animation::toolbox_leaving_animation(){
  int8_t menu_movement[menu_leaving_frame] = {-1, 5, 26, 45};
  for (int i = 0; i < menu_leaving_frame; i++){
    clear_toolbox();
    print_toolbox_item(menu_movement[i], 0, prev_select_toolitem);
    display.drawBitmap(SCREEN_WIDTH - 1 - MENU_FRAME_WIDTH, 16, toolbox_leaving_sprite + (MENU_FRAME_WIDTH*MENU_FRAME_HEIGHT*i)/8, 40, 40, WHITE);
    display.display();
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void Animation::clear_menuscroll(){
  display.fillRect(menuscroll_x, menuscroll_y-1, MENUSCROLL_FRAME_WIDTH, MENUSCROLL_FRAME_HEIGHT, BLACK);
}

string Animation::get_curr_display(){
  return this->curr_display;
}
void Animation::set_curr_display(string display){
  this->curr_display = display;
}

void Animation::menuscroll_items_entered_animation(){
  if (selected_menuscrollitem == "toolbox"){
    display.drawBitmap(menuscroll_x, menuscroll_y, toolIcon_entered_modified, MENUSCROLL_FRAME_WIDTH, MENUSCROLL_FRAME_HEIGHT, WHITE);
    display.drawBitmap(menuscroll_x, menuscroll_y, menuIcon_still, MENUSCROLL_FRAME_WIDTH, MENUSCROLL_FRAME_HEIGHT, WHITE);
    display.drawBitmap(menuscroll_x, menuscroll_y, menuscroll_on_still, MENUSCROLL_FRAME_WIDTH, MENUSCROLL_FRAME_HEIGHT, WHITE);
    display.display();
    vTaskDelay(pdMS_TO_TICKS(25));
    display.drawBitmap(menuscroll_x, menuscroll_y, toolIcon_entered_modified, MENUSCROLL_FRAME_WIDTH, MENUSCROLL_FRAME_HEIGHT, BLACK);
    display.drawBitmap(menuscroll_x, menuscroll_y, toolIcon_selected, MENUSCROLL_FRAME_WIDTH, MENUSCROLL_FRAME_HEIGHT, WHITE);
    display.display();
  }
  else if (selected_menuscrollitem == "menu"){
    display.drawBitmap(menuscroll_x, menuscroll_y, toolIcon_still, MENUSCROLL_FRAME_WIDTH, MENUSCROLL_FRAME_HEIGHT, WHITE);
    display.drawBitmap(menuscroll_x, menuscroll_y, menuIcon_entered, MENUSCROLL_FRAME_WIDTH, MENUSCROLL_FRAME_HEIGHT, WHITE);
    display.drawBitmap(menuscroll_x, menuscroll_y, menuscroll_on_still, MENUSCROLL_FRAME_WIDTH, MENUSCROLL_FRAME_HEIGHT, WHITE);
    display.display();
    vTaskDelay(pdMS_TO_TICKS(25));
    display.drawBitmap(menuscroll_x, menuscroll_y, menuIcon_entered, MENUSCROLL_FRAME_WIDTH, MENUSCROLL_FRAME_HEIGHT, BLACK);
    display.drawBitmap(menuscroll_x, menuscroll_y, menuIcon_selected, MENUSCROLL_FRAME_WIDTH, MENUSCROLL_FRAME_HEIGHT, WHITE);
    display.display();
  }

}

void Animation::menu_display(int16_t x, int16_t y){
  if(curr_display == "menu" && scrolled){
    if(selected_menuitem == "battle"){
      selected_menuitem = "exit";
    }
    else{
      selected_menuitem = "battle";
    }
    scroll = 0;
    scrolled = false;
  }

  display.fillRect(0+x, y, SCREEN_WIDTH, SCREEN_HEIGHT-1-y, WHITE);
  display.setCursor(52+x, 4+y);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.println("MENU");
  display.drawLine(0+x, 15 + y, SCREEN_WIDTH - 1, 15 + y, BLACK);
  display.drawPixel(0+x, 0+y, BLACK);
  display.drawPixel(SCREEN_WIDTH - 1+x, 0+y, BLACK);
  display.drawPixel(0+x, 14+y, BLACK);
  display.drawPixel(SCREEN_WIDTH - 1+x, 14+y, BLACK);
  display.drawPixel(0+x, 16+y, BLACK);
  display.drawPixel(SCREEN_WIDTH - 1+x, 16+y, BLACK);
  display.drawPixel(0+x, SCREEN_HEIGHT - 2+y, BLACK);
  display.drawPixel(SCREEN_WIDTH - 1+x, SCREEN_HEIGHT - 2+y, BLACK);

  if(selected_menuitem == "battle"){
    display.fillRoundRect(42+x,22+y,42,15,3,BLACK);
    display.drawRoundRect(42+x,42+y,42,15,3,BLACK);
    display.setCursor(42 + x + 4, 22 + y + 4);
    display.setTextColor(WHITE);
    display.println("Battle");
    display.setTextColor(BLACK);
    display.setCursor(42 + x + 10, 42 + y + 4);
    display.println("Exit");  
  }
  else if(selected_menuitem == "exit"){
    display.drawRoundRect(42+x,22+y,42,15,3,BLACK);
    display.fillRoundRect(42+x,42+y,42,15,3,BLACK);
    display.setCursor(42 + x + 4, 22 + y + 4);
    display.setTextColor(BLACK);
    display.println("Battle");
    display.setTextColor(WHITE);
    display.setCursor(42 + x + 10, 42 + y + 4);
    display.println("Exit");  
  }
  display.display();
}

void Animation::detecting_display(int16_t x, int16_t y){
  display.fillRect(x, y, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);
  display.fillRoundRect(x+3, y, SCREEN_WIDTH-(3*2), 15, 2, WHITE);
  display.setCursor(32+x, 4+y);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  if (x < SCREEN_WIDTH-80){
    display.println("DETECTING");
  }
  else if (x < SCREEN_WIDTH-30){
    display.println("DETECTING");
  }
}

void Animation::print_peer(int16_t y, string name, bool selected, bool battle_attempt, bool challenging){
  display.setTextSize(1);
  display.setTextColor(WHITE);
  if(selected){
    display.drawRoundRect(4, 22 + y*15, 48, 13, 3, WHITE);
    display.setCursor(9, 25 + y*15);
    display.print(String(name.c_str()));

    display.fillRoundRect(55, 22 + y*15, 69, 13, 3, WHITE);
    display.setTextColor(BLACK);
    if (battle_attempt){
      display.setCursor(75, 25 + y*15);
      display.print("FIGHT");
    }
    else if(!challenging){
      display.setCursor(63, 25 + y*15);
      display.print("Challenge");
    } 
    else if(challenging){
      loading_dots_black(80, 25 + y*15);
    }
  }
  else{
    display.drawRoundRect(6, 22 + y*15, 53, 13, 3, WHITE);
    display.setCursor(11, 25 + y*15);
    display.print(String(name.c_str()));

    display.drawRoundRect(61, 22 + y*15, 61, 13, 3, WHITE);
    display.setTextColor(WHITE);
    if (battle_attempt){
      display.setCursor(77, 25 + y*15);
      display.print("FIGHT");
    }
    else if(!challenging){
      display.setCursor(65, 25 + y*15);
      display.print("Challenge");
    } 
    else if(challenging){
      loading_dots_white(82, 25 + y*15);
    }
  }
  display.println("");
}

void Animation::set_placement(){
  display.setCursor(4, 20);
  display.setTextColor(WHITE);
  display.setTextSize(1);
}

void Animation::display_num(uint8_t i){
  display.println(String(i));
}

void Animation::display_all(){
  display.display();
}

string Animation::get_selected_menuitem(){
  return this->selected_menuitem;
}

bool Animation::palBattleDisplay(bool selfAttack, bool oppAttack){
  display.fillRect(0, 16, 64, 24, BLACK);
  display.fillRect(64, 0, 64, 24, BLACK);
  if(!selfAttack && !oppAttack){
    display.drawBitmap(0, 16, battleSelfIdle_sprite + (BATTLE_PAL_FRAME_WIDTH*BATTLE_PAL_FRAME_HEIGHT*battleSelfIdle_frameState)/8, BATTLE_PAL_FRAME_WIDTH, BATTLE_PAL_FRAME_HEIGHT, WHITE);
    display.drawBitmap(64, 0, battleOppIdle_sprite + (BATTLE_PAL_FRAME_WIDTH*BATTLE_PAL_FRAME_HEIGHT*battleOppIdle_frameState)/8, BATTLE_PAL_FRAME_WIDTH, BATTLE_PAL_FRAME_HEIGHT, WHITE);
    battleSelfIdle_frameState++;
    battleOppIdle_frameState++;
  }
  else if(oppAttack){
    display.drawBitmap(64, 0, battleOppAttack_sprite + (BATTLE_PAL_FRAME_WIDTH*BATTLE_PAL_FRAME_HEIGHT*battleOppAttack_frameState)/8, BATTLE_PAL_FRAME_WIDTH, BATTLE_PAL_FRAME_HEIGHT, WHITE);
    if(battleOppAttack_frameState>=5){
      display.drawBitmap(0, 16, battleSelfDamaged_sprite + (BATTLE_PAL_FRAME_WIDTH*BATTLE_PAL_FRAME_HEIGHT*battleSelfDamaged_frameState)/8, BATTLE_PAL_FRAME_WIDTH, BATTLE_PAL_FRAME_HEIGHT, WHITE);
      battleSelfDamaged_frameState++;
    }
    else{
      display.drawBitmap(0, 16, battleSelfIdle_sprite + (BATTLE_PAL_FRAME_WIDTH*BATTLE_PAL_FRAME_HEIGHT*battleSelfIdle_frameState)/8, BATTLE_PAL_FRAME_WIDTH, BATTLE_PAL_FRAME_HEIGHT, WHITE);
      battleSelfIdle_frameState++;
    }
    battleOppAttack_frameState++;
    if(battleOppAttack_frameState>=battleOppAttack_frame){
      battleOppIdle_frameState = 0;
      battleSelfIdle_frameState = 0;
      battleOppAttack_frameState = 0;
      battleSelfAttack_frameState = 0;
      battleOppDamaged_frameState = 0;
      battleSelfDamaged_frameState = 0;
      return true;
    }
  }
  else if(selfAttack){
    display.drawBitmap(0, 16, battleSelfAttack_sprite + (BATTLE_PAL_FRAME_WIDTH*BATTLE_PAL_FRAME_HEIGHT*battleSelfAttack_frameState)/8, BATTLE_PAL_FRAME_WIDTH, BATTLE_PAL_FRAME_HEIGHT, WHITE);
    if(battleSelfAttack_frameState>=5){
      display.drawBitmap(64, 0, battleOppDamaged_sprite + (BATTLE_PAL_FRAME_WIDTH*BATTLE_PAL_FRAME_HEIGHT*battleOppDamaged_frameState)/8, BATTLE_PAL_FRAME_WIDTH, BATTLE_PAL_FRAME_HEIGHT, WHITE);
      battleOppDamaged_frameState++;
    }
    else{
      display.drawBitmap(64, 0, battleOppIdle_sprite + (BATTLE_PAL_FRAME_WIDTH*BATTLE_PAL_FRAME_HEIGHT*battleOppIdle_frameState)/8, BATTLE_PAL_FRAME_WIDTH, BATTLE_PAL_FRAME_HEIGHT, WHITE);
      battleOppIdle_frameState++;
    }
    battleSelfAttack_frameState++;
    if(battleSelfAttack_frameState>=battleSelfAttack_frame){
      battleOppIdle_frameState = 0;
      battleSelfIdle_frameState = 0;
      battleOppAttack_frameState = 0;
      battleSelfAttack_frameState = 0;
      battleOppDamaged_frameState = 0;
      battleSelfDamaged_frameState = 0;
      return true;
    }
  }

  if (battleSelfIdle_frameState >= 9){
    battleSelfIdle_frameState = 0;
  }
  if(battleOppIdle_frameState >= battleOppIdle_frame){
    battleOppIdle_frameState = 0;
  }
  return false;
}

void Animation::clearAllExcpPal(){
  display.fillRect(0, 0, 52, 12, BLACK);
  display.fillRect(75, 25, 52, 12, BLACK);
  display.fillRect(0, 40, SCREEN_WIDTH, SCREEN_HEIGHT - 40, BLACK);
}

void Animation::battleDisplay(String oppName, uint8_t opphp){
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println(oppName);
  display.drawRoundRect(0, 10, 52, 3, 1, WHITE);
  display.drawLine(0 + 1, 10 + 1, 0 + 1 + opphp/2, 10 + 1, WHITE);

  display.setCursor(75, 25);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println(String(pal->get_name().c_str()));
  display.drawRoundRect(75, 35, 52, 3, 1, WHITE);
  display.drawLine(75 + 1, 35 + 1, 75 + 1 + pal->get_hp()/2, 35 + 1, WHITE);
}

void Animation::battleBorderDisplay(){
  display.drawRoundRect(0, 40, 90-10, SCREEN_HEIGHT-41, 3, WHITE);
  display.drawRoundRect(92-10, 40, SCREEN_WIDTH-92+10, SCREEN_HEIGHT-41, 3, WHITE);
  //display.drawRect(0, 16, 64, 24, WHITE);
  //display.drawRect(64, 0, 64, 24, WHITE);
}

void Animation::battleOptionsDisplay(){
  display.setCursor(94, 43);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.println("FIGHT\n");
  display.setCursor(94, 53);
  display.println("RUN");
  if(battleSelectedItem == "fight"){
    display.drawLine(85+1,43,86+1,43,WHITE);
    display.drawLine(85+1,44,87+1,44,WHITE);
    display.drawLine(85+1,45,88+1,45,WHITE);
    display.drawLine(85+1,46,89+1,46,WHITE);
    display.drawLine(85+1,47,89+1,47,WHITE);
    display.drawLine(85+1,48,88+1,48,WHITE);
    display.drawLine(85+1,49,87+1,49,WHITE);
    display.drawLine(85+1,50,86+1,50,WHITE);
  }
  else{
    display.drawLine(85+1,43+10,86+1,43+10,WHITE);
    display.drawLine(85+1,44+10,87+1,44+10,WHITE);
    display.drawLine(85+1,45+10,88+1,45+10,WHITE);
    display.drawLine(85+1,46+10,89+1,46+10,WHITE);
    display.drawLine(85+1,47+10,89+1,47+10,WHITE);
    display.drawLine(85+1,48+10,88+1,48+10,WHITE);
    display.drawLine(85+1,49+10,87+1,49+10,WHITE);
    display.drawLine(85+1,50+10,86+1,50+10,WHITE);
  }
}

void Animation::displayBattleAttack(){
  display.setCursor(6, 45);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println("Wht u do?");
}
void Animation::displayBattleAttackClear(){
  display.setCursor(6, 45);
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.println("Wht u do?");
}
void Animation::displayBattleWaiting(){
  display.setCursor(6, 45);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println("Waiting...");
}
void Animation::displayBattleWaitingClear(){
  display.setCursor(6, 45);
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.println("Waiting...");
}
void Animation::displayBattleSurrendered(){
  display.setCursor(6, 45);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println("Surrendered");
}
void Animation::displayBattleRunning(string oppName){
  display.setCursor(6, 45);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.print(String(oppName.c_str()));
  display.println(" ran...");
}

void Animation::youWin(){
  display.setCursor(20, 20);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("YOU WON!");
}
void Animation::youLose(){
  display.setCursor(20, 20);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("YOU LOST!");
}

void Animation::reflexGame(uint8_t place){
  display.fillRoundRect(0, 40, SCREEN_WIDTH, SCREEN_HEIGHT - 41, 3, BLACK);
  display.drawRoundRect(0, 40, SCREEN_WIDTH, SCREEN_HEIGHT - 41, 2, WHITE);
  display.drawLine(62, 40, 62, 62, WHITE);
  display.drawLine(64, 40, 64, 62, WHITE);
  display.drawLine(place, 40, place, 62, WHITE);
}
void Animation::flashBar(uint8_t place){
  vTaskDelay(pdMS_TO_TICKS(250));
  display.drawLine(place, 40, place, 62, BLACK);
  display.display();
  vTaskDelay(pdMS_TO_TICKS(250));
  display.drawLine(place, 40, place, 62, WHITE);
  display.display();
  vTaskDelay(pdMS_TO_TICKS(250));
  display.drawLine(place, 40, place, 62, BLACK);
  display.display();
  vTaskDelay(pdMS_TO_TICKS(250));
  display.drawLine(place, 40, place, 62, WHITE);
  display.display();
  vTaskDelay(pdMS_TO_TICKS(250));
}

void Animation::displayBattleDamaged(int8_t damage){
  display.fillRoundRect(0, 40, SCREEN_WIDTH, SCREEN_HEIGHT - 41, 3, BLACK);
  display.drawRoundRect(0, 40, SCREEN_WIDTH, SCREEN_HEIGHT - 41, 3, WHITE);
  display.setCursor(6, 45);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.print(String(damage));
  display.println(" hp lost...");
}

void Animation::displayAttackPoint(int8_t attack){
  display.fillRoundRect(0, 40, SCREEN_WIDTH, SCREEN_HEIGHT - 41, 3, BLACK);
  display.drawRoundRect(0, 40, SCREEN_WIDTH, SCREEN_HEIGHT - 41, 3, WHITE);
  display.setCursor(6, 45);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.print(String(attack));
  display.println(" damage");
}

void Animation::loading_dots_black(int16_t x, int16_t y){
  display.setCursor(x, y);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  if(dotcount < 5){
    dotcount++;
    display.println(".");
  }
  else if(dotcount < 10){
    dotcount++;
    display.println("..");
  }
  else if(dotcount < 15){
    dotcount++;
    display.println("...");
  }
  else{
    dotcount = 0;
  }
}

void Animation::loading_dots_white(int16_t x, int16_t y){
  display.setCursor(x, y);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  if(dotcount < 5){
    dotcount++;
    display.println(".");
  }
  else if(dotcount < 10){
    dotcount++;
    display.println("..");
  }
  else if(dotcount < 15){
    dotcount++;
    display.println("...");
  }
  else{
    dotcount = 0;
  }
}

void Animation::menu_detecting_transition(){
  int16_t transition_movement[6] = {-10,-30,-80,-130,-132,-125};

  display.clearDisplay();
  menu_display(transition_movement[0], 0);
  detecting_display(SCREEN_WIDTH+transition_movement[0],0);
  display.display();
  vTaskDelay(pdMS_TO_TICKS(100));

  display.clearDisplay();
  menu_display(transition_movement[1], 0);
  detecting_display(SCREEN_WIDTH+transition_movement[1],0);
  display.display();
  vTaskDelay(pdMS_TO_TICKS(100));

  display.clearDisplay();
  menu_display(transition_movement[2], 0);
  detecting_display(SCREEN_WIDTH+transition_movement[2],0);
  display.display();
  vTaskDelay(pdMS_TO_TICKS(100));

  display.clearDisplay();
  detecting_display(SCREEN_WIDTH+transition_movement[3],0);
  display.display();
  vTaskDelay(pdMS_TO_TICKS(100));

  display.clearDisplay();
  detecting_display(SCREEN_WIDTH+transition_movement[4],0);
  display.display();
  vTaskDelay(pdMS_TO_TICKS(100));

  display.clearDisplay();
  menu_display(transition_movement[5], 0);
  detecting_display(SCREEN_WIDTH+transition_movement[5],0);
  display.display();
  vTaskDelay(pdMS_TO_TICKS(100));
  
}

void Animation::exiting_animation(){
  display.clearDisplay();
  display.setCursor(30,30);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.println("See you soon");
  display.display();
  vTaskDelay(pdMS_TO_TICKS(1000));
  display.clearDisplay();
  display.setCursor(30,30);
  display.println("See you soon.");
  display.display();
  vTaskDelay(pdMS_TO_TICKS(1000));
  display.clearDisplay();
  display.setCursor(30,30);
  display.println("See you soon..");
  display.display();
  vTaskDelay(pdMS_TO_TICKS(1000));
  display.clearDisplay();
  display.setCursor(30,30);
  display.println("See you soon...");
  display.display();
  vTaskDelay(pdMS_TO_TICKS(1000));
  display.clearDisplay();
  display.display();
}

void Animation::menu_items_entered_animation(){
  if (selected_menuitem == "battle"){
    display.fillRoundRect(42,22,42,15,3,BLACK);
    display.display();
    vTaskDelay(pdMS_TO_TICKS(300));
    display.setCursor(42 + 4, 22 + 4);
    display.setTextColor(WHITE);
    display.println("Battle");
    display.display();
  }
  else if (selected_menuitem == "exit"){
    display.fillRoundRect(42,42,42,15,3,BLACK);
    display.display();
    vTaskDelay(pdMS_TO_TICKS(300));
    display.setTextColor(WHITE);
    display.setCursor(42 + 10, 42 + 4);
    display.println("Exit"); 
    display.display();
  }
}

void Animation::menu_emerging_transition(){
  menu_display(0, 63-10);
  vTaskDelay(pdMS_TO_TICKS(100));
  menu_display(0, 63-25);
  vTaskDelay(pdMS_TO_TICKS(100));
  menu_display(0, 63-45);
  vTaskDelay(pdMS_TO_TICKS(100));
  menu_display(0, 63-67);
  vTaskDelay(pdMS_TO_TICKS(100));
  menu_display(0, 63-61);
  vTaskDelay(pdMS_TO_TICKS(100));
  menu_display(0, 63-64);
  vTaskDelay(pdMS_TO_TICKS(100));
  menu_display(0, 63-63);
  vTaskDelay(pdMS_TO_TICKS(100));
}

void Animation::menu_leaving_transition(){
  int16_t x1, y1;
  uint16_t w, h;

  display.getTextBounds(String(pal->get_name().c_str()), 0, 0, &x1, &y1, &w, &h);
  
  display.clearDisplay();
  //name tag
  display.fillRect(0,0,SCREEN_WIDTH-1,15,BLACK);
  display.fillRect(0, 0, 8 + w, 15, WHITE);
  display.drawPixel(0,0,BLACK);
  display.drawPixel(0,14,BLACK);
  display.drawPixel(7+w,0,BLACK);
  display.drawPixel(7+w,14,BLACK);
  display.setCursor(4,4);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.println(String(pal->get_name().c_str()));

  //hp bar
  display.drawLine(75, 1, 125, 1, WHITE);
  display.drawLine(74, 2, 126, 2, WHITE);
  display.drawLine(74, 3, 126, 3, WHITE);
  display.drawLine(74, 4, 126, 4, WHITE);
  display.drawLine(75, 5, 125, 5, WHITE);
  display.fillRect(75 + pal->get_hp()/2, 2, (max_health - pal->get_hp())/2, 3, BLACK);

  //hp icon
  display.drawLine(60+3, 0, 61+3, 0, WHITE);  
  display.drawLine(64+3, 0, 65+3, 0, WHITE);  
  display.drawLine(59+3, 1, 66+3, 1, WHITE);
  display.drawLine(59+3, 2, 66+3, 2, WHITE);
  display.drawLine(59+3, 3, 66+3, 3, WHITE);
  display.drawLine(60+3, 4, 65+3, 4, WHITE);
  display.drawLine(61+3, 5, 64+3, 5, WHITE);
  display.drawLine(62+3, 6, 63+3, 6, WHITE);

  //hunger bar
  display.drawLine(75, 1+9, 125, 1+9, WHITE);
  display.drawLine(74, 2+9, 126, 2+9, WHITE);
  display.drawLine(74, 3+9, 126, 3+9, WHITE);
  display.drawLine(74, 4+9, 126, 4+9, WHITE);
  display.drawLine(75, 5+9, 125, 5+9, WHITE);
  display.fillRect(75 + pal->get_hunger()/2, 2+9, (max_hunger - pal->get_hunger())/2, 3, BLACK);

  //hunger icon
  display.drawLine(65+3, 0+10, 65+3, 4+10, WHITE);  
  display.drawLine(64+3, 1+10, 64+3, 2+10, WHITE);  
  display.drawLine(60+3, 0+10, 60+3, 2+10, WHITE);  
  display.drawLine(62+3, 0+10, 62+3, 2+10, WHITE);  
  display.drawLine(61+3, 2+10, 61+3, 4+10, WHITE);
  display.drawBitmap((SCREEN_WIDTH-PAL_FRAME_WIDTH)/2 - 1, 15, pal_default_still, 24, 48, WHITE);
  print_menuscroll_toolbox(0, 0);
  print_menuscroll_setting(0, 0);
  display.drawBitmap(menuscroll_x, menuscroll_y, menuscroll_on_still, MENUSCROLL_FRAME_WIDTH, MENUSCROLL_FRAME_HEIGHT, WHITE);
  menu_display(0, -4);
  vTaskDelay(pdMS_TO_TICKS(100));

  display.clearDisplay();
  //name tag
  display.fillRect(0,0,SCREEN_WIDTH-1,15,BLACK);
  display.fillRect(0, 0, 8 + w, 15, WHITE);
  display.drawPixel(0,0,BLACK);
  display.drawPixel(0,14,BLACK);
  display.drawPixel(7+w,0,BLACK);
  display.drawPixel(7+w,14,BLACK);
  display.setCursor(4,4);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.println(String(pal->get_name().c_str()));

  //hp bar
  display.drawLine(75, 1, 125, 1, WHITE);
  display.drawLine(74, 2, 126, 2, WHITE);
  display.drawLine(74, 3, 126, 3, WHITE);
  display.drawLine(74, 4, 126, 4, WHITE);
  display.drawLine(75, 5, 125, 5, WHITE);
  display.fillRect(75 + pal->get_hp()/2, 2, (max_health - pal->get_hp())/2, 3, BLACK);

  //hp icon
  display.drawLine(60+3, 0, 61+3, 0, WHITE);  
  display.drawLine(64+3, 0, 65+3, 0, WHITE);  
  display.drawLine(59+3, 1, 66+3, 1, WHITE);
  display.drawLine(59+3, 2, 66+3, 2, WHITE);
  display.drawLine(59+3, 3, 66+3, 3, WHITE);
  display.drawLine(60+3, 4, 65+3, 4, WHITE);
  display.drawLine(61+3, 5, 64+3, 5, WHITE);
  display.drawLine(62+3, 6, 63+3, 6, WHITE);

  //hunger bar
  display.drawLine(75, 1+9, 125, 1+9, WHITE);
  display.drawLine(74, 2+9, 126, 2+9, WHITE);
  display.drawLine(74, 3+9, 126, 3+9, WHITE);
  display.drawLine(74, 4+9, 126, 4+9, WHITE);
  display.drawLine(75, 5+9, 125, 5+9, WHITE);
  display.fillRect(75 + pal->get_hunger()/2, 2+9, (max_hunger - pal->get_hunger())/2, 3, BLACK);

  //hunger icon
  display.drawLine(65+3, 0+10, 65+3, 4+10, WHITE);  
  display.drawLine(64+3, 1+10, 64+3, 2+10, WHITE);  
  display.drawLine(60+3, 0+10, 60+3, 2+10, WHITE);  
  display.drawLine(62+3, 0+10, 62+3, 2+10, WHITE);  
  display.drawLine(61+3, 2+10, 61+3, 4+10, WHITE);
  display.drawBitmap((SCREEN_WIDTH-PAL_FRAME_WIDTH)/2 - 1, 15, pal_default_still, 24, 48, WHITE);
  print_menuscroll_toolbox(0, 0);
  print_menuscroll_setting(0, 0);
  display.drawBitmap(menuscroll_x, menuscroll_y, menuscroll_on_still, MENUSCROLL_FRAME_WIDTH, MENUSCROLL_FRAME_HEIGHT, WHITE);
  menu_display(0, -6);
  vTaskDelay(pdMS_TO_TICKS(100));

  display.clearDisplay();
  //name tag
  display.fillRect(0,0,SCREEN_WIDTH-1,15,BLACK);
  display.fillRect(0, 0, 8 + w, 15, WHITE);
  display.drawPixel(0,0,BLACK);
  display.drawPixel(0,14,BLACK);
  display.drawPixel(7+w,0,BLACK);
  display.drawPixel(7+w,14,BLACK);
  display.setCursor(4,4);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.println(String(pal->get_name().c_str()));

  //hp bar
  display.drawLine(75, 1, 125, 1, WHITE);
  display.drawLine(74, 2, 126, 2, WHITE);
  display.drawLine(74, 3, 126, 3, WHITE);
  display.drawLine(74, 4, 126, 4, WHITE);
  display.drawLine(75, 5, 125, 5, WHITE);
  display.fillRect(75 + pal->get_hp()/2, 2, (max_health - pal->get_hp())/2, 3, BLACK);

  //hp icon
  display.drawLine(60+3, 0, 61+3, 0, WHITE);  
  display.drawLine(64+3, 0, 65+3, 0, WHITE);  
  display.drawLine(59+3, 1, 66+3, 1, WHITE);
  display.drawLine(59+3, 2, 66+3, 2, WHITE);
  display.drawLine(59+3, 3, 66+3, 3, WHITE);
  display.drawLine(60+3, 4, 65+3, 4, WHITE);
  display.drawLine(61+3, 5, 64+3, 5, WHITE);
  display.drawLine(62+3, 6, 63+3, 6, WHITE);

  //hunger bar
  display.drawLine(75, 1+9, 125, 1+9, WHITE);
  display.drawLine(74, 2+9, 126, 2+9, WHITE);
  display.drawLine(74, 3+9, 126, 3+9, WHITE);
  display.drawLine(74, 4+9, 126, 4+9, WHITE);
  display.drawLine(75, 5+9, 125, 5+9, WHITE);
  display.fillRect(75 + pal->get_hunger()/2, 2+9, (max_hunger - pal->get_hunger())/2, 3, BLACK);

  //hunger icon
  display.drawLine(65+3, 0+10, 65+3, 4+10, WHITE);  
  display.drawLine(64+3, 1+10, 64+3, 2+10, WHITE);  
  display.drawLine(60+3, 0+10, 60+3, 2+10, WHITE);  
  display.drawLine(62+3, 0+10, 62+3, 2+10, WHITE);  
  display.drawLine(61+3, 2+10, 61+3, 4+10, WHITE);
  display.drawBitmap((SCREEN_WIDTH-PAL_FRAME_WIDTH)/2 - 1, 15, pal_default_still, 24, 48, WHITE);
  print_menuscroll_toolbox(0, 0);
  print_menuscroll_setting(0, 0);
  display.drawBitmap(menuscroll_x, menuscroll_y, menuscroll_on_still, MENUSCROLL_FRAME_WIDTH, MENUSCROLL_FRAME_HEIGHT, WHITE);
  menu_display(0, 10);
  vTaskDelay(pdMS_TO_TICKS(100));
  
  display.clearDisplay();
  //name tag
  display.fillRect(0,0,SCREEN_WIDTH-1,15,BLACK);
  display.fillRect(0, 0, 8 + w, 15, WHITE);
  display.drawPixel(0,0,BLACK);
  display.drawPixel(0,14,BLACK);
  display.drawPixel(7+w,0,BLACK);
  display.drawPixel(7+w,14,BLACK);
  display.setCursor(4,4);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.println(String(pal->get_name().c_str()));

  //hp bar
  display.drawLine(75, 1, 125, 1, WHITE);
  display.drawLine(74, 2, 126, 2, WHITE);
  display.drawLine(74, 3, 126, 3, WHITE);
  display.drawLine(74, 4, 126, 4, WHITE);
  display.drawLine(75, 5, 125, 5, WHITE);
  display.fillRect(75 + pal->get_hp()/2, 2, (max_health - pal->get_hp())/2, 3, BLACK);

  //hp icon
  display.drawLine(60+3, 0, 61+3, 0, WHITE);  
  display.drawLine(64+3, 0, 65+3, 0, WHITE);  
  display.drawLine(59+3, 1, 66+3, 1, WHITE);
  display.drawLine(59+3, 2, 66+3, 2, WHITE);
  display.drawLine(59+3, 3, 66+3, 3, WHITE);
  display.drawLine(60+3, 4, 65+3, 4, WHITE);
  display.drawLine(61+3, 5, 64+3, 5, WHITE);
  display.drawLine(62+3, 6, 63+3, 6, WHITE);

  //hunger bar
  display.drawLine(75, 1+9, 125, 1+9, WHITE);
  display.drawLine(74, 2+9, 126, 2+9, WHITE);
  display.drawLine(74, 3+9, 126, 3+9, WHITE);
  display.drawLine(74, 4+9, 126, 4+9, WHITE);
  display.drawLine(75, 5+9, 125, 5+9, WHITE);
  display.fillRect(75 + pal->get_hunger()/2, 2+9, (max_hunger - pal->get_hunger())/2, 3, BLACK);

  //hunger icon
  display.drawLine(65+3, 0+10, 65+3, 4+10, WHITE);  
  display.drawLine(64+3, 1+10, 64+3, 2+10, WHITE);  
  display.drawLine(60+3, 0+10, 60+3, 2+10, WHITE);  
  display.drawLine(62+3, 0+10, 62+3, 2+10, WHITE);  
  display.drawLine(61+3, 2+10, 61+3, 4+10, WHITE);
  display.drawBitmap((SCREEN_WIDTH-PAL_FRAME_WIDTH)/2 - 1, 15, pal_default_still, 24, 48, WHITE);
  print_menuscroll_toolbox(0, 0);
  print_menuscroll_setting(0, 0);
  display.drawBitmap(menuscroll_x, menuscroll_y, menuscroll_on_still, MENUSCROLL_FRAME_WIDTH, MENUSCROLL_FRAME_HEIGHT, WHITE);
  menu_display(0, 25);
  vTaskDelay(pdMS_TO_TICKS(100));

  display.clearDisplay();
  //name tag
  display.fillRect(0,0,SCREEN_WIDTH-1,15,BLACK);
  display.fillRect(0, 0, 8 + w, 15, WHITE);
  display.drawPixel(0,0,BLACK);
  display.drawPixel(0,14,BLACK);
  display.drawPixel(7+w,0,BLACK);
  display.drawPixel(7+w,14,BLACK);
  display.setCursor(4,4);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.println(String(pal->get_name().c_str()));

  //hp bar
  display.drawLine(75, 1, 125, 1, WHITE);
  display.drawLine(74, 2, 126, 2, WHITE);
  display.drawLine(74, 3, 126, 3, WHITE);
  display.drawLine(74, 4, 126, 4, WHITE);
  display.drawLine(75, 5, 125, 5, WHITE);
  display.fillRect(75 + pal->get_hp()/2, 2, (max_health - pal->get_hp())/2, 3, BLACK);

  //hp icon
  display.drawLine(60+3, 0, 61+3, 0, WHITE);  
  display.drawLine(64+3, 0, 65+3, 0, WHITE);  
  display.drawLine(59+3, 1, 66+3, 1, WHITE);
  display.drawLine(59+3, 2, 66+3, 2, WHITE);
  display.drawLine(59+3, 3, 66+3, 3, WHITE);
  display.drawLine(60+3, 4, 65+3, 4, WHITE);
  display.drawLine(61+3, 5, 64+3, 5, WHITE);
  display.drawLine(62+3, 6, 63+3, 6, WHITE);

  //hunger bar
  display.drawLine(75, 1+9, 125, 1+9, WHITE);
  display.drawLine(74, 2+9, 126, 2+9, WHITE);
  display.drawLine(74, 3+9, 126, 3+9, WHITE);
  display.drawLine(74, 4+9, 126, 4+9, WHITE);
  display.drawLine(75, 5+9, 125, 5+9, WHITE);
  display.fillRect(75 + pal->get_hunger()/2, 2+9, (max_hunger - pal->get_hunger())/2, 3, BLACK);

  //hunger icon
  display.drawLine(65+3, 0+10, 65+3, 4+10, WHITE);  
  display.drawLine(64+3, 1+10, 64+3, 2+10, WHITE);  
  display.drawLine(60+3, 0+10, 60+3, 2+10, WHITE);  
  display.drawLine(62+3, 0+10, 62+3, 2+10, WHITE);  
  display.drawLine(61+3, 2+10, 61+3, 4+10, WHITE);
  display.drawBitmap((SCREEN_WIDTH-PAL_FRAME_WIDTH)/2 - 1, 15, pal_default_still, 24, 48, WHITE);
  print_menuscroll_toolbox(0, 0);
  print_menuscroll_setting(0, 0);
  display.drawBitmap(menuscroll_x, menuscroll_y, menuscroll_on_still, MENUSCROLL_FRAME_WIDTH, MENUSCROLL_FRAME_HEIGHT, WHITE);
  menu_display(0, 45);
  vTaskDelay(pdMS_TO_TICKS(100));

  display.clearDisplay();
  //name tag
  display.fillRect(0,0,SCREEN_WIDTH-1,15,BLACK);
  display.fillRect(0, 0, 8 + w, 15, WHITE);
  display.drawPixel(0,0,BLACK);
  display.drawPixel(0,14,BLACK);
  display.drawPixel(7+w,0,BLACK);
  display.drawPixel(7+w,14,BLACK);
  display.setCursor(4,4);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.println(String(pal->get_name().c_str()));

  //hp bar
  display.drawLine(75, 1, 125, 1, WHITE);
  display.drawLine(74, 2, 126, 2, WHITE);
  display.drawLine(74, 3, 126, 3, WHITE);
  display.drawLine(74, 4, 126, 4, WHITE);
  display.drawLine(75, 5, 125, 5, WHITE);
  display.fillRect(75 + pal->get_hp()/2, 2, (max_health - pal->get_hp())/2, 3, BLACK);

  //hp icon
  display.drawLine(60+3, 0, 61+3, 0, WHITE);  
  display.drawLine(64+3, 0, 65+3, 0, WHITE);  
  display.drawLine(59+3, 1, 66+3, 1, WHITE);
  display.drawLine(59+3, 2, 66+3, 2, WHITE);
  display.drawLine(59+3, 3, 66+3, 3, WHITE);
  display.drawLine(60+3, 4, 65+3, 4, WHITE);
  display.drawLine(61+3, 5, 64+3, 5, WHITE);
  display.drawLine(62+3, 6, 63+3, 6, WHITE);

  //hunger bar
  display.drawLine(75, 1+9, 125, 1+9, WHITE);
  display.drawLine(74, 2+9, 126, 2+9, WHITE);
  display.drawLine(74, 3+9, 126, 3+9, WHITE);
  display.drawLine(74, 4+9, 126, 4+9, WHITE);
  display.drawLine(75, 5+9, 125, 5+9, WHITE);
  display.fillRect(75 + pal->get_hunger()/2, 2+9, (max_hunger - pal->get_hunger())/2, 3, BLACK);

  //hunger icon
  display.drawLine(65+3, 0+10, 65+3, 4+10, WHITE);  
  display.drawLine(64+3, 1+10, 64+3, 2+10, WHITE);  
  display.drawLine(60+3, 0+10, 60+3, 2+10, WHITE);  
  display.drawLine(62+3, 0+10, 62+3, 2+10, WHITE);  
  display.drawLine(61+3, 2+10, 61+3, 4+10, WHITE);
  display.drawBitmap((SCREEN_WIDTH-PAL_FRAME_WIDTH)/2 - 1, 15, pal_default_still, 24, 48, WHITE);
  print_menuscroll_toolbox(0, 0);
  print_menuscroll_setting(0, 0);
  display.drawBitmap(menuscroll_x, menuscroll_y, menuscroll_on_still, MENUSCROLL_FRAME_WIDTH, MENUSCROLL_FRAME_HEIGHT, WHITE);
  display.display();
  vTaskDelay(pdMS_TO_TICKS(10));
}