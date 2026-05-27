#ifndef MED_SCENE_H
#define MED_SCENE_H

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

class MedScene {
public:
    sf::Texture bgTex;
    sf::Sprite bgSprite;
    bool isLoaded;

    NPC mark;
    
    Enemy zombie1; 
    Enemy zombie2; 

    Interactable operatingTable;   
    Interactable quarantineSwitch; 

    bool nearTable, nearSwitch;
    bool cardFound;
    bool defenseActive;
    bool quarantineBypassed;
    float defenseTimer;

    bool successDialoguePlayed;

public:
    MedScene() 
        : operatingTable("laptop_sprite.png", sf::Vector2f(350.f, 210.f), "op_table", 70.f, 60.f),
          quarantineSwitch("laptop_sprite.png", sf::Vector2f(1150.f, 210.f), "quarantine_panel", 50.f, 80.f)
    {
        isLoaded = false;
        nearTable = nearSwitch = cardFound = defenseActive = quarantineBypassed = false;
        successDialoguePlayed = false;
        defenseTimer = 20000.f; 
    }

    void init() {
        if (isLoaded) return;

        if (!bgTex.loadFromFile("med_quarantine.png")) {
            std::cout << "CRITICAL ERROR: med_quarantine.png not found!" << std::endl;
        }
        bgTex.setSmooth(false);
        bgSprite.setTexture(bgTex, true);
        bgSprite.setScale(1600.f / bgTex.getSize().x, 400.f / bgTex.getSize().y);
        
        mark.init("npc_sprite.png", "mark_move.png", sf::Vector2f(150.f, 385.f));

        zombie1.init("zombie_static.png", "zombie_walk.png", "zombie_attack.png", sf::Vector2f(600.f, 385.f));
        zombie2.init("zombie_static.png", "zombie_walk.png", "zombie_attack.png", sf::Vector2f(900.f, 385.f));
        
        zombie1.health = 50.f;
        zombie2.health = 50.f;

        isLoaded = true;
    }

    void update(float time, Player& hero, StoryManager& story, DialogueSystem& dialogue, DialogueDatabase& dialogueDb) {
        if (!isLoaded) return;

        float playerX = hero.sprite.getPosition().x;

        if (defenseActive && !quarantineBypassed) {
            if (!dialogue.isOpen) {
                defenseTimer -= time;
            }

            if (defenseTimer <= 0.f) {
                defenseTimer = 0.f;
                quarantineBypassed = true;
                story.medQuarantineBypassed = true;
                hero.showMessage(L"ØËŞÇ ÄÅÇÈÍÔÅÊÖÈÈ ÎÒÊĞÛÒ!", sf::Color::Green);
            }

            if (zombie1.health > 0 && !dialogue.isOpen) {
                float zX = zombie1.getPosition().x;

                if (zombie1.hitCooldown <= 0.f) {
                    zombie1.setState(1, 0);
                }

                if (zX < playerX) zombie1.move(0.05f * time, 0.f);
                else zombie1.move(-0.05f * time, 0.f);
            }

            if (zombie2.health > 0 && !dialogue.isOpen) {
                float zX = zombie2.getPosition().x;

                if (zombie2.hitCooldown <= 0.f) {
                    zombie2.setState(1, 0);
                }

                if (zX < playerX) zombie2.move(0.05f * time, 0.f);
                else zombie2.move(-0.05f * time, 0.f);
            }
        }

        if (quarantineBypassed && !successDialoguePlayed && !dialogue.isOpen) {
            if (zombie1.health <= 0 && zombie2.health <= 0) {
                successDialoguePlayed = true;
                dialogue.startDialogue(dialogueDb.getDialogue("med_quarantine_success"));
            }
        }

        if (!dialogue.isOpen) {
            float markX = mark.getPosition().x;
            float targetMarkX = playerX + (hero.faceRight ? -55.f : 55.f);
            static float markFrame = 0.f;

            if (std::abs(markX - targetMarkX) > 15.f) {
                markFrame += 0.0025f * time;
                if (markFrame >= 4.f) markFrame = 0.f;

                if (markX < targetMarkX) {
                    mark.move(0.08f * time, 0.f);
                    mark.setState(true, static_cast<int>(markFrame));
                    mark.moveSprite.setScale(-std::abs(mark.moveSprite.getScale().x), mark.moveSprite.getScale().y);
                }
                else {
                    mark.move(-0.08f * time, 0.f);
                    mark.setState(true, static_cast<int>(markFrame));
                    mark.moveSprite.setScale(std::abs(mark.moveSprite.getScale().x), mark.moveSprite.getScale().y);
                }
            }
            else {
                mark.setState(false);
            }
        }

        updateDistances(playerX, dialogue.isOpen);
    }


    void updateDistances(float playerX, bool dialogueIsOpen) {
        nearTable = (std::abs(playerX - 350.f) < 65.f);
        operatingTable.showHint = (nearTable && !dialogueIsOpen && !cardFound);
        nearSwitch = (std::abs(playerX - 1150.f) < 70.f);
        quarantineSwitch.showHint = (nearSwitch && !dialogueIsOpen && cardFound && !quarantineBypassed);
    }

    void handleInteraction(Player& hero, StoryManager& story, DialogueSystem& dialogue, DialogueDatabase& dialogueDb) {
        if (nearTable && !cardFound) {
            cardFound = true;
            story.medCardFound = true; 
            hero.inventory.addItem("Keys", 1); 
            dialogue.startDialogue(dialogueDb.getDialogue("tech_inspect_terminal")); 
            hero.showMessage(L"ÍÀÉÄÅÍÀ ÊËŞ×-ÊÀĞÒÀ ÑÒÀĞØÅÃÎ ÂÈĞÓÑÎËÎÃÀ", sf::Color::Green);
            return;
        }
        if (nearSwitch && cardFound && !defenseActive) {
            defenseActive = true;
            story.medQuarantineStarted = true; 
            dialogue.startDialogue(dialogueDb.getDialogue("med_quarantine_start")); 
            hero.showMessage(L"ÊÀĞÀÍÒÈÍ! ÀÊÒÈÂÀÖÈß ÇÀÙÈÒÍÛÕ ÏĞÎÒÎÊÎËÎÂ ØËŞÇÀ!", sf::Color::Red);
            return;
        }
    }

    void draw(sf::RenderWindow& window) {
        if (!isLoaded) return;
        window.draw(bgSprite);
        mark.draw(window);

        if (defenseActive) {
            if (zombie1.health > 0) zombie1.draw(window);
            if (zombie2.health > 0) zombie2.draw(window);
        }

        sf::Font font;
        if (!font.loadFromFile("PixeloidSans.ttf")) return;

        if (operatingTable.showHint) {
            sf::Text hint(L"Íàæìèòå E (Îáûñêàòü õèğóğãè÷åñêèå ñòîëû)", font, 12);
            hint.setPosition(350.f - 85.f, 220.f);
            window.draw(hint);
        }
        if (quarantineSwitch.showHint && !defenseActive) {
            sf::Text hint(L"Íàæìèòå E (Ïğîñêàíèğîâàòü êëş÷-êàğòó)", font, 12);
            hint.setPosition(1150.f - 95.f, 220.f);
            window.draw(hint);
        }
        if (defenseActive && !quarantineBypassed) {
            sf::Text timerText(L"ÒĞÅÂÎÃÀ! ÄÅÇÈÍÔÅÊÖÈß ØËŞÇÀ: " + std::to_wstring(static_cast<int>(defenseTimer / 1000.f)) + L"ñ", font, 13);
            timerText.setFillColor(sf::Color::Red);
            timerText.setPosition(250.f, 55.f);
            window.draw(timerText);
        }
    }
};

#endif
