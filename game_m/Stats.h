#ifndef STATS_H
#define STATS_H

class Stats {
public:
    float health;
    float hunger;
    bool isAlive;

    Stats(); 
    void update(float time);
    void eat(float amount);
};

#endif
