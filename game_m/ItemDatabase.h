#ifndef ITEM_DATABASE_H
#define ITEM_DATABASE_H

#include <string>
#include <map>
#include "Config.h"

struct ItemProps {
    float effectValue;
    int stackSize;     
};

class ItemDatabase {
public:
    std::map<std::string, ItemProps> data;

    ItemDatabase(Difficulty diff) {
        if (diff == EASY) {
            data["Medkit"] = { 50.f, 10 };
            data["Ammo"] = { 1.f, 90 };
            data["Flash"] = { 20.f, 5 };   
            data["Sandwich"] = { 30.f, 10 }; 
        }
        else if (diff == NORMAL) {
            data["Medkit"] = { 30.f, 5 };
            data["Ammo"] = { 1.f, 30 };
            data["Flash"] = { 15.f, 3 };
            data["Sandwich"] = { 15.f, 5 };
        }
        else { // HARD
            data["Medkit"] = { 15.f, 3 };
            data["Ammo"] = { 1.f, 10 };
            data["Flash"] = { 10.f, 2 };
            data["Sandwich"] = { 5.f, 2 };
        }


        data["Laptop"] = { 0.f, 1 };
        data["USB_Adapter"] = { 0.f, 1 };
        data["Stimulator"] = { 0.f, 1 };
        data["Radio"] = { 0.f, 1 };
        data["Token"] = { 0.f, 5 };
        data["Keys"] = { 0.f, 1 };
    }
};

#endif
