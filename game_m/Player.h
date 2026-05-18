#ifndef PLAYER_H
#define PLAYER_H

#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "Inventory.h" 

struct PlayerStats {
    float health;
    float maxHealth;
    float speedMultiplier;
    bool isFlashlightUpgraded;

    PlayerStats() {
        health = 100.f;
        maxHealth = 100.f;
        speedMultiplier = 1.f;
        isFlashlightUpgraded = false;
    }
};

class Player {
public:
    sf::Sprite sprite;
    sf::Texture textureIdle;
    sf::Texture textureShoot;

    int w, h;
    float currentFrame;
    float speed;
    bool isShooting;
    bool faceRight;
    float health;
    Inventory inventory;
    PlayerStats stats;
    class ItemDatabase* db;

    sf::Clock animationClock;

    sf::Text messageText;
    sf::Font messageFont;
    float messageTimer;
    float flashTimer;
    bool isFlashActive;

    Player(std::string pathIdle, std::string pathShoot, int width, int height);

    void handleInput(sf::Event& event);
    void update(float time);
    void draw(sf::RenderWindow& window);
    void showMessage(std::wstring message, sf::Color color);
    void drawMessage(sf::RenderWindow& window);
};

#endif