#ifndef ELEVATOR_SCENE_H
#define ELEVATOR_SCENE_H

#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <vector>
#include "NPC.h" 
#include "Interactable.h"
#include "Player.h"
#include "StoryManager.h"
#include "DialogueSystem.h"
#include "DialogueDatabase.h"

class ElevatorScene {
public:
    sf::Texture bgTex;
    sf::Sprite bgSprite;
    bool isLoaded;

    NPC mark;

    sf::Sprite bossSprite;
    sf::Texture bossTex;
    float bossHealth;
    bool isBossSpawned;
    float bossAnimFrame;

    sf::RectangleShape hackWindow;
    sf::RectangleShape targetPoints[4];
    sf::RectangleShape waveMarker;

    float markerX;
    float markerSpeed;
    int successfulHits;
    bool waveDirectionRight;
    bool pointHitTracked[4];
    bool wasSpacePressedLastFrame;

    bool nearPanel, nearElevator, nearShield;
    bool hackStarted;
    bool hackSuccess;
    bool isMinigameActive;
    bool ammoCollected;

    Interactable securityPanel;
    Interactable offlineElevator;
    Interactable fireShield;

public:
    ElevatorScene()
        : securityPanel("laptop_sprite.png", sf::Vector2f(80.f, 210.f), "panel_hack", 45.f, 60.f),
        offlineElevator("laptop_sprite.png", sf::Vector2f(800.f, 210.f), "elevator_shaft", 120.f, 110.f),
        fireShield("laptop_sprite.png", sf::Vector2f(1450.f, 210.f), "fire_shield", 50.f, 60.f)
    {
        isLoaded = false;
        nearPanel = nearElevator = nearShield = false;
        hackStarted = false;
        hackSuccess = false;
        isMinigameActive = false;
        wasSpacePressedLastFrame = false;
        isBossSpawned = false;
        ammoCollected = false;
        bossHealth = 100.f;
        bossAnimFrame = 0.f;

        markerX = 0.f;
        markerSpeed = 0.08f;
        successfulHits = 0;
        waveDirectionRight = true;

        hackWindow.setSize(sf::Vector2f(300.f, 120.f));
        hackWindow.setFillColor(sf::Color(0, 15, 0, 240));
        hackWindow.setOutlineThickness(2.f);
        hackWindow.setOutlineColor(sf::Color(0, 255, 0));
        hackWindow.setPosition(250.f, 140.f);

        waveMarker.setSize(sf::Vector2f(8.f, 8.f));
        waveMarker.setFillColor(sf::Color::Green);
        waveMarker.setOrigin(4.f, 4.f);
        waveMarker.setOutlineThickness(1.f);
        waveMarker.setOutlineColor(sf::Color::Black);

        for (int i = 0; i < 4; i++) {
            targetPoints[i].setSize(sf::Vector2f(10.f, 10.f));
            targetPoints[i].setFillColor(sf::Color::Red);
            targetPoints[i].setOrigin(5.f, 5.f);
            targetPoints[i].setOutlineThickness(1.f);
            targetPoints[i].setOutlineColor(sf::Color::White);
            pointHitTracked[i] = false;
        }
    }

    void init() {
        if (isLoaded) return;
        if (!bgTex.loadFromFile("elevator_hall.png")) {
            std::cout << "CRITICAL ERROR: elevator_hall.png not found!" << std::endl;
        }
        bgTex.setSmooth(false);
        bgSprite.setTexture(bgTex, true);
        bgSprite.setScale(1600.f / bgTex.getSize().x, 400.f / bgTex.getSize().y);

        if (bossTex.loadFromFile("zombie_walk.png")) {
            bossSprite.setTexture(bossTex);
            bossSprite.setTextureRect(sf::IntRect(0, 0, bossTex.getSize().x / 4, bossTex.getSize().y));
            bossSprite.setOrigin((bossTex.getSize().x / 4) / 2.f, bossTex.getSize().y);
            bossSprite.setScale(-1.4f, 1.4f);
            bossSprite.setColor(sf::Color(240, 100, 100));
            bossSprite.setPosition(800.f, 385.f);
        }

        mark.init("npc_sprite.png", "mark_move.png", sf::Vector2f(1350.f, 385.f));
        isLoaded = true;
    }

    void resetMinigame() {
        successfulHits = 0;
        markerX = 0.f;
        waveDirectionRight = true;
        wasSpacePressedLastFrame = false;
        for (int i = 0; i < 4; i++) {
            targetPoints[i].setFillColor(sf::Color::Red);
            pointHitTracked[i] = false;
        }
    }

    void update(float time, Player& hero, StoryManager& story, DialogueSystem& dialogue, DialogueDatabase& dialogueDb) {
        if (!isLoaded) return;

        float playerX = hero.sprite.getPosition().x;
        updateDistances(playerX, dialogue.isOpen);

        float secureTime = (time > 100.f) ? 16.f : time;

        if (isBossSpawned && bossHealth > 0 && !dialogue.isOpen) {
            float bX = bossSprite.getPosition().x;

            bossAnimFrame += 0.005f * secureTime;
            if (bossAnimFrame >= 4.f) bossAnimFrame = 0.f;
            int frameWidth = bossTex.getSize().x / 4;
            bossSprite.setTextureRect(sf::IntRect(int(bossAnimFrame) * frameWidth, 0, frameWidth, bossTex.getSize().y));

            if (!isMinigameActive) {
                if (bX < playerX - 15.f) bossSprite.move(0.04f * secureTime, 0.f);
                else if (bX > playerX + 15.f) bossSprite.move(-0.04f * secureTime, 0.f);
            }

            if (std::abs(playerX - bX) < 65.f) {
                hero.stats.health -= 0.12f * secureTime;
                hero.health = hero.stats.health;
                if (isMinigameActive) {
                    isMinigameActive = false;
                    hero.showMessage(L"ÂÇËÎÌ ÏÐÅÐÂÀÍ ÓÄÀÐÎÌ ÌÓÒÀÍÒÀ!", sf::Color::Red);
                }
            }
        }

        if (isMinigameActive) {
            if (!nearPanel) {
                isMinigameActive = false;
                hero.showMessage(L"ÑÎÅÄÈÍÅÍÈÅ ÏÎÒÅÐßÍÎ! ÂÅÐÍÈÒÅÑÜ Ê ÏÀÍÅËÈ!", sf::Color::Red);
                return;
            }

            float startX = hackWindow.getPosition().x + 30.f;
            float endX = hackWindow.getPosition().x + 270.f;
            float totalWidth = endX - startX;
            float centerY = hackWindow.getPosition().y + 60.f;

            for (int i = 0; i < 4; i++) {
                float targetMarkerX = totalWidth * (0.2f * (i + 1));
                float pX = startX + targetMarkerX;
                float pY = centerY + std::sin((targetMarkerX / totalWidth * 5.f) * 1.5f) * 30.f;
                targetPoints[i].setPosition(pX, pY);
            }

            if (waveDirectionRight) {
                markerX += markerSpeed * secureTime;
                if (startX + markerX >= endX) waveDirectionRight = false;
            }
            else {
                markerX -= markerSpeed * secureTime;
                if (markerX <= 0.f) waveDirectionRight = true;
            }

            float currentMarkerGlobalX = startX + markerX;
            float currentMarkerGlobalY = centerY + std::sin((markerX / totalWidth * 5.f) * 1.5f) * 30.f;
            waveMarker.setPosition(currentMarkerGlobalX, currentMarkerGlobalY);

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
                if (!wasSpacePressedLastFrame) {
                    wasSpacePressedLastFrame = true;
                    bool pointHit = false;

                    for (int i = 0; i < 4; i++) {
                        if (!pointHitTracked[i]) {
                            float dist = std::abs(currentMarkerGlobalX - targetPoints[i].getPosition().x);
                            if (dist < 22.f) {
                                targetPoints[i].setFillColor(sf::Color::Green);
                                pointHitTracked[i] = true;
                                successfulHits++;
                                hero.showMessage(L"ÐÅÇÎÍÀÍÑ ÑÈÍÕÐÎÍÈÇÈÐÎÂÀÍ!", sf::Color::Green);
                                pointHit = true;
                                break;
                            }
                        }
                    }

                    if (!pointHit) {
                        hero.showMessage(L"ÑÁÎÉ ×ÀÑÒÎÒÛ!", sf::Color::Red);
                    }

                    if (successfulHits >= 4) {
                        isMinigameActive = false;
                        hackSuccess = true;
                        story.elevatorHackSuccess = true;
                        hero.showMessage(L"ÑÈÑÒÅÌÀ ÏÅÐÅÇÀÏÓÙÅÍÀ! ËÈÔÒ ÎÒÊÐÛÒ!", sf::Color::Green);
                    }
                }
            }
            else {
                wasSpacePressedLastFrame = false;
            }
            return;
        }

        if (!dialogue.isOpen) {
            float markX = mark.getPosition().x;
            float targetMarkX = playerX + (hero.faceRight ? -55.f : 55.f);

            static float markFrame = 0.f;
            if (std::abs(markX - targetMarkX) > 15.f) {
                markFrame += 0.0025f * secureTime;
                if (markFrame >= 4.f) markFrame = 0.f;

                if (markX < targetMarkX) {
                    mark.move(0.08f * secureTime, 0.f);
                    mark.setState(true, static_cast<int>(markFrame));
                    mark.moveSprite.setScale(-std::abs(mark.moveSprite.getScale().x), mark.moveSprite.getScale().y);
                }
                else {
                    mark.move(-0.08f * secureTime, 0.f);
                    mark.setState(true, static_cast<int>(markFrame));
                    mark.moveSprite.setScale(std::abs(mark.moveSprite.getScale().x), mark.moveSprite.getScale().y);
                }
            }
            else {
                mark.setState(false);
            }
        }
    }

    void updateDistances(float playerX, bool dialogueIsOpen) {
        nearPanel = (std::abs(playerX - 80.f) < 65.f);
        securityPanel.showHint = (nearPanel && !dialogueIsOpen && !hackSuccess);

        nearElevator = (std::abs(playerX - 800.f) < 90.f);
        offlineElevator.showHint = (nearElevator && !dialogueIsOpen);

        nearShield = (std::abs(playerX - 1450.f) < 60.f);
        fireShield.showHint = (nearShield && !dialogueIsOpen && !ammoCollected);
    }

    void handleInteraction(Player& hero, StoryManager& story, DialogueSystem& dialogue, DialogueDatabase& dialogueDb) {
        if (nearPanel) {
            if (!hackStarted) {
                hackStarted = true;
                isBossSpawned = true;
                dialogue.startDialogue(dialogueDb.getDialogue("elevator_panel_hack_start"));
            }
            else if (!hackSuccess) {
                if (!isMinigameActive) {
                    resetMinigame();
                }
                isMinigameActive = true;
                story.elevatorInspected = true;
            }
            return;
        }

        if (nearShield && !ammoCollected) {
            ammoCollected = true;
            hero.inventory.addItem("Ammo", 30);
            hero.showMessage(L"ÍÀÉÄÅÍ ÀÂÀÐÈÉÍÛÉ ÇÀÏÀÑ ÑÅÊÒÎÐÀ (+30 ÏÀÒÐÎÍÎÂ)", sf::Color::Green);
            return;
        }

        if (nearElevator) {
            if (hackSuccess) {
                story.currentScene = 4;
            }
            else {
                dialogue.startDialogue(dialogueDb.getDialogue("elevator_stuck"));
            }
            return;
        }
    }

    void draw(sf::RenderWindow& window) {
        if (!isLoaded) return;

        bgSprite.setTexture(bgTex, false);
        bgSprite.setColor(sf::Color::White);
        window.draw(bgSprite);

        if (isBossSpawned && bossHealth > 0) {
            window.draw(bossSprite);
        }

        mark.draw(window);

        if (isMinigameActive) {
            window.draw(hackWindow);
            for (int i = 0; i < 4; i++) {
                window.draw(targetPoints[i]);
            }
            window.draw(waveMarker);
        }

        sf::Font font;
        if (!font.loadFromFile("PixeloidSans.ttf")) return;

        if (offlineElevator.showHint) {
            sf::Text hint(hackSuccess ? L"Íàæìèòå E (Øàãíóòü â ëèôò)" : L"Íàæìèòå E (Îñìîòð ëèôòà)", font, 12);
            hint.setPosition(800.f - 75.f, 220.f);
            window.draw(hint);
        }

        if (securityPanel.showHint) {
            sf::Text hint(isMinigameActive ? L"ÂÇËÎÌ Â ÏÐÎÖÅÑÑÅ..." : L"Íàæìèòå E (Ïîäêëþ÷èòü íîóòáóê)", font, 12);
            hint.setPosition(80.f - 85.f, 220.f);
            window.draw(hint);
        }

        if (fireShield.showHint) {
            sf::Text hint(L"Íàæìèòå E (Îòêðûòü ïîæàðíûé ùèòîê)", font, 12);
            hint.setPosition(1450.f - 95.f, 220.f);
            window.draw(hint);
        }
    }
};

#endif
