#include "Stats.h"

Stats::Stats() : health(100.f), hunger(100.f), isAlive(true) {}

void Stats::update(float time) {
    if (hunger > 0) hunger -= 0.0005f * time;
    if (hunger <= 0) health -= 0.001f * time;
    if (health <= 0) {
        health = 0;
        isAlive = false;
    }
}

void Stats::eat(float amount) {
    hunger += amount;
    if (hunger > 100) hunger = 100;
}