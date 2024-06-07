#ifndef PAL_H
#define PAL_H

#include <string>
#include <Arduino.h>
#include "nvs_flash.h"

using namespace std;

class Pal{
public:
    Pal();
    //Pal(string, int8_t, int8_t);
    string get_name();
    int8_t get_hp();
    int8_t get_hunger();
    uint32_t get_epoch();
    uint16_t get_sec_until_weaker();
    uint16_t get_sec_until_hungrier();
    int8_t alive;

    bool set_name(string);
    bool change_hp(int8_t);
    bool set_hp(int8_t);
    bool change_hunger(int8_t);
    bool set_hunger(int8_t);
    bool nvs_retrieve();
    bool update_time(uint32_t);
    bool update_sec_until_weaker(uint16_t);
    bool update_sec_until_hungrier(uint16_t);
    bool set_sec_until_weaker(uint16_t);
    bool set_sec_until_hungrier(uint16_t);
    bool set_alive_nvs(int8_t);

private:
    string name;
    int8_t hp;
    int8_t hunger;

    uint32_t epoch;
    uint16_t sec_until_weaker;
    uint16_t sec_until_hungrier;
    
};
#endif