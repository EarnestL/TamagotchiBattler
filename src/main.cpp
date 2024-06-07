#include <Arduino.h>
#include <string>

#include <WiFi.h>
#include <esp_now.h>

#include "esp_sleep.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/rtc_io.h"

#include "driver/gpio.h"
#include "freertos/queue.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_system.h"

#include "nvs_flash.h"
#include "Pal.h"
#include "Animation.h"

#include <vector>
#include <algorithm>
using namespace std;
//uxTaskGetStackHighWaterMark(NULL)

#define enter_b 34
#define scroll_b 35
#define menu_b 32
#define sleep_b 26
#define max_health 100
#define max_hunger 100
#define sec_per_hunger 1800
#define sec_per_hp 3600
#define hunger_threshold 80

#define uS_TO_S_FACTOR 1000000  // Conversion factor for micro seconds to seconds


RTC_DATA_ATTR uint64_t sleep_start_time;
Animation animation;

TaskHandle_t stats_display_handle;
TaskHandle_t pal_display_handle;
TaskHandle_t menuscroll_display_handle;
TaskHandle_t toolbox_display_handle;
TaskHandle_t menu_display_handle;
TaskHandle_t broadcastingAddressTask_handle;
TaskHandle_t receivingAddressTask_handle;
TaskHandle_t battleAttemptTask_handle;
TaskHandle_t peerAuthTask_handle;
TaskHandle_t battleTask_handle;
TaskHandle_t palBattleTask_handle;

bool pal_init(Pal*);
void pal_new(Pal *);
void input_task(void*);
void display_task(void*);
void time_update_task(void*);
String current_time(uint16_t*, uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint8_t*);
void home_display(Pal*);
void menu_display(Pal*);
void stats_display_task(void *);
void pal_display_task(void *);
void toolbox_display_task(void *);
void menuscroll_display_task(void *);
void menu_display_task(void *);
void deepSleepTask(void *);
void broadcastingAddressTask(void *);
void receivingAddressTask(void *);
void battleAttemptTask(void *);
void peerAuthTask(void *);
void battleTask(void *);
int8_t battleAttackGame();
void palBattleTask(void *);
bool battleGameInProgress = false;

esp_now_peer_info_t peerInfo;
typedef struct stats{
  char name[20];
  uint8_t hp;
} stats;

typedef struct broadcastData{
  char name[20];
  uint8_t address[6];
  bool battle_attempt;
  bool notFree;
  bool feedback;
  uint8_t ignoreIfAddress[6];
} broadcastData;

typedef struct localData{
  broadcastData data;
  uint8_t remove_countdown;
  bool selected;
  bool challenging;
} localData;

typedef struct battleData{
  char name[20];
  uint8_t address[6];
  int8_t hp;
  int8_t attackDamage;
} battleData;

typedef struct validation{
  uint8_t address[6];
  bool valid;
} validation;

bool oppAttack = false;
bool selfAttack = false;
//problem- after exiting with battle task, not broadcasting to all addresses
uint8_t battleInitializationSend = 0;
bool challenger = false;
uint8_t validatedCount = 0;
bool battleValidationFeedback = false;
bool broadcastUpdate = false;
bool peerValid = true;
bool surrender = false;
uint8_t selfMACAddress[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
//uint8_t selfMACAddress[6] = {0xE4, 0x65, 0xB8, 0x79, 0x0E, 0xCC};//esp1
//uint8_t selfMACAddress[6] = {0xFC, 0xB4, 0x67, 0x74, 0x64, 0x28};//esp2
//uint8_t selfMACAddress[6] = {0xA0, 0xA3, 0xB3, 0xAB, 0x2F, 0x2C};//esp3
uint8_t broadcastAddress[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t allAddress[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t defaultIgnoreAddress[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
vector<broadcastData*> AddressesInRangeQueue;
vector<localData*> AddressesInRange;

battleData peerBattleData;
localData* peerData;
broadcastData allBroadcastData;

void debugBlink(){
  digitalWrite(2, HIGH);
  vTaskDelay(pdMS_TO_TICKS(100));
  digitalWrite(2, LOW);
  vTaskDelay(pdMS_TO_TICKS(100));
}

void OnSent(const uint8_t *mac_addr, esp_now_send_status_t status){
  //Serial.print("\r\nLast Packet Send Status:\t");
  //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
 
void onReceiveBattle(const uint8_t *macAddr, const uint8_t *data, int len){

  if (len == 6){
    //check address
    bool match = true;
    for (int i = 0; i < 6; i++){
      if(peerData->data.address[i] != data[i]){
        match = false;
        break;
      }
    }
    if(match){
      //send feedback data
      debugBlink();
      validation feedbackData;
      memcpy(feedbackData.address, selfMACAddress, 6);
      feedbackData.valid = true;
      esp_now_send(peerData->data.address, (uint8_t *)&feedbackData, sizeof(validation)); //give feedback
    }
  }
  else if(len == 7){
    validation recvData;
    memcpy(&recvData, data, 7);
    //check address
    bool match = true;
    for (int i = 0; i < 6; i++){
      if(peerData->data.address[i] != recvData.address[i]){
        match = false;
        break;
      }
    }
    /*if (match){
      debugBlink();
    }*/
    //check feedback
    if (match && recvData.valid && validatedCount<3){
      validatedCount++;
      battleValidationFeedback = true;
      animation.clearDisplay();
      animation.print_dotcount(validatedCount);
      animation.display_all();
    }
    if(match && recvData.valid){
      battleValidationFeedback = true;
    }
  }
  else{
    memcpy(&peerBattleData, data, sizeof(battleData));
    if (battleInitializationSend>=2){
      challenger = true;
    }
    else{
      battleInitializationSend++;
    }
  }
}


void OnReceive(const uint8_t *macAddr, const uint8_t *data, int len){
  broadcastData* peer = new broadcastData;
  memcpy(peer, data, sizeof(broadcastData));
  AddressesInRangeQueue.push_back(peer);
}


void setup() {

  Serial.begin(921600);

    WiFi.mode(WIFI_STA);
    // Initialize ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    // Read the MAC address from the hardware registers
    esp_read_mac(selfMACAddress, ESP_MAC_WIFI_STA);
    WiFi.mode(WIFI_OFF);

    pinMode(enter_b, INPUT);
    pinMode(scroll_b, INPUT);
    pinMode(menu_b, INPUT);
    pinMode(sleep_b, INPUT);
    pinMode(2, OUTPUT);
    digitalWrite(2, LOW);

    Pal *pal = new Pal;
    nvs_flash_init();
    bool old_exist = pal_init(pal);
    animation.display_init(pal);
    if (!old_exist)
    {
      pal_new(pal);
  }
  pal->set_hp(100);
  animation.set_curr_display("home");
  animation.start_transition_animation();
  xTaskCreate(&input_task, "input_task", 4096, (void*) pal, 1, NULL);
  xTaskCreate(&time_update_task, "time_update_task", 2048, (void *)pal, 2, NULL);
  xTaskCreate(&stats_display_task, "stats_display_task", 2048, (void *)pal, 1, &stats_display_handle);
  xTaskCreate(&pal_display_task, "pal_display_task", 4096, (void *)pal, 1, &pal_display_handle);
  xTaskCreate(&toolbox_display_task, "toolbox_display_task", 4096, (void *)pal, 1, &toolbox_display_handle);
  xTaskCreate(&menuscroll_display_task, "menuscroll_display_task", 4096, NULL, 1, &menuscroll_display_handle);
  //xTaskCreate(&menuscroll_display_task, "menuscroll_display_task", 4096, NULL, 1, &menuscroll_display_handle);
  xTaskCreate(&deepSleepTask, "deepSleepTask", 2048, NULL, 2, NULL);
  //xTaskCreate(&battleTask, "BattleTask", 4096, (void *)pal, 1, &battleTask_handle);
}

void loop() {
}


void deepSleepTask(void *pvParameters) {
  for (;;){
    if(digitalRead(sleep_b)){
      while(digitalRead(sleep_b)){};
      //if(animation.get_curr_display()=="home" ||animation.get_curr_display()=="menuscroll"||animation.get_curr_display()=="toolbox"){
        //vTaskDelete(stats_display_handle);
        //vTaskDelete(pal_display_handle);
        //vTaskDelete(menuscroll_display_handle);
        //vTaskDelete(toolbox_display_handle);
      //}
      //else if(animation.get_curr_display()=="menu"){
      //  vTaskDelete(menu_display_handle);
      //}
      animation.set_curr_display("deepsleep");
      while(menu_display_handle != NULL || stats_display_handle != NULL || pal_display_handle != NULL || menuscroll_display_handle != NULL || toolbox_display_handle != NULL || peerAuthTask_handle != NULL){
        vTaskDelay(pdMS_TO_TICKS(50));
      };
      esp_now_deinit();
      animation.exiting_animation();
      esp_sleep_enable_ext0_wakeup((gpio_num_t)sleep_b, HIGH);
      sleep_start_time = esp_timer_get_time();
      vTaskDelay(pdMS_TO_TICKS(250));
      esp_deep_sleep_start();
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

bool pal_init(Pal* pal){
  if (pal->nvs_retrieve()){
    Serial.println("Existing Pal!");
    Serial.println("Hunger was " + String(pal->get_hunger()));
    Serial.println("HP was " + String(pal->get_hp()));
    Serial.println("hungrier until was " + String(pal->get_sec_until_hungrier()));
    Serial.println("weaker until was " + String(pal->get_sec_until_weaker()));
    Serial.println("\n");
    int8_t prev_hunger = pal->get_hunger();
    uint64_t wake_time = esp_timer_get_time();
    uint64_t sleep_duration = (wake_time - sleep_start_time)/uS_TO_S_FACTOR;

    //hunger update
    /*int hunger_increase = 2 * ((sleep_duration + pal->get_sec_until_hungrier()) / sec_per_hunger);
    if (prev_hunger == 100){
      Serial.println("Do not forget to feed your pet!");
    }
    else if ((pal->get_hunger() + hunger_increase) >= 100){
      Serial.println("Your pal became hangry while you were away. :(");
      pal->set_sec_until_hungrier(0);
      pal->set_hunger(100);
    }
    else{
      pal->set_sec_until_hungrier((sleep_duration + pal->get_sec_until_hungrier()) % sec_per_hunger);
      pal->set_hunger(pal->get_hunger() + hunger_increase);
      Serial.println(String(hunger_increase) + " hunger points have increased");
      Serial.println("Current hunger is " + String(pal->get_hunger()));
      Serial.println("Hunger updated");  
    }

    Serial.println("Health was " + String(pal->get_hp()));
    //potential health lost
    int time_accum_hp_lost;
    if (prev_hunger >= hunger_threshold){
      time_accum_hp_lost = sleep_duration + pal->get_sec_until_weaker();
    }
    else if (pal->get_hunger()>= hunger_threshold){
      int sec_before_hp_decrease = (prev_hunger - hunger_threshold) * (sec_per_hunger/2);
      time_accum_hp_lost = sleep_duration - sec_before_hp_decrease;
    }
    else{
      time_accum_hp_lost = 0;
    }

    int hp_lost = 2 * (time_accum_hp_lost / sec_per_hp);
    //health update
    if ((pal->get_hp() - hp_lost) <= 0){
      Serial.println("Your pal died while you were away. :(");
      pal->set_sec_until_weaker(0);
      pal->set_hp(0);
    }
    else{
      pal->set_sec_until_weaker(time_accum_hp_lost%sec_per_hp);
      pal->set_hp(pal->get_hp()-hp_lost);
      Serial.println(String(hp_lost) + " hp has decreased");
      Serial.println("Current health is " + String(pal->get_hp()));
      Serial.println("Health updated");  
    }*/
    return 1;
  }
  else{
    return 0;
  }
}

void pal_new(Pal* pal){
  int option = 0;
  for (;;){
    String names[4] = {"Carl", "Jake", "Jameson", "Yokota"};
    animation.display_newgame(option);
    if (digitalRead(scroll_b)){
      while(digitalRead(scroll_b)){};
      if (option == 3){
        option = 0;
      }
      else{
        option++;
      }
    }

    if (digitalRead(enter_b)){
      while(digitalRead(enter_b)){};
      animation.clearDisplay();
      pal->set_name(names[option].c_str());
      pal->set_hp(100);
      pal->set_hunger(60);
      pal->set_sec_until_hungrier(0);
      pal->set_sec_until_weaker(0);
      pal->set_alive_nvs(1);
      return;
    }
  }
}

void stats_display_task(void *pvParameter){
  Pal* pal = (Pal*)pvParameter;
  for (;;){
    if (animation.get_curr_display() == "home" || animation.get_curr_display() == "menuscroll"  || animation.get_curr_display() == "toolbox" ){
      animation.display_stats();
    }
    if(animation.get_curr_display() == "menu" || animation.get_curr_display() == "deepsleep"){
      stats_display_handle = NULL;
      vTaskDelete(NULL);
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }

}
void pal_display_task(void *pvParameter){
  Pal* pal = (Pal*)pvParameter;
  for (;;){
    if (animation.get_curr_display() == "home" || animation.get_curr_display() == "menuscroll"  || animation.get_curr_display() == "toolbox" ){
      animation.pal_display();
    }
    if(animation.get_curr_display() == "menu" || animation.get_curr_display() == "deepsleep"){
      pal_display_handle = NULL;
      vTaskDelete(NULL);
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
void toolbox_display_task(void *pvParameter){
  Pal* pal = (Pal*)pvParameter;
  string prev_display = "menuscroll";
  for (;;){
    if (animation.get_curr_display() == "toolbox" || animation.get_curr_display() == "menuscroll"){
      if(prev_display != animation.get_curr_display()){
        if (animation.get_curr_display() == "toolbox" && prev_display == "menuscroll"){
         animation.toolbox_emerging_animation();
        }
        else if (animation.get_curr_display() == "menuscroll" && prev_display == "toolbox"){
          animation.toolbox_leaving_animation();
        }
        prev_display = animation.get_curr_display();
      }

      if (animation.get_curr_display() == "toolbox"){
        animation.toolbox_display();
        }
      else{
        animation.clear_toolbox();
      }
    }
    if(animation.get_curr_display() == "menu" || animation.get_curr_display() == "deepsleep"){
      toolbox_display_handle = NULL;
      vTaskDelete(NULL);
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }

}
void menuscroll_display_task(void* pvParameter){
  string prev_display = animation.get_curr_display();
  for (;;){
    if (animation.get_curr_display() == "home"  || animation.get_curr_display() == "menuscroll"  || animation.get_curr_display() == "toolbox" ){
      if(prev_display != animation.get_curr_display()){
        if (animation.get_curr_display() == "menuscroll" && prev_display == "home"){
          animation.menuscroll_opening_animation();
        }
        else if (animation.get_curr_display() == "home" && prev_display == "menuscroll"){
          animation.menuscroll_closing_animation();
        }
        prev_display = animation.get_curr_display();
      }

      if(animation.get_curr_display() == "home"){
        animation.menuscroll_off_display();
      }
      else if (animation.get_curr_display() == "menuscroll" || animation.get_curr_display() == "toolbox"){
        animation.menuscroll_on_display();
      }
    }
    if(animation.get_curr_display() == "menu" || animation.get_curr_display() == "deepsleep"){
      animation.menuscroll_items_entered_animation();
      menuscroll_display_handle = NULL;
      vTaskDelete(NULL);
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
  
}
void input_task(void* pvParameter){
  Pal* pal = (Pal*)pvParameter;
  string prev_display = animation.get_curr_display();
  string curr_display = animation.get_curr_display();
  for (;;){
    curr_display = animation.get_curr_display();
    if (curr_display == "home"){
      if (digitalRead(menu_b)){
        while (digitalRead(menu_b)){};
        Serial.println("MENU pressed");
        animation.set_curr_display("menuscroll");
      }
    }
    else if (curr_display == "menuscroll"){

      if (digitalRead(enter_b)){
        while(digitalRead(enter_b)){};
        string curr_selected_tool = animation.get_selected_menuscrollitem();
        if (curr_selected_tool == "toolbox"){
          animation.enterexecute();
        }
        else if (curr_selected_tool == "menu"){
          animation.set_curr_display("menu");
          while(pal_display_handle != NULL || stats_display_handle != NULL || menuscroll_display_handle != NULL || toolbox_display_handle != NULL ){
            //digitalWrite(2, !digitalRead(2));
            vTaskDelay(pdMS_TO_TICKS(50));
          };
          animation.menu_emerging_transition();
          xTaskCreate(&menu_display_task, "menu_display_task", 2048, (void *)pal, 1, &menu_display_handle);
        }
      }
      else if (digitalRead(menu_b)){
        while(digitalRead(menu_b)){};
        animation.set_curr_display("home");
      }
      else if (digitalRead(scroll_b)){
        while(digitalRead(scroll_b)){};
        animation.scrollexecute();
      }
    }
    else if(curr_display == "toolbox"){
      if (digitalRead(scroll_b)){
        while(digitalRead(scroll_b)){};
        animation.scrollexecute();

      }
      else if (digitalRead(enter_b)){
        while(digitalRead(enter_b)){};
        animation.enterexecute();
      }
      else if (digitalRead(menu_b)){
        while(digitalRead(menu_b)){};
        animation.set_curr_display("menuscroll");
      }
    }
    else if(curr_display == "menu"){
      if (digitalRead(menu_b)){
        while(digitalRead(menu_b)){};
        animation.set_curr_display("menuscroll");
        while(menu_display_handle != NULL){            
          //digitalWrite(2, !digitalRead(2));
          vTaskDelay(pdMS_TO_TICKS(1000));
        };
        animation.menu_leaving_transition();
        xTaskCreate(&stats_display_task, "stats_display_task", 2048, (void *)pal, 1, &stats_display_handle);
        xTaskCreate(&pal_display_task, "pal_display_task", 4096, (void *)pal, 1, &pal_display_handle);
        xTaskCreate(&toolbox_display_task, "toolbox_display_task", 4096, (void *)pal, 1, &toolbox_display_handle);
        xTaskCreate(&menuscroll_display_task, "menuscroll_display_task", 4096, NULL, 1, &menuscroll_display_handle);
      }
      else if(digitalRead(scroll_b)){
        while(digitalRead(scroll_b)){};
        animation.scrollexecute();
      }
      else if(digitalRead(enter_b)){
        while(digitalRead(enter_b)){};
        if(animation.get_selected_menuitem()=="battle"){
          animation.set_curr_display("detecting");
          //transmission of address begins
          xTaskCreate(&broadcastingAddressTask, "broadcastingAddressTask", 2048, (void *)pal, 1, &broadcastingAddressTask_handle);
          xTaskCreate(&receivingAddressTask, "receivingAddressTask", 2048, NULL, 1, &receivingAddressTask_handle);
        }
        else if(animation.get_selected_menuitem()=="exit"){
          //enter deepsleep
        }
      }
    }
    else if(curr_display == "detecting" || curr_display == "battle_attempt"){
      if (digitalRead(menu_b)){
        while(digitalRead(menu_b)){};
        animation.set_curr_display("menu");
        while(broadcastingAddressTask_handle != NULL || receivingAddressTask_handle != NULL || battleAttemptTask_handle != NULL){vTaskDelay(pdMS_TO_TICKS(50));}
        esp_now_deinit();
      }
      else if(digitalRead(scroll_b)){
        while(digitalRead(scroll_b)){};
        if (AddressesInRange.size() > 1){
          uint8_t index = 99;
          for (int i = 0; i < AddressesInRange.size(); i++){
            if (AddressesInRange[i]->selected){
              index = i;
              break;
            }
          }
          if (index != 99){
            AddressesInRange[index]->selected = false;
            if(index == AddressesInRange.size()-1){
              AddressesInRange[0]->selected = true;
            }
            else{
              AddressesInRange[index + 1]->selected = true;
            }
          }
          broadcastUpdate = true;
        }
      }
      else if(digitalRead(enter_b)){
        while(digitalRead(enter_b)){};
        if(AddressesInRange.size()>=1){
          for (int i = 0; i < AddressesInRange.size(); i++){
            if (AddressesInRange[i]->selected){
              if(AddressesInRange[i]->data.battle_attempt){
                if (peerData!=nullptr){
                  esp_now_del_peer(peerData->data.address);
                }
                peerData = AddressesInRange[i];
                //add peer/////////////////////////////////////////////////////////////////////////////////////////////////
                memset(&peerInfo, 0, sizeof(peerInfo));  // Clear the peerInfo structure
                memcpy(peerInfo.peer_addr, peerData->data.address, 6);
                peerInfo.channel = 0;
                peerInfo.encrypt = false;

                if (esp_now_add_peer(&peerInfo) != ESP_OK){
                  Serial.println("Failed to add peer");
                  return;
                }

                //notify other of battle acceptance
                broadcastData targetData;
                memcpy(targetData.address, selfMACAddress, 6);
                targetData.battle_attempt = true;
                memcpy(allBroadcastData.ignoreIfAddress, peerData->data.address, 6 * sizeof(uint8_t));
                esp_now_send(peerData->data.address, (uint8_t *)&targetData, sizeof(targetData));
                vTaskDelay(pdMS_TO_TICKS(100));
                esp_now_send(peerData->data.address, (uint8_t *)&targetData, sizeof(targetData));
                vTaskDelay(pdMS_TO_TICKS(100));
                esp_now_send(peerData->data.address, (uint8_t *)&targetData, sizeof(targetData));

                //try to begin battle
                animation.set_curr_display("peerAuth");
                while(broadcastingAddressTask_handle != NULL || menu_display_handle != NULL|| receivingAddressTask_handle != NULL || battleAttemptTask_handle != NULL){vTaskDelay(pdMS_TO_TICKS(50));};
                //authentication back and forth (call function)
                challenger = false;
                debugBlink();
                xTaskCreate(&peerAuthTask, "peerAuthTask", 2048,  (void *)pal, 1, &peerAuthTask_handle);
                break;
              }


              if(peerData == AddressesInRange[i]){
                memcpy(allBroadcastData.ignoreIfAddress, defaultIgnoreAddress, 6);
                animation.set_curr_display("detecting");
                peerData->challenging = false;
                while(battleAttemptTask_handle != NULL){
                  //digitalWrite(2, HIGH);
                  vTaskDelay(pdMS_TO_TICKS(500));
                  //digitalWrite(2, LOW);
                };
                esp_now_del_peer(peerData->data.address);
                peerData = nullptr;
              }
              else if(peerData == nullptr){
                peerData = AddressesInRange[i];
                memcpy(peerInfo.peer_addr, peerData->data.address, 6);
                peerInfo.channel = 0;
                peerInfo.encrypt = false;

                if (esp_now_add_peer(&peerInfo) != ESP_OK){
                  Serial.println("Failed to add peer");
                  return;
                }
                memcpy(allBroadcastData.ignoreIfAddress, peerData->data.address, 6 * sizeof(uint8_t));
                peerData->challenging = true;
                animation.set_curr_display("battle_attempt");
                if (battleAttemptTask_handle == NULL){
                xTaskCreate(&battleAttemptTask, "battleAttemptTask", 2048,  (void *)pal, 1, &battleAttemptTask_handle);
                }
              }
              else if(peerData != AddressesInRange[i] && peerData != nullptr){
                memcpy(peerInfo.peer_addr, AddressesInRange[i]->data.address, 6);
                peerInfo.channel = 0;
                peerInfo.encrypt = false;

                if (esp_now_add_peer(&peerInfo) != ESP_OK){
                  Serial.println("Failed to add peer");
                  return;
                }
                uint8_t addressToBeDeleted[6];
                memcpy(addressToBeDeleted, peerData->data.address, 6);
                peerData->challenging = false;
                peerData = AddressesInRange[i];
                peerData->challenging = true;
                esp_now_del_peer(addressToBeDeleted);
                memcpy(allBroadcastData.ignoreIfAddress, peerData->data.address, 6 * sizeof(uint8_t));
              }
              break;
            }
          }
          broadcastUpdate = true;
        }
      }
    }
    else if(curr_display == "battle"){
      if(digitalRead(enter_b)){
        while(digitalRead(enter_b)){};
        if (animation.battleSelectedItem == "fight"){
          if (challenger){
            battleData attackData;
            memcpy(attackData.address, selfMACAddress, 6);
            strcpy(attackData.name, pal->get_name().c_str());
            attackData.hp = pal->get_hp();
            attackData.attackDamage = battleAttackGame();
            selfAttack = true;
            esp_now_send(peerData->data.address, (uint8_t *)&attackData, sizeof(attackData));
            peerBattleData.hp -= attackData.attackDamage;
            challenger = false;
          }
        }
        else if(animation.battleSelectedItem == "run" && challenger){
          battleData attackData;
          memcpy(attackData.address, selfMACAddress, 6);
          strcpy(attackData.name, pal->get_name().c_str());
          attackData.hp = pal->get_hp();
          attackData.attackDamage = -1;
          esp_now_send(peerData->data.address, (uint8_t *)&attackData, sizeof(attackData));
          surrender = true;
          animation.set_curr_display("menu");
          validatedCount = 0;
          vTaskDelay(pdMS_TO_TICKS(1500));
        }
      }
      if(digitalRead(scroll_b)){
        while(digitalRead(scroll_b)){};
        if (animation.battleSelectedItem == "fight"){
          animation.battleSelectedItem = "run";
        }
        else{
          animation.battleSelectedItem = "fight";
        }
      }
    }
    prev_display = curr_display;
    /*
    if(!peerValid){
      animation.set_curr_display("detecting");
      while(battleTask_handle!=NULL || peerAuthTask_handle != NULL){vTaskDelay(pdMS_TO_TICKS(50));}
      esp_now_unregister_recv_cb();
      if(broadcastingAddressTask_handle==NULL){
        xTaskCreate(&broadcastingAddressTask, "broadcastingAddressTask", 2048, (void *)pal, 1, &broadcastingAddressTask_handle);
      }
      vTaskDelay(pdMS_TO_TICKS(50));
      if(receivingAddressTask_handle == NULL){
        xTaskCreate(&receivingAddressTask, "receivingAddressTask", 2048, NULL, 1, &receivingAddressTask_handle);      
      }
      if(menu_display_handle==NULL){
        xTaskCreate(&menu_display_task, "menu_display_task", 2048, (void *)pal, 1, &menu_display_handle);
      }
      peerValid = true;
    }*/
  }
}
void time_update_task(void* pvParameter){
  Pal *pal = (Pal *)pvParameter;
  for (;;){

    if (pal->get_hunger()<100){
      pal->update_sec_until_hungrier(1);
    }
    //condition clause for hunger overtime
    if (pal->get_sec_until_hungrier() >= sec_per_hunger){
      pal->change_hunger(2);
      pal->set_sec_until_hungrier(0);
      Serial.println("Pal got hungrier! Hunger value: " + String(pal->get_hunger()));
    }

    //condition clause for when hunger gets below threshold and hp starts to decrease
    if (pal->get_hunger() >= hunger_threshold){
      if (pal->get_hp() > 0){
        pal->update_sec_until_weaker(1);
      }
      else{
        pal->alive = 0;
        Serial.println("Pet died!");
      }

      if (pal->get_sec_until_weaker() >= sec_per_hp){
        pal->change_hp(-2);
        pal->set_sec_until_weaker(0);
        Serial.println("Pal got weaker! HP value: " + String(pal->get_hp()));
      }
    }

    Serial.println(String(sec_per_hunger - pal->get_sec_until_hungrier()) + " seconds until more hunger");
    Serial.println(String(sec_per_hp - pal->get_sec_until_weaker()) + " seconds until more health lost");
    Serial.println("");
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void menu_display_task(void* pvParameter){
  Pal *pal = (Pal *)pvParameter;
  string prev_display = animation.get_curr_display();
  string curr_display;
  for (;;){
    curr_display = animation.get_curr_display();

    if(prev_display =="menu" && curr_display == "detecting"){
      animation.menu_detecting_transition();
    }
    else if(prev_display =="detecting" && curr_display == "menu"){
      //detecting screen leaving
    }

    if(curr_display == "menu"){
      animation.menu_display(0,0);
    }
    else if(curr_display == "detecting" || curr_display == "battle_attempt"){
      animation.detecting_display(0,0);
      animation.loading_dots_black(87,4);
      for (int i = 0; i < AddressesInRange.size(); i++){
        /*char macStr[18];
        snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X", 
           temp_addressesInRange[i]->address[0], temp_addressesInRange[i]->address[1], temp_addressesInRange[i]->address[2], 
           temp_addressesInRange[i]->address[3], temp_addressesInRange[i]->address[4], temp_addressesInRange[i]->address[5]);*/
        animation.print_peer(i,string(AddressesInRange[i]->data.name),AddressesInRange[i]->selected, AddressesInRange[i]->data.battle_attempt,
        AddressesInRange[i]->challenging);
      }
      animation.display_all();
    }
    vTaskDelay(pdMS_TO_TICKS(250));

      if (animation.get_curr_display() != "menu" && animation.get_curr_display() != "detecting" && animation.get_curr_display() != "battle_attempt")
      {
        menu_display_handle = NULL;
        vTaskDelete(NULL);
      } 
    prev_display = curr_display;
  }
}

void broadcastingAddressTask(void* pvParameter){
  Pal *pal = (Pal *)pvParameter;
            //espnow initialization
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK){
  //Serial.println("Error initializing ESP-NOW");
  }
          //registering broadcast address as peer
  esp_now_unregister_recv_cb();
  esp_now_register_recv_cb(OnReceive);
  esp_now_register_send_cb(OnSent);

  memcpy(peerInfo.peer_addr, allAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  //initialization
  strcpy(allBroadcastData.name, pal->get_name().c_str());
  memcpy(allBroadcastData.address, selfMACAddress, sizeof(selfMACAddress));
  allBroadcastData.battle_attempt = 0;
  allBroadcastData.notFree = 0;
  allBroadcastData.feedback = 0;
  memcpy(allBroadcastData.ignoreIfAddress, defaultIgnoreAddress, 6 * sizeof(uint8_t));

  for (;;){
    esp_now_send(allAddress, (uint8_t *)&allBroadcastData, sizeof(allBroadcastData));
    vTaskDelay(pdMS_TO_TICKS(1000));

    if (animation.get_curr_display() != "detecting"
    && animation.get_curr_display() != "battle_attempt")
    {
      esp_now_del_peer(allAddress);

      for (int i = 0; i < AddressesInRange.size(); i++){
        delete AddressesInRange[i];
      }
      AddressesInRange.clear();

      for (int i = 0; i < AddressesInRangeQueue.size(); i++){
        delete AddressesInRangeQueue[i];
      }
      AddressesInRangeQueue.clear();

      broadcastingAddressTask_handle = NULL;
      vTaskDelete(NULL);
    }
  }
}

void receivingAddressTask(void* pvParameter){
  AddressesInRange.clear();
  AddressesInRangeQueue.clear();
  peerData = nullptr;
  bool change;
  for (;;){
    change = false;
    if(!AddressesInRangeQueue.empty()){
      
      bool ignore = (AddressesInRangeQueue[0]->ignoreIfAddress[0] == selfMACAddress[0]) && (AddressesInRangeQueue[0]->ignoreIfAddress[1] == selfMACAddress[1]) && (AddressesInRangeQueue[0]->ignoreIfAddress[2] == selfMACAddress[2]) && (AddressesInRangeQueue[0]->ignoreIfAddress[3] == selfMACAddress[3]) && (AddressesInRangeQueue[0]->ignoreIfAddress[4] == selfMACAddress[4]) && (AddressesInRangeQueue[0]->ignoreIfAddress[5] == selfMACAddress[5]);

      if(ignore){
        delete AddressesInRangeQueue[0];
        AddressesInRangeQueue.erase(AddressesInRangeQueue.begin());
        //digitalWrite(2, HIGH);
        vTaskDelay(pdMS_TO_TICKS(400));
        //digitalWrite(2, LOW);
        continue;
      }

      uint8_t index;
      bool match = false;
      for (int i = 0; i < AddressesInRange.size(); i++){
        match = true;
        for (int j = 0; j < 6; j++){
          if (AddressesInRangeQueue[0]->address[j] != AddressesInRange[i]->data.address[j])
          {
            match = false;
            break;
          }
        }
        if (match){
          index = i;
          break;
        }
      }
      if (match){
        AddressesInRange[index]->remove_countdown = 8;
        AddressesInRange[index]->data.battle_attempt = AddressesInRangeQueue[0]->battle_attempt;
        //memcpy(&AddressesInRange[index]->data, AddressesInRangeQueue[0], sizeof(broadcastData));
        if (AddressesInRange[index]->data.battle_attempt){
          change = true;
        }
      }
      else if(!match && AddressesInRangeQueue[0]->battle_attempt != true)
      {
        localData *new_peer = new localData;
        memcpy(&new_peer->data, AddressesInRangeQueue[0], sizeof(broadcastData));
        new_peer->remove_countdown = 8;
        new_peer->challenging = false;
        if(AddressesInRange.empty()){
          new_peer->selected = true;
        }
        else{
          new_peer->selected = false;
        }
        AddressesInRange.push_back(new_peer);
        change = true;
      }
      delete AddressesInRangeQueue[0];
      AddressesInRangeQueue.erase(AddressesInRangeQueue.begin());
    }
    for (int i = 0; i < AddressesInRange.size(); i++){
      AddressesInRange[i]->remove_countdown--;
      if(AddressesInRange[i]->remove_countdown <= 0){
        if (AddressesInRange[i]->selected){
          if (i >= AddressesInRange.size()-1){
            if(AddressesInRange.size() != 1){
              AddressesInRange[0]->selected = true;
            }
          }
          else{
            AddressesInRange[i + 1]->selected = true;
          }
        }
        if (AddressesInRange[i] == peerData){
          peerData = nullptr;
        }
        delete AddressesInRange[i];
        AddressesInRange.erase(AddressesInRange.begin() + i);
        change = true;
      }
    }
    /*animation.set_placement();
    animation.display_num((uint8_t)AddressesInRange.size());
    animation.display_all();*/

    broadcastUpdate = change;

    if (animation.get_curr_display() != "detecting" &&
    animation.get_curr_display() != "battle_attempt")
    {
      esp_now_unregister_recv_cb();
      receivingAddressTask_handle = NULL;
      vTaskDelete(NULL);
    }
    vTaskDelay(pdMS_TO_TICKS(250));
  }
}

void battleAttemptTask(void* pvParameter){
  Pal *pal = (Pal *)pvParameter;
  debugBlink();
  broadcastData targetData;
  memcpy(targetData.address, selfMACAddress, 6);
  targetData.battle_attempt = true;
  memcpy(allBroadcastData.ignoreIfAddress, peerData->data.address, 6 * sizeof(uint8_t));
  for (;;){
    challenger = true;
    if(peerData != nullptr){
      esp_now_send(peerData->data.address, (uint8_t *)&targetData, sizeof(targetData));

      if (peerData->data.battle_attempt){
        //try to begin battle
        animation.set_curr_display("peerAuth");
        while(broadcastingAddressTask_handle != NULL || menu_display_handle != NULL || receivingAddressTask_handle != NULL){vTaskDelay(pdMS_TO_TICKS(50));};
        //authentication back and forth (call function)
        xTaskCreate(&peerAuthTask, "peerAuthTask", 2048,  (void *)pal, 1, &peerAuthTask_handle);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(500));
    if (animation.get_curr_display() != "battle_attempt"){
      if(animation.get_curr_display() != "peerAuth"){
        challenger = false;
      }
      memcpy(broadcastAddress, defaultIgnoreAddress, 6 * sizeof(uint8_t));
      memcpy(allBroadcastData.ignoreIfAddress, defaultIgnoreAddress, 6);
      battleAttemptTask_handle = NULL;
      vTaskDelete(NULL);
    }
  }
}

void peerAuthTask(void*pvParameter){
  int8_t battleValidationCountDown = 3;
  Pal *pal = (Pal *)pvParameter;
  memcpy(allBroadcastData.ignoreIfAddress, defaultIgnoreAddress, 6 * sizeof(uint8_t)); //remove address ignorance
  esp_now_unregister_recv_cb();
  esp_now_register_recv_cb(onReceiveBattle);
  for (;;){
    esp_now_send(peerData->data.address, (uint8_t *)selfMACAddress, sizeof(selfMACAddress)); //initiate back and forth validation
    vTaskDelay(pdMS_TO_TICKS(2000));
    if(battleValidationFeedback){
      battleValidationCountDown = 3;
      battleValidationFeedback = false;
    }
    else{
      battleValidationCountDown--;
    }

    if(validatedCount >= 3){
      animation.set_curr_display("battle");
    }
    if(animation.get_curr_display()=="battle" && battleTask_handle == NULL){
      xTaskCreate(&battleTask, "battleTask", 4096, (void *)pal, 1, &battleTask_handle);
    }
    if (battleValidationCountDown <= 0){
      peerValid = false;
      animation.set_curr_display("detecting");
    }

    if((animation.get_curr_display()!="peerAuth" && animation.get_curr_display()!="battle") || battleValidationCountDown <= 0){

      memcpy(allBroadcastData.ignoreIfAddress, defaultIgnoreAddress, 6);
      peerData = nullptr;
      peerAuthTask_handle = NULL;
      validatedCount = 0;

      animation.set_curr_display("menu");
      while(battleTask_handle!=NULL || peerAuthTask_handle != NULL || receivingAddressTask_handle != NULL || broadcastingAddressTask_handle != NULL || menu_display_handle != NULL){debugBlink();}
      AddressesInRange.clear();
      AddressesInRangeQueue.clear();
      esp_now_deinit();
      if(menu_display_handle==NULL){
        xTaskCreate(&menu_display_task, "menu_display_task", 2048, (void *)pal, 1, &menu_display_handle);
      }

      vTaskDelete(NULL);
    }
  }
}

void battleTask(void*pvParameter){
  Pal *pal = (Pal *)pvParameter;
  bool prevChallengerStatus = challenger;
  bool localChallengerStatus = challenger;
  battleData attackData;
  memcpy(attackData.address, selfMACAddress, 6);
  strcpy(attackData.name, pal->get_name().c_str());
  attackData.hp = pal->get_hp();
  attackData.attackDamage = 0;

  peerBattleData.hp = 100;
  esp_now_send(peerData->data.address, (uint8_t *)&attackData, sizeof(attackData));
  vTaskDelay(pdMS_TO_TICKS(100));
  esp_now_send(peerData->data.address, (uint8_t *)&attackData, sizeof(attackData));

  xTaskCreate(&palBattleTask, "palBattleTask", 4096, (void *)pal, 1, &palBattleTask_handle);
  for (;;){
    while(battleGameInProgress){ vTaskDelay(pdMS_TO_TICKS(100));}
    localChallengerStatus = challenger;
    vTaskDelay(pdMS_TO_TICKS(100));
    animation.clearAllExcpPal();
    animation.battleDisplay(String(peerBattleData.name), peerBattleData.hp);
    animation.battleBorderDisplay();
    animation.battleOptionsDisplay();
    if (localChallengerStatus)
    {
      if (prevChallengerStatus != localChallengerStatus)
      {
        oppAttack = true;
        animation.displayBattleDamaged(peerBattleData.attackDamage);
        animation.display_all();
        vTaskDelay(pdMS_TO_TICKS(2500));
        if (pal->get_hp() <= peerBattleData.attackDamage)
        {
          pal->set_hp(0);
        }
        else
        {
          pal->change_hp(-peerBattleData.attackDamage);
        }
      }
      animation.displayBattleAttack();
      }
      else{
        if(peerBattleData.attackDamage <0){
          animation.set_curr_display("menu");
          validatedCount = 0;

          animation.displayBattleRunning(string(peerBattleData.name));
          animation.display_all();

          vTaskDelay(pdMS_TO_TICKS(1500));
        }
        else{
         animation.displayBattleWaiting();
        }

      }
      animation.display_all();

    prevChallengerStatus = localChallengerStatus;
    if(pal->get_hp() <= 0){
      animation.set_curr_display("menu");
      vTaskDelay(pdMS_TO_TICKS(1000));
      animation.clearDisplay();
      animation.youLose();
      animation.display_all();
      animation.set_curr_display("menu");
      vTaskDelay(pdMS_TO_TICKS(2000));
      while(palBattleTask_handle!=NULL){};
      animation.set_curr_display("menu");
      validatedCount = 0;
    }
    else if(peerBattleData.hp <= 0){
      animation.set_curr_display("menu");
      vTaskDelay(pdMS_TO_TICKS(1000));
      animation.clearDisplay();
      animation.youWin();
      animation.display_all();
      animation.set_curr_display("menu");
      vTaskDelay(pdMS_TO_TICKS(2000));
      while(palBattleTask_handle!=NULL){};
      animation.set_curr_display("menu");
      validatedCount = 0;
    }

    if(animation.get_curr_display() != "battle" || peerBattleData.hp <= 0 || pal->get_hp() <= 0){
      if(surrender){

        animation.displayBattleAttackClear();
        animation.displayBattleSurrendered();
        animation.display_all();
        surrender = false;
      }
      while(palBattleTask_handle!=NULL){};
      battleInitializationSend = 0;
      battleTask_handle = NULL;
      vTaskDelete(NULL);
    }
  }
}

int8_t battleAttackGame(){
  battleGameInProgress = true;
  int8_t attack = 0;
  uint8_t barPlacement[21] = {1,1,1,2,2,4,4,8,16,32,63,127-32,127-16,127-8,127-4,127-4,127-2,127-2,127-1,127-1,127-1};
  int8_t state = 0;
  int8_t direction = 1;
  vTaskDelay(pdMS_TO_TICKS(250));
  while(true){
    animation.reflexGame(barPlacement[state]);
    animation.display_all();
    if(digitalRead(enter_b)){
      while(digitalRead(enter_b)){};
      animation.flashBar(barPlacement[state]);
      break;
    }
    state += direction;
    if(state>=21){
      direction = -1;
    }
    else if(state <= -1){
      direction = 1;
    }
    
    vTaskDelay(pdMS_TO_TICKS(50));
  }
  uint8_t position = barPlacement[state];
  if(position == 63){
    attack = 40;
  }
  else if(position == 1 || position == 126){
    attack = 0;
  }
  else if(position == 2 || position == 125){
    attack = 2;
  }
  else if(position == 4 || position == 123){
    attack = 4;
  }
  else if(position == 8 || position == 119){
    attack = 6;
  }
  else if(position == 16 || position == 111){
    attack = 10;
  }
  else if(position == 32 || position == 95){
    attack = 20;
  }
  animation.displayAttackPoint(attack);
  animation.display_all();
  vTaskDelay(pdMS_TO_TICKS(2500));
  battleGameInProgress = false;
  return attack;
}

void palBattleTask(void*pvParameter){
  Pal *pal = (Pal *)pvParameter;
  for (;;){
    bool done = animation.palBattleDisplay(selfAttack,oppAttack);
    if(oppAttack || selfAttack){
      if(selfAttack && done){
        selfAttack = false;
      }
      else if(oppAttack && done){
        oppAttack = false;
      }
    }
    animation.display_all();
    vTaskDelay(pdMS_TO_TICKS(100));

    if(animation.get_curr_display()!="battle"){
      battleInitializationSend = 0;
      palBattleTask_handle = NULL;
      vTaskDelete(NULL);
    }
  }

}

/*
String current_time(uint16_t*year, uint8_t*month, uint8_t*day, uint8_t*hours, uint8_t*minutes, uint8_t*seconds) {
  timeClient.update();
  time_t rawtime = timeClient.getEpochTime();

  *hours = timeClient.getHours();
  String meridiem = "AM";
  if (*hours>12){meridiem = "PM";};

   struct tm * ti;
   ti = localtime (&rawtime);

   *year = ti->tm_year + 1900;
   String yearStr = String(*year);

   *month = ti->tm_mon + 1;
   String monthStr = *month < 10 ? "0" + String(*month) : String(*month);

   *day = ti->tm_mday;
   String dayStr = *day < 10 ? "0" + String(*day) : String(*day);

   String hoursStr = (String)(*hours%12);

   *minutes = ti->tm_min;
   String minuteStr = *minutes < 10 ? "0" + String(*minutes) : String(*minutes);

   *seconds = ti->tm_sec;
   String secondStr = *seconds < 10 ? "0" + String(*seconds) : String(*seconds);

   return yearStr + "-" + monthStr + "-" + dayStr + " " +
          hoursStr + ":" + minuteStr + ":" + secondStr + " " + meridiem;
}*/