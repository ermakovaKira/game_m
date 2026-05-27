#pragma warning(disable : 4996)
#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H
#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include "Config.h"
#include "ItemDatabase.h"
#include "Player.h"
#include "Bullet.h"
#include "HealthBar.h"
#include "StoryManager.h"
#include "DialogueSystem.h"
#include "DialogueDatabase.h"
#include "MenuManager.h"
#include "ApartmentScene.h"
#include "HallwayScene.h"
#include "ElevatorScene.h"
#include "TechScene.h"
#include "MedScene.h"
#include "ServerScene.h"

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
    MenuManager menu;
    Player hero;
    ApartmentScene apartmentScene;
    HallwayScene hallwayScene;
    ElevatorScene elevatorScene;
    MedScene medScene;
    TechScene techScene;
    ServerScene serverScene;
    HealthBar playerHPBar;
    HealthBar zombieHPBar;
    sf::Texture questBoxTex;
    sf::Sprite questBoxSprite;
    sf::Font questFont;
    sf::Text questText;
    sf::Texture bulletTex;
    sf::Texture tunnelTex;
    sf::Texture graveyardTex;
    std::vector<Bullet> activeBullets;
    void spawnPlayerBullet();
    bool wasShootingLastFrame;

    bool showInventory;
    bool pendingMedkitMessage;
    bool pendingGlockMessage;
    bool isGamePaused;
    bool isHeroDead;
    sf::Clock techSceneClock;
    bool techClockStarted;
    bool failedCloseRange;
    int totalShots;
    int totalHits;
    bool usedMedkitOnHard;
    sf::Text achievementPopupText;
    float achievementPopupTimer;
    bool showAchievementPopup;
    void checkBulletCollisions(float time, Enemy& targetEnemy, float cameraX, const std::wstring& hitMessage, sf::Color hitColor);
    void saveCurrentProgress(int nextSceneNum);
    void processEvents();
    void update(float time);
    void render();
    void triggerAchievementNotification(int index);
    bool isGamePassed;
    sf::Music menuMusic;
    sf::Music gameMusic;
    sf::SoundBuffer shootBuffer;
    sf::SoundBuffer hitBuffer;
    sf::Sound shootSound;
    sf::Sound hitSound;
    bool isSoundMenuOpen;

public:
    GameManager();
    void run();
};

#endif
