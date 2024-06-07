#include "Pal.h"
Pal::Pal(){
    this->name = "XXXX";
    this->hp = 0;
    this->hunger = 0;
    this->sec_until_weaker = 0;
    this->sec_until_hungrier = 0;
    this->alive = 0;
}

string Pal::get_name(){
    return this->name;
}

int8_t Pal::get_hp(){
    return this->hp;
}

int8_t Pal::get_hunger(){
    return this->hunger;
}

uint32_t Pal::get_epoch(){
    return this->epoch;
}

uint16_t Pal::get_sec_until_weaker(){
    return this->sec_until_weaker;
}

uint16_t Pal::get_sec_until_hungrier(){
    return this->sec_until_hungrier;
}

bool Pal::set_name(string name){
    bool status = 1;
    this->name = name;

    nvs_handle_t nvs_handle;
    if (nvs_open("storage", NVS_READWRITE, &nvs_handle) != ESP_OK) {
        status = 0;
    }

    if (nvs_set_str(nvs_handle, "name", this->name.c_str()) == ESP_OK) {
        status = 1;
    }
    else{
        status = 0;
    }
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);

    return status;
}
bool Pal::change_hp(int8_t shift){
    bool status = 1;
    this->hp += shift;

    if (this->hp >=100){
        this->hp = 100;
    }
    if (this->hp <=0){
        this->hp = 0;
    }
    
    nvs_handle_t nvs_handle;
    if (nvs_open("storage", NVS_READWRITE, &nvs_handle) != ESP_OK) {
        status = 0;
    }

    if (nvs_set_i8(nvs_handle, "hp", this->hp) == ESP_OK) {
        status = 1;
    }
    else{
        status = 0;
    }
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);

    return status;
}
bool Pal::set_hp(int8_t hp){
    bool status = 1;
    this->hp = hp;
    
    nvs_handle_t nvs_handle;
    if (nvs_open("storage", NVS_READWRITE, &nvs_handle) != ESP_OK) {
        status = 0;
    }

    if (nvs_set_i8(nvs_handle, "hp", this->hp) == ESP_OK) {
        status = 1;
    }
    else{
        status = 0;
    }
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);

    return status;
}
bool Pal::change_hunger(int8_t shift){
    bool status = 1;
    this->hunger += shift;

    if (this->hunger >=100){
        this->hunger = 100;
    }
    if (this->hunger <=0){
        this->hunger = 0;
    }
    
    nvs_handle_t nvs_handle;
    if (nvs_open("storage", NVS_READWRITE, &nvs_handle) != ESP_OK) {
        status = 0;
    }

    if (nvs_set_i8(nvs_handle, "hunger", this->hunger) == ESP_OK) {
        status = 1;
    }
    else{
        status = 0;
    }
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);

    return status;
}
bool Pal::set_hunger(int8_t hunger){
    bool status = 1;
    this->hunger = hunger;
    
    nvs_handle_t nvs_handle;
    if (nvs_open("storage", NVS_READWRITE, &nvs_handle) != ESP_OK) {
        status = 0;
    }

    if (nvs_set_i8(nvs_handle, "hunger", this->hunger) == ESP_OK) {
        status = 1;
    }
    else{
        status = 0;
    }
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);

    return status;
}

bool Pal::nvs_retrieve() {

    char cstr_name[32];
    size_t str_size = 32;

    nvs_handle_t nvs_handle;
    if (nvs_open("storage", NVS_READWRITE, &nvs_handle) != ESP_OK) {
        Serial.println("Error opening nvs...");
        return 0;
    }

    if (nvs_get_str(nvs_handle, "name", cstr_name, &str_size) == ESP_OK) {
        this->name = (string)cstr_name;
        Serial.println("Name found");
    }
    else{
        Serial.println("Name not found.");
        return 0;
    }

    if (nvs_get_i8(nvs_handle, "hp", &(this->hp)) == ESP_OK) {
        Serial.println("Hp value found");
    }
    else{
        Serial.println("Hp value not found.");
        return 0;
    }

    if (nvs_get_i8(nvs_handle, "hunger", &(this->hunger)) == ESP_OK) {
        Serial.println("Hunger value found");
    }
    else{
        Serial.println("Hunger value not found.");
        return 0;
    }

    if (nvs_get_u16(nvs_handle, "until_weaker", &(this->sec_until_weaker)) == ESP_OK) {
        Serial.println("sec_until_weaker found");
    }
    else{
        Serial.println("sec_until_weaker not found.");
        return 0;
    }

    if (nvs_get_u16(nvs_handle, "until_hungrier", &(this->sec_until_hungrier)) == ESP_OK) {
        Serial.println("sec_until_hungrier found");
    }
    else{
        Serial.println("sec_until_hungrier not found.");
        return 0;
    }

    if (nvs_get_i8(nvs_handle, "alive", &(this->alive)) == ESP_OK) {
        Serial.println("Alive status found");
    }
    else{
        Serial.println("Alive status not found.");
        return 0;
    }
    
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    return 1;
}

bool Pal::update_time(uint32_t epoch){
    bool status = 1;
    this->epoch = epoch;
    
    nvs_handle_t nvs_handle;
    if (nvs_open("storage", NVS_READWRITE, &nvs_handle) != ESP_OK) {
        status = 0;
    }

    if (nvs_set_u32(nvs_handle, "epoch", this->epoch) == ESP_OK) {
        status = 1;
    }
    else{
        status = 0;
    }
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);

    return status;
}

bool Pal::update_sec_until_weaker(uint16_t shift){
    bool status = 1;
    this->sec_until_weaker += shift;
    
    nvs_handle_t nvs_handle;
    if (nvs_open("storage", NVS_READWRITE, &nvs_handle) != ESP_OK) {
        status = 0;
    }

    if (nvs_set_u16(nvs_handle, "until_weaker", this->sec_until_weaker) == ESP_OK) {
        status = 1;
    }
    else{
        status = 0;
    }
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);

    return status;
}

bool Pal::update_sec_until_hungrier(uint16_t shift){
    bool status = 1;
    this->sec_until_hungrier += shift;
    
    nvs_handle_t nvs_handle;
    if (nvs_open("storage", NVS_READWRITE, &nvs_handle) != ESP_OK) {
        status = 0;
    }

    if (nvs_set_u16(nvs_handle, "until_hungrier", this->sec_until_hungrier) == ESP_OK) {
        status = 1;
    }
    else{
        status = 0;
    }
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);

    return status;
}

bool Pal::set_sec_until_weaker(uint16_t sec_until_weaker){
    bool status = 1;
    this->sec_until_weaker = sec_until_weaker;
    
    nvs_handle_t nvs_handle;
    if (nvs_open("storage", NVS_READWRITE, &nvs_handle) != ESP_OK) {
        status = 0;
    }

    if (nvs_set_u16(nvs_handle, "until_weaker", this->sec_until_weaker) == ESP_OK) {
        status = 1;
    }
    else{
        status = 0;
    }
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);

    return status;
}

bool Pal::set_sec_until_hungrier(uint16_t sec_until_hungrier){
    bool status = 1;
    this->sec_until_hungrier = sec_until_hungrier;
    
    nvs_handle_t nvs_handle;
    if (nvs_open("storage", NVS_READWRITE, &nvs_handle) != ESP_OK) {
        status = 0;
    }

    if (nvs_set_u16(nvs_handle, "until_hungrier", this->sec_until_hungrier) == ESP_OK) {
        status = 1;
    }
    else{
        status = 0;
    }
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);

    return status;
}

bool Pal::set_alive_nvs(int8_t alive){
    bool status = 1;
    this->alive = alive;
    
    nvs_handle_t nvs_handle;
    if (nvs_open("storage", NVS_READWRITE, &nvs_handle) != ESP_OK) {
        status = 0;
    }

    if (nvs_set_i8(nvs_handle, "alive", this->alive) == ESP_OK) {
        status = 1;
    }
    else{
        status = 0;
    }
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);

    return status;
}
/**Pal::Pal(string name, int8_t hp, int8_t hunger){

    char cstr_name[32];
    const char *new_name = name.c_str();
    size_t str_size = 32;

    nvs_handle_t nvs_handle;
    if (nvs_open("storage", NVS_READWRITE, &nvs_handle) != ESP_OK) {
        Serial.println("Error opening nvs...");
    }

    if (nvs_get_str(nvs_handle, "name", cstr_name, &str_size) == ESP_OK) {
        this->name = (string)cstr_name;
        Serial.println("Name found");
    }
    else{
        nvs_set_str(nvs_handle, "name", new_name);
        this->name = name;
        Serial.println("Name not found. New name written");
    }

    if (nvs_get_i8(nvs_handle, "hp", &(this->hp)) == ESP_OK) {
        Serial.println("Hp value found");
    }
    else{
        nvs_set_i8(nvs_handle, "hp", hp);
        this->hp = hp;
        Serial.println("Hp value not found. New value written");
    }

    if (nvs_get_i8(nvs_handle, "hunger", &(this->hunger)) == ESP_OK) {
        Serial.println("Hunger value found");
    }
    else{
        nvs_set_i8(nvs_handle, "hunger", hunger);
        this->hunger = hunger;
        Serial.println("Hunger value not found. New value written");
    }

    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
}**/