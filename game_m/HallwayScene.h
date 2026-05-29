#ifndef HALLWAY_SCENE_H
#define HALLWAY_SCENE_H
#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include "NPC.h"
#include "Enemy.h"
#include "Interactable.h"
#include "Player.h"
#include "StoryManager.h"
#include "DialogueSystem.h"
#include "DialogueDatabase.h"

class HallwayScene {
public:
    sf::Texture bgTex;
    sf::Sprite bgSprite;
    bool isLoaded;

    Enemy zombie;
    Enemy zombieAmbush;
    NPC mark;

    Interactable doorCloset;
    Interactable apartment40;
    Interactable apartment40Note;
    Interactable hallwayElevator;

    sf::CircleShape keyGlow;
    sf::Vector2f keyPosition;

    bool nearZombie, nearCloset, nearApartment, nearNote, nearKey, nearAmbush, nearElevator;
    bool noteRead;
    bool isKeyPickedUp;
    bool isAmbushTriggered;

    void moveZombieTowardsPlayer(Enemy& targetZombie, float playerX, float speed, float time);
    void updateMarkPosition(float playerX, bool faceRight, float time);

    HallwayScene()
        : doorCloset("laptop_sprite.png", sf::Vector2f(220.f, 210.f), "closet_door", 60.f, 90.f),
        apartment40("laptop_sprite.png", sf::Vector2f(1200.f, 200.f), "door40", 70.f, 90.f),
        apartment40Note("laptop_sprite.png", sf::Vector2f(340.f, 240.f), "note40", 30.f, 20.f),
        hallwayElevator("laptop_sprite.png", sf::Vector2f(2320.f, 210.f), "hallway_elevator_shaft", 80.f, 110.f)
    {
        isLoaded = false;

        nearZombie = nearCloset = nearApartment = nearNote = nearKey = nearAmbush = nearElevator = false;
        noteRead = false;
        isKeyPickedUp = false;
        isAmbushTriggered = false;
        keyGlow.setRadius(4.f);
        keyGlow.setFillColor(sf::Color(255, 255, 150, 220));
        keyGlow.setOutlineColor(sf::Color::Black);
        keyGlow.setOutlineThickness(1.f);
        keyPosition = sf::Vector2f(400.f, 360.f);
    }

    void init() {
        if (isLoaded) return;

        if (!bgTex.loadFromFile("hallway.png")) {
            std::cout << "CRITICAL ERROR: hallway.png not found!" << std::endl;
        }
        bgTex.setSmooth(false);
        bgSprite.setTexture(bgTex);
        bgSprite.setScale(2400.f / bgTex.getSize().x, 400.f / bgTex.getSize().y);

        zombie.init("zombie_static.png", "zombie_walk.png", "zombie_attack.png", sf::Vector2f(400.f, 385.f));
        zombie.health = 60.f;

        zombieAmbush.init("zombie_static.png", "zombie_walk.png", "zombie_attack.png", sf::Vector2f(2450.f, 385.f));
        zombieAmbush.health = 40.f;

        mark.init("npc_sprite.png", "mark_move.png", sf::Vector2f(2225.f, 385.f));

        isLoaded = true;
        isAmbushTriggered = false;
        isKeyPickedUp = false;
    }

    void update(float time, Player& hero, StoryManager& story, DialogueSystem& dialogue, DialogueDatabase& dialogueDb) {
        if (!isLoaded) return;
        float playerX = hero.sprite.getPosition().x;

        if (!isAmbushTriggered && playerX <= 1250.f && !dialogue.isOpen && story.hallwayIntroPlayed) {
            isAmbushTriggered = true;
            zombieAmbush.setPosition(2380.f, 385.f);
        }

        if (zombie.health > 0) {

            static float zombieFrame = 0.f;
            zombieFrame += 0.005f * time;
            if (zombieFrame >= 4.f) zombieFrame = 0.f;

            if (story.hallwayIntroPlayed) {
                if (zombie.hitCooldown <= 0.f) {
                    zombie.setState(1, static_cast<int>(zombieFrame));
                }
                moveZombieTowardsPlayer(zombie, playerX, 0.04f, time);
            }
            keyPosition = sf::Vector2f(zombie.getPosition().x, 365.f);
            keyGlow.setPosition(keyPosition);
        }
        else {
            zombie.setState(0);
        }

        if (isAmbushTriggered && zombieAmbush.health > 0) {
            static float ambushFrame = 0.f;
            ambushFrame += 0.007f * time;
            if (ambushFrame >= 4.f) ambushFrame = 0.f;

            if (zombieAmbush.hitCooldown <= 0.f) {
                zombieAmbush.setState(1, static_cast<int>(ambushFrame));
            }
            moveZombieTowardsPlayer(zombieAmbush, playerX, 0.07f, time);
        }

        updateMarkPosition(playerX, hero.faceRight, time);
        updateDistances(playerX, dialogue.isOpen, story, hero.inventory.items["Keys"]);
    }

    void updateDistances(float playerX, bool dialogueIsOpen, StoryManager& story, int hasKeys) {
        nearZombie = (std::abs(playerX - zombie.getPosition().x) < 60.f);
        nearKey = (std::abs(playerX - keyPosition.x) < 45.f);
        nearAmbush = (std::abs(playerX - zombieAmbush.getPosition().x) < 60.f);

        // ИСПРАВЛЕНО: Считаем дистанцию до лифта на его реальной позиции 2320.f в конце коридора!
        nearElevator = (std::abs(playerX - 2320.f) < 70.f);
        hallwayElevator.showHint = (nearElevator && !dialogueIsOpen);

        nearCloset = (std::abs(playerX - 220.f) < 60.f);
        doorCloset.showHint = (nearCloset && !dialogueIsOpen);

        nearApartment = (std::abs(playerX - apartment40.sprite.getPosition().x) < 50.f);
        apartment40.showHint = (nearApartment && !dialogueIsOpen);

        nearNote = (std::abs(playerX - 340.f) < 50.f);
        apartment40Note.showHint = (nearNote && !dialogueIsOpen && !noteRead);
    }


    void handleInteraction(Player& hero, StoryManager& story, DialogueSystem& dialogue, DialogueDatabase& dialogueDb) {
        if (nearKey && zombie.health <= 0 && !isKeyPickedUp) {
            isKeyPickedUp = true;
            story.hallwayKeysFound = true;
            hero.inventory.addItem("Keys", 1);
            hero.showMessage(L"ПОЛУЧЕНО: СВЯЗКА КЛЮЧЕЙ ЭЛЕКТРИКА", sf::Color::Green);
            return;
        }
        if (nearElevator) {
            dialogue.startDialogue(dialogueDb.getDialogue("elevator_stuck"));
            return;
        }
        if (nearNote && !noteRead) {
            noteRead = true;
            story.hallwayNote2Read = true;
            hero.inventory.addItem("Note2", 1);
            dialogue.startDialogue(dialogueDb.getDialogue("hallway_note_uncle_tolya"));
            return;
        }
    }

    void draw(sf::RenderWindow& window) {
        window.draw(bgSprite);
        doorCloset.draw(window);
        apartment40.draw(window);
        zombie.draw(window);
        zombieAmbush.draw(window);

        if (zombie.health <= 0 && !isKeyPickedUp) {
            window.draw(keyGlow);
        }
        mark.draw(window);

        sf::Font font;
        if (!font.loadFromFile("PixeloidSans.ttf")) return;

        if (zombie.health <= 0 && !isKeyPickedUp && nearKey) {
            sf::Text hint(L"Нажмите Е (Поднять ключи)", font, 12);
            hint.setFillColor(sf::Color::Yellow);
            hint.setOutlineColor(sf::Color::Black);
            hint.setOutlineThickness(1.f);
            hint.setPosition(keyPosition.x - 65.f, keyPosition.y - 30.f);
            window.draw(hint);
        }
        if (hallwayElevator.showHint) {
            sf::Text hint(L"Нажмите Е (Осмотреть лифт)", font, 12);
            hint.setFillColor(sf::Color::Yellow);
            hint.setOutlineColor(sf::Color::Black);
            hint.setOutlineThickness(1.f);
            hint.setPosition(hallwayElevator.sprite.getPosition().x - 40.f, 220.f);
            window.draw(hint);
        }
        if (doorCloset.showHint) {
            sf::Text hint(L"Нажмите Е (Железная дверь)", font, 12);
            hint.setFillColor(sf::Color::Yellow);
            hint.setOutlineColor(sf::Color::Black);
            hint.setOutlineThickness(1.f);
            hint.setPosition(220.f - 45.f, 220.f);
            window.draw(hint);
        }
        if (!noteRead) {
            window.draw(apartment40Note.sprite);
            if (apartment40Note.showHint) {
                sf::Text hint(L"Нажмите Е (Записка соседа)", font, 12);
                hint.setFillColor(sf::Color::Yellow);
                hint.setOutlineColor(sf::Color::Black);
                hint.setOutlineThickness(1.f);
                hint.setPosition(340.f - 55.f, 220.f);
                window.draw(hint);
            }
        }
    }
};

inline void HallwayScene::moveZombieTowardsPlayer(Enemy& targetZombie, float playerX, float speed, float time) {
    if (targetZombie.health <= 0) {
        targetZombie.setState(0);
        return;
    }
    float zombieX = targetZombie.getPosition().x;
    if (zombieX < playerX - 10.f) {
        targetZombie.move(speed * time, 0.f);
        targetZombie.setFacing(true);
    }
    else if (zombieX > playerX + 10.f) {
        targetZombie.move(-speed * time, 0.f);
        targetZombie.setFacing(false);
    }
}

inline void HallwayScene::updateMarkPosition(float playerX, bool faceRight, float time) {
    float markX = mark.getPosition().x;
    float targetMarkX = playerX + (faceRight ? -55.f : 55.f);
    static float markFrame = 0.f;

    if (std::abs(markX - targetMarkX) > 15.f) {
        markFrame += 0.0025f * time;
        if (markFrame >= 4.f) markFrame = 0.f;

        if (markX < targetMarkX) {
            mark.move(0.08f * time, 0.f);
            mark.setState(true, static_cast<int>(markFrame));
            mark.moveSprite.setScale(-std::abs(mark.moveSprite.getScale().x), mark.moveSprite.getScale().y);
            mark.staticSprite.setScale(-std::abs(mark.staticSprite.getScale().x), mark.staticSprite.getScale().y);
        }
        else {
            mark.move(-0.08f * time, 0.f);
            mark.setState(true, static_cast<int>(markFrame));
            mark.moveSprite.setScale(std::abs(mark.moveSprite.getScale().x), mark.moveSprite.getScale().y);
            mark.staticSprite.setScale(std::abs(mark.staticSprite.getScale().x), mark.staticSprite.getScale().y);
        }
    }
    else {
        mark.setState(false);
        if (faceRight) {
            mark.staticSprite.setScale(-std::abs(mark.staticSprite.getScale().x), mark.staticSprite.getScale().y);
        }
        else {
            mark.staticSprite.setScale(std::abs(mark.staticSprite.getScale().x), mark.staticSprite.getScale().y);
        }
    }
}

#endif
