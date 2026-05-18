#ifndef APARTMENT_SCENE_H
#define APARTMENT_SCENE_H

#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include "NPC.h"
#include "Interactable.h"
#include "Player.h"
#include "StoryManager.h"
#include "DialogueSystem.h"
#include "DialogueDatabase.h"

class ApartmentScene {
public:
    sf::Texture roomTex;
    sf::Sprite roomSprite;

    NPC mark;

    Interactable pcStation;
    Interactable familyPhoto;
    Interactable kitchenTable;
    Interactable clothesCloset;

    sf::CircleShape noteGlow;
    bool nearMark, nearPc, nearPhoto, nearTable, nearCloset;

public:
    ApartmentScene()
        : pcStation("laptop_sprite.png", sf::Vector2f(1460.f, 245.f), "pc", 60.f, 30.f),
        familyPhoto("laptop_sprite.png", sf::Vector2f(1380.f, 260.f), "photo", 15.f, 20.f),
        kitchenTable("laptop_sprite.png", sf::Vector2f(740.f, 255.f), "table", 80.f, 30.f),
        clothesCloset("laptop_sprite.png", sf::Vector2f(90.f, 200.f), "closet", 60.f, 90.f)
    {
        if (!roomTex.loadFromFile("room.png")) {
            std::cout << "Error: room.png not found!" << std::endl;
        }
        roomTex.setSmooth(false);
        roomSprite.setTexture(roomTex);
        roomSprite.setScale(1584.f / roomSprite.getLocalBounds().width, 400.f / roomSprite.getLocalBounds().height);

        mark.init("npc_sprite.png", "mark_move.png", sf::Vector2f(1050.f, 385.f));

        noteGlow.setRadius(3.f);
        noteGlow.setFillColor(sf::Color(255, 255, 180, 240));
        noteGlow.setOutlineColor(sf::Color::Black);
        noteGlow.setOutlineThickness(1.f);
        noteGlow.setPosition(740.f, 255.f);

        nearMark = nearPc = nearPhoto = nearTable = nearCloset = false;
    }

    void update(float time, Player& hero, StoryManager& story) {
        if (story.markMovingToExit) {
            float targetExitX = 180.f;
            float currentMarkX = mark.getPosition().x;

            if (currentMarkX > targetExitX) {
                mark.move(-0.08f * time, 0.f);

                static float markFrame = 0.f;
                markFrame += 0.0025f * time;
                if (markFrame >= 4.f) markFrame = 0.f;

                mark.setState(true, static_cast<int>(markFrame));
            }
            else {
                mark.setState(false);
                mark.setPosition(targetExitX, 385.f);
            }
        }

        updateDistances(hero.sprite.getPosition().x, false, story, hero.inventory.items["Ammo"]);
    }

    void updateDistances(float playerX, bool dialogueIsOpen, StoryManager& story, int currentAmmo) {
        nearMark = (std::abs(playerX - mark.getPosition().x) < 60.f);
        mark.showHint = (nearMark && !dialogueIsOpen);

        nearPc = (std::abs(playerX - 1460.f) < 60.f);
        pcStation.showHint = (nearPc && !dialogueIsOpen && story.noteRead && !story.readLaptopEmail);

        nearPhoto = (std::abs(playerX - 1380.f) < 40.f);
        familyPhoto.showHint = (nearPhoto && !dialogueIsOpen);

        nearTable = (std::abs(playerX - 740.f) < 60.f);
        kitchenTable.showHint = (nearTable && !dialogueIsOpen && !story.noteRead);

        nearCloset = (std::abs(playerX - 90.f) < 60.f);
        clothesCloset.showHint = (nearCloset && !dialogueIsOpen && story.talkedToMarkStart && currentAmmo == 0);
    }

    void handleInteraction(Player& hero, StoryManager& story, DialogueSystem& dialogue, DialogueDatabase& dialogueDb) {
        if (nearTable && !story.noteRead) {
            story.noteRead = true;
            hero.inventory.addItem("Note", 1);
            dialogue.startDialogue(dialogueDb.getDialogue("apartment_note"));
            return;
        }

        if (nearPc) {
            if (!story.noteRead) {
                dialogue.startDialogue(dialogueDb.getDialogue("inspect_pc_blocked"));
            }
            else if (!story.readLaptopEmail) {
                dialogue.startDialogue(dialogueDb.getDialogue("apartment_pc"));
                story.readLaptopEmail = true;
            }
            return;
        }

        if (nearMark) {
            if (!story.noteRead) {
                dialogue.startDialogue(dialogueDb.getDialogue("inspect_pc_blocked"));
            }
            else if (!story.readLaptopEmail) {
                dialogue.startDialogue(dialogueDb.getDialogue("apartment_mark_before_pc"));
            }
            else {
                dialogue.startDialogue(dialogueDb.getDialogue("apartment_mark_after_pc"));
                if (!story.talkedToMarkStart) {
                    story.talkedToMarkStart = true;
                    hero.inventory.addItem("Medkit", 1);
                }
            }
            return;
        }

        if (nearPhoto) {
            if (!story.readLaptopEmail) {
                dialogue.startDialogue(dialogueDb.getDialogue("inspect_pc_blocked"));
            }
            else {
                dialogue.startDialogue(dialogueDb.getDialogue("inspect_photo"));
            }
            return;
        }

        if (nearCloset && story.talkedToMarkStart) {
            if (hero.inventory.items["Ammo"] == 0) {
                dialogue.startDialogue(dialogueDb.getDialogue("inspect_drawer_open"));
                hero.inventory.addItem("Ammo", 15);
            }
            return;
        }
    }

    void draw(sf::RenderWindow& window) {
        window.draw(roomSprite);
        pcStation.draw(window);
        familyPhoto.draw(window);
        clothesCloset.draw(window);
        mark.draw(window);

        sf::Font font;
        if (!font.loadFromFile("PixeloidSans.ttf")) return;

        if (mark.showHint) {
            sf::Text hint(L"Нажмите E (Марк)", font, 12);
            hint.setFillColor(sf::Color::Yellow);
            hint.setOutlineColor(sf::Color::Black);
            hint.setOutlineThickness(1.f);
            hint.setPosition(mark.getPosition().x - 40.f, mark.getPosition().y - 215.f);
            window.draw(hint);
        }

        if (kitchenTable.showHint) {
            window.draw(noteGlow);
            sf::Text hint(L"Нажмите E (Прочесть записку)", font, 12);
            hint.setFillColor(sf::Color::Yellow);
            hint.setOutlineColor(sf::Color::Black);
            hint.setOutlineThickness(1.f);
            hint.setPosition(740.f - 85.f, 225.f);
            window.draw(hint);
        }

        if (pcStation.showHint) {
            sf::Text hint(L"Нажмите E (Компьютер)", font, 12);
            hint.setFillColor(sf::Color::Yellow);
            hint.setOutlineColor(sf::Color::Black);
            hint.setOutlineThickness(1.f);
            hint.setPosition(1460.f - 60.f, 215.f);
            window.draw(hint);
        }

        if (familyPhoto.showHint) {
            sf::Text hint(L"Нажмите E (Осмотреть фото)", font, 12);
            hint.setFillColor(sf::Color::Yellow);
            hint.setOutlineColor(sf::Color::Black);
            hint.setOutlineThickness(1.f);
            hint.setPosition(1380.f - 75.f, 230.f);
            window.draw(hint);
        }

        if (clothesCloset.showHint) {
            sf::Text hint(L"Нажмите E (Открыть шкаф)", font, 12);
            hint.setFillColor(sf::Color::Yellow);
            hint.setOutlineColor(sf::Color::Black);
            hint.setOutlineThickness(1.f);
            hint.setPosition(90.f - 60.f, 170.f);
            window.draw(hint);
        }
    }
};

#endif
