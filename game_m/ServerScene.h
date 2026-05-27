#ifndef SERVER_SCENE_H
#define SERVER_SCENE_H

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

class ServerScene {
public:
    sf::Texture bgTex;
    sf::Sprite bgSprite;
    bool isLoaded;

    NPC mark;
    Enemy finalBoss;

    Interactable mainframeTerminal;

    bool nearTerminal;
    bool bossSpawned;
    bool bossDefeated;
    bool dataDownloaded;
    bool isEndingSelectionActive;
    int selectedEnding;

public:
    ServerScene()
        : mainframeTerminal("laptop_sprite.png", sf::Vector2f(800.f, 210.f), "mainframe", 90.f, 80.f)
    {
        isLoaded = false;
        nearTerminal = bossSpawned = bossDefeated = dataDownloaded = isEndingSelectionActive = false;
        selectedEnding = 0;
    }

    void init() {
        if (isLoaded) return;
        if (!bgTex.loadFromFile("lab_core.png")) {
            std::cout << "CRITICAL ERROR: lab_core.png not found!" << std::endl;
        }
        bgTex.setSmooth(false);
        bgSprite.setTexture(bgTex, true);
        bgSprite.setScale(1600.f / bgTex.getSize().x, 400.f / bgTex.getSize().y);

        mark.init("npc_sprite.png", "mark_move.png", sf::Vector2f(150.f, 385.f));

        finalBoss.init("zombie_static.png", "zombie_walk.png", "zombie_attack.png", sf::Vector2f(240.f, 385.f));
        finalBoss.health = 300.f;

        finalBoss.staticSprite.setColor(sf::Color(180, 30, 255));
        finalBoss.walkSprite.setColor(sf::Color(180, 30, 255));
        finalBoss.attackSprite.setColor(sf::Color(180, 30, 255));

        isLoaded = true;
    }

    void update(float time, Player& hero, StoryManager& story, DialogueSystem& dialogue, DialogueDatabase& dialogueDb) {
        if (!isLoaded) return;

        float playerX = hero.sprite.getPosition().x;

        if (!bossSpawned && playerX >= 500.f && !dialogue.isOpen) {
            bossSpawned = true;
            story.serverBossSpawned = true;
            dialogue.startDialogue(dialogueDb.getDialogue("lab_stranger_meet"));
        }

        if (bossSpawned && !bossDefeated && !dialogue.isOpen) {
            if (finalBoss.health <= 0) {
                bossDefeated = true;
                story.serverBossDefeated = true;
                dialogue.startDialogue(dialogueDb.getDialogue("boss_defeat_text"));
                return;
            }

            float bX = finalBoss.getPosition().x;
            bool playerMoving = sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::D);
            float bossSpeed = playerMoving ? 0.07f : 0.02f;

            if (finalBoss.hitCooldown <= 0.f) {
                finalBoss.setState(1);
            }

            if (bX < playerX - 10.f) finalBoss.move(bossSpeed * time, 0.f);
            else if (bX > playerX + 10.f) finalBoss.move(-bossSpeed * time, 0.f);
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
                if (hero.faceRight) {
                    mark.staticSprite.setScale(-std::abs(mark.staticSprite.getScale().x), mark.staticSprite.getScale().y);
                }
                else {
                    mark.staticSprite.setScale(std::abs(mark.staticSprite.getScale().x), mark.staticSprite.getScale().y);
                }
            }
        }

        if (dataDownloaded && !dialogue.isOpen && selectedEnding == 0) {
            isEndingSelectionActive = true;
            story.serverDataDownloaded = true;
        }

        nearTerminal = (std::abs(playerX - 800.f) < 90.f);
        if (finalBoss.hitCooldown <= 0.f) {
            finalBoss.setState(1, 0);
        }

    }


    void draw(sf::RenderWindow& window) {
        if (!isLoaded) return;
        window.draw(bgSprite);
        mark.draw(window);

        if (bossSpawned && !bossDefeated) {
            finalBoss.draw(window);
        }

        sf::Font font;
        if (!font.loadFromFile("PixeloidSans.ttf")) return;

        if (mainframeTerminal.showHint && !isEndingSelectionActive) {
            sf::Text hint(L"Нажмите E (Подключить ноутбук к Главному Терминалу)", font, 11);
            hint.setPosition(800.f - 120.f, 220.f);
            window.draw(hint);
        }

        if (isEndingSelectionActive) {
            sf::RectangleShape choiceBg(sf::Vector2f(440.f, 110.f));
            choiceBg.setFillColor(sf::Color(10, 10, 15, 240));
            choiceBg.setOutlineColor(sf::Color::Yellow);
            choiceBg.setOutlineThickness(2.f);
            choiceBg.setPosition(window.getView().getCenter().x - 220.f, 130.f);
            window.draw(choiceBg);

            sf::Text title(L"БЕСКОМПРОМИССНЫЙ ВЫБОР:", font, 12);
            title.setFillColor(sf::Color::Yellow);
            title.setPosition(choiceBg.getPosition().x + 20.f, 140.f);
            window.draw(title);

            sf::Text opt1(L"[Нажмите 1] - Сбежать через коллектор (Спасти жизнь)", font, 11);
            opt1.setFillColor(sf::Color::Green);
            opt1.setPosition(choiceBg.getPosition().x + 20.f, 170.f);
            window.draw(opt1);

            sf::Text opt2(L"[Нажмите 2] - Транслировать архивы (Смерть ради правды)", font, 11);
            opt2.setFillColor(sf::Color::Red);
            opt2.setPosition(choiceBg.getPosition().x + 20.f, 195.f);
            window.draw(opt2);
        }
    }
};

#endif
