#ifndef TECH_SCENE_H
#define TECH_SCENE_H

#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <string>
#include "NPC.h"
#include "Interactable.h"
#include "Player.h"
#include "StoryManager.h"
#include "DialogueSystem.h"
#include "DialogueDatabase.h"

class TechScene {
public: 
    sf::Texture bgTex;
    sf::Sprite bgSprite;
    bool isLoaded;

    NPC mark;

    Interactable techCorpse;
    Interactable techTerminal;
    Interactable valvePipe;

    sf::CircleShape corpseGlow;

    bool nearCorpse, nearTerminal, nearValve;
    bool pdaFound;
    bool terminalBypassed;
    bool gasCleared;

    float gasTimer;
    float damageInterval;

    bool isCodeInputActive;
    std::wstring playerEnteredCode;
    sf::RectangleShape codeInputWindow;

    float currentGreenTint;
    float currentRedTint;

    TechScene()
        : techCorpse("laptop_sprite.png", sf::Vector2f(450.f, 210.f), "corpse_tech", 60.f, 60.f),
        techTerminal("laptop_sprite.png", sf::Vector2f(1100.f, 210.f), "terminal_tech", 65.f, 80.f),
        valvePipe("laptop_sprite.png", sf::Vector2f(750.f, 210.f), "valve_tech", 50.f, 70.f)
    {
        isLoaded = false;
        nearCorpse = nearTerminal = nearValve = false;
        pdaFound = false;
        terminalBypassed = false;
        gasCleared = false;
        gasTimer = 45000.f;
        damageInterval = 0.f;

        isCodeInputActive = false;
        playerEnteredCode = L"";

        codeInputWindow.setSize(sf::Vector2f(260.f, 80.f));
        codeInputWindow.setFillColor(sf::Color(15, 15, 15, 245));
        codeInputWindow.setOutlineThickness(2.f);
        codeInputWindow.setOutlineColor(sf::Color(0, 255, 0));

        corpseGlow.setRadius(4.f);
        corpseGlow.setFillColor(sf::Color(255, 100, 100, 220));
        corpseGlow.setOutlineColor(sf::Color::Black);
        corpseGlow.setOutlineThickness(1.f);
        corpseGlow.setPosition(450.f, 250.f);

        currentGreenTint = 255.f;
        currentRedTint = 150.f;
    }

    void init() {
        if (isLoaded) return;
        if (!bgTex.loadFromFile("tech_floor.png")) {
            std::cout << "CRITICAL ERROR: tech_floor.png not found!" << std::endl;
        }
        bgTex.setSmooth(false);
        bgSprite.setTexture(bgTex, true);
        bgSprite.setScale(1600.f / bgTex.getSize().x, 400.f / bgTex.getSize().y);
        mark.init("npc_sprite.png", "mark_move.png", sf::Vector2f(150.f, 385.f));
        isLoaded = true;
    }

    void handleTextInMinigame(sf::Event& event) {
        if (!isCodeInputActive) return;

        if (event.type == sf::Event::TextEntered) {
            if (event.text.unicode >= '0' && event.text.unicode <= '9') {
                if (playerEnteredCode.length() < 4) {
                    playerEnteredCode += static_cast<wchar_t>(event.text.unicode);
                }
            }
        }
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::BackSpace && !playerEnteredCode.empty()) {
                playerEnteredCode.pop_back();
            }
            if (event.key.code == sf::Keyboard::Enter) {
                if (playerEnteredCode == L"4518") {
                    terminalBypassed = true;
                    isCodeInputActive = false;
                    playerEnteredCode = L"";
                }
                else {
                    playerEnteredCode = L"";
                }
            }
            if (event.key.code == sf::Keyboard::Escape) {
                isCodeInputActive = false;
                playerEnteredCode = L"";
            }
        }
    }

    void update(float time, Player& hero, StoryManager& story, DialogueSystem& dialogue, DialogueDatabase& dialogueDb) {
        if (!isLoaded) return;

        if (isCodeInputActive) return;

        if (terminalBypassed && !gasCleared && !dialogue.isOpen) {
            static bool wasHintShown = false;
            if (!wasHintShown) {
                wasHintShown = true;
                hero.showMessage(L"*ÂÅÍÒÈËßÖÈß ÇÀÃÓÄÅËÀ* Àâòîìàòèêà ñáğîøåíà! Ñğî÷íî êğóòè âåíòèëü!", sf::Color::Cyan);
            }
        }

        if (!gasCleared && !dialogue.isOpen) {
            gasTimer -= time;
            if (gasTimer < 0.f) gasTimer = 0.f;
            damageInterval += time;
            if (damageInterval >= 1000.f) {
                damageInterval = 0.f;
                hero.stats.health -= 2.f;
                hero.health = hero.stats.health;
                if (gasTimer <= 0.f || hero.stats.health <= 0.f) {
                    hero.stats.health = 0.f;
                    hero.health = 0.f;
                    hero.showMessage(L"ÅÂÀ ÇÀÄÎÕÍÓËÀÑÜ Â ÒÎÊÑÈ×ÍÎÌ ÎÁËÀÊÅ!", sf::Color::Red);
                }
                else {
                    hero.showMessage(L"*êàøåëü* ÃÀÇ ÎÒĞÀÂËßÅÒ ÂÎÇÄÓÕ!", sf::Color::Magenta);
                }
            }
        }

        if (gasCleared && (currentRedTint < 255.f)) {
            currentRedTint += 0.05f * time;
            currentGreenTint -= 0.02f * time;
            if (currentRedTint > 255.f) currentRedTint = 255.f;
            if (currentGreenTint < 255.f) currentGreenTint = 255.f;
        }

        float playerX = hero.sprite.getPosition().x;

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
        updateDistances(playerX, dialogue.isOpen, story);
    }

    void updateDistances(float playerX, bool dialogueIsOpen, StoryManager& story) {
        nearCorpse = (std::abs(playerX - 450.f) < 65.f);
        techCorpse.showHint = (nearCorpse && !dialogueIsOpen && !pdaFound);

        nearTerminal = (std::abs(playerX - 1100.f) < 70.f);
        techTerminal.showHint = (nearTerminal && !dialogueIsOpen && !terminalBypassed);

        nearValve = (std::abs(playerX - 750.f) < 60.f);
        valvePipe.showHint = (nearValve && !dialogueIsOpen && terminalBypassed);

        story.techPdaFound = pdaFound;
        story.techTerminalBypassed = terminalBypassed;
        story.techGasCleared = gasCleared;
    }

    void handleInteraction(Player& hero, StoryManager& story, DialogueSystem& dialogue, DialogueDatabase& dialogueDb) {
        if (nearCorpse && !pdaFound) {
            pdaFound = true;
            hero.inventory.addItem("PDA", 1);
            dialogue.startDialogue(dialogueDb.getDialogue("tech_corpse"));
            hero.showMessage(L"ÏÎËÓ×ÅÍ ÑËÓÆÅÁÍÛÉ ÊÏÊ ÒÅÕÍÈÊÀ", sf::Color::Green);
            return;
        }
        if (nearTerminal && !terminalBypassed) {
            if (pdaFound) {
                isCodeInputActive = true;
            }
            else {
                hero.showMessage(L"ÄËß ÄÎÑÒÓÏÀ ÍÓÆÅÍ ÊÏÊ ÈÍÆÅÍÅĞÀ", sf::Color::Red);
            }
            return;
        }
        if (nearValve && terminalBypassed && !gasCleared) {
            gasCleared = true;
            dialogue.startDialogue(dialogueDb.getDialogue("tech_valve_success"));
            hero.showMessage(L"ÂÅÍÒÈËÜ ÏĞÎÂÅĞÍÓÒ! ÃÀÇ ÎÒÊÀ×ÀÍ!", sf::Color::Green);
            return;
        }
    }

    void draw(sf::RenderWindow& window) {
        if (!isLoaded) return;

        bgSprite.setColor(sf::Color(static_cast<int>(currentRedTint), static_cast<int>(currentGreenTint), static_cast<int>(currentRedTint)));
        window.draw(bgSprite);
        mark.draw(window);

        if (!pdaFound) {
            window.draw(corpseGlow);
        }

        sf::Font font;
        if (!font.loadFromFile("PixeloidSans.ttf")) return;

        if (techCorpse.showHint) {
            sf::Text hint(L"Íàæìèòå E (Îáûñêàòü òåëî)", font, 12);
            hint.setPosition(450.f - 70.f, 220.f);
            window.draw(hint);
        }
        if (techTerminal.showHint) {
            sf::Text hint(L"Íàæìèòå E (Ââåñòè êîä äîñòóïà)", font, 12);
            hint.setPosition(1100.f - 85.f, 220.f);
            window.draw(hint);
        }
        if (valvePipe.showHint) {
            sf::Text hint(gasCleared ? L"Íàæìèòå E (Ñïóñòèòüñÿ â Ìåäáëîê)" : L"Íàæìèòå E (Ïğîâåğíóòü âåíòèëü)", font, 12);
            hint.setPosition(750.f - 75.f, 220.f);
            window.draw(hint);
        }
        if (!gasCleared) {
            sf::Text timerText(L"ÔÈËÜÒĞÀÖÈß ÂÎÇÄÓÕÀ: " + std::to_wstring(static_cast<int>(gasTimer / 1000.f)) + L"ñ", font, 13);
            timerText.setFillColor(sf::Color::Magenta);
            timerText.setPosition(320.f, 55.f);
            window.draw(timerText);
        }

        if (isCodeInputActive) {
            float cameraX = window.getView().getCenter().x;
            codeInputWindow.setPosition(cameraX - 130.f, 160.f);
            window.draw(codeInputWindow);

            sf::Text codeTitle(L"ÂÂÅÄÈÒÅ ÊÎÄ ÈÇ ÊÏÊ:", font, 11);
            codeTitle.setFillColor(sf::Color::Green);
            codeTitle.setPosition(cameraX - 110.f, 175.f);
            window.draw(codeTitle);

            sf::Text codeVal(playerEnteredCode + L"_", font, 14);
            codeVal.setFillColor(sf::Color::Yellow);
            codeVal.setPosition(cameraX - 20.f, 205.f);
            window.draw(codeVal);
        }
    }
};

#endif
