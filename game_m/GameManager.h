#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Config.h"
#include "Player.h"
#include "Bullet.h"
#include "ItemDatabase.h"
#include "StoryManager.h"
#include "DialogueSystem.h"
#include "DialogueDatabase.h"
#include "ApartmentScene.h"  
#include "HallwayScene.h"   
#include "ElevatorScene.h"
#include "TechScene.h"
#include "HealthBar.h"

class GameManager {
private:
    sf::RenderWindow window;
    sf::View gameView;
    sf::View uiView;
    sf::Clock clock;

    ItemDatabase gameDb;
    StoryManager story;
    DialogueDatabase dialogueDb;
    DialogueSystem dialogue;

    Player hero;
    ApartmentScene apartmentScene;
    HallwayScene hallwayScene;
    ElevatorScene elevatorScene;
    TechScene techScene;

    HealthBar playerHPBar;
    HealthBar zombieHPBar;

    sf::Texture questBoxTex;
    sf::Sprite questBoxSprite;
    sf::Font questFont;
    sf::Text questText;

    sf::Texture bulletTex;
    std::vector<Bullet> activeBullets;
    bool wasShootingLastFrame;

    bool showInventory;
    bool pendingMedkitMessage;
    bool pendingGlockMessage;

    void processEvents();
    void update(float time);
    void render();

public:
    GameManager();
    void run();
};

#endif
