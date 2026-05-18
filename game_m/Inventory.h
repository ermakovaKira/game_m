#ifndef INVENTORY_H
#define INVENTORY_H

#pragma once
#include <SFML/Graphics.hpp>
#include <map>
#include <string>
#include <iostream>
#include <vector>

class Inventory {
public:
    std::map<std::string, int> items;
    std::map<std::string, sf::Texture> textures;

    sf::RectangleShape uiBox;
    sf::RectangleShape selectorRect;
    int selectedSlot;
    std::vector<std::string> displayOrder;

    // Пересборка логики под прозрачные .png иконки
public:
    Inventory() {
        uiBox.setSize(sf::Vector2f(500.f, 200.f));
        uiBox.setFillColor(sf::Color(10, 10, 10, 230));
        uiBox.setOutlineThickness(2.f);
        uiBox.setOutlineColor(sf::Color(150, 150, 150));
        uiBox.setPosition(150.f, 100.f);

        selectorRect.setSize(sf::Vector2f(54.f, 54.f));
        selectorRect.setFillColor(sf::Color::Transparent);
        selectorRect.setOutlineThickness(2.f);
        selectorRect.setOutlineColor(sf::Color::Yellow);

        items["Laptop"] = 0;
        items["Note"] = 0;
        items["Note2"] = 0;
        items["PDA"] = 0;
        items["Ammo"] = 0;
        items["Medkit"] = 0;
        items["Keys"] = 0;

        displayOrder = { "Laptop", "Note", "Note2", "PDA", "Ammo", "Medkit" };
        selectedSlot = 0;
    }

    void loadItemTexture(std::string name, std::string filename) {
        sf::Texture tex;
        if (tex.loadFromFile(filename)) {
            tex.setSmooth(false);
            textures[name] = tex;
        }
        else {
            std::cout << "Inventory Error: Failed to load " << filename << std::endl;
        }
    }

    void handleScroll(float delta) {
        if (delta > 0) {
            selectedSlot--;
            if (selectedSlot < 0) selectedSlot = 5;
        }
        else if (delta < 0) {
            selectedSlot++;
            if (selectedSlot > 5) selectedSlot = 0;
        }
    }

    std::string getSelectedItemName() {
        if (selectedSlot >= 0 && selectedSlot < 6) {
            return displayOrder[selectedSlot];
        }
        return "";
    }

    void addItem(std::string name, int count) {
        items[name] += count;
    }

    void removeItem(std::string name, int count) {
        items[name] -= count;
        if (items[name] < 0) items[name] = 0;
    }

    void drawUI(sf::RenderWindow& window, sf::Font& font) {
        window.draw(uiBox);

        sf::Text title(L"ИНВЕНТАРЬ ЕВЫ", font, 14);
        title.setFillColor(sf::Color::Yellow);
        title.setPosition(330.f, 115.f);
        window.draw(title);

        float slotX = 175.f;
        float slotY = 160.f;
        float spacing = 75.f;

        for (int i = 0; i < 6; i++) {
            std::string itemName = displayOrder[i];

            sf::RectangleShape slotRect(sf::Vector2f(50.f, 50.f));
            slotRect.setFillColor(sf::Color(30, 30, 30));
            slotRect.setOutlineThickness(1.f);
            slotRect.setOutlineColor(sf::Color(100, 100, 100));
            slotRect.setPosition(slotX + (i * spacing), slotY);
            window.draw(slotRect);

            if (i == selectedSlot) {
                selectorRect.setPosition(slotX + (i * spacing) - 2.f, slotY - 2.f);
                window.draw(selectorRect);
            }

            if (items[itemName] > 0) {
                if (textures.find(itemName) != textures.end()) {
                    sf::Sprite itemSprite;
                    itemSprite.setTexture(textures[itemName]);

                    float sX = 40.f / itemSprite.getLocalBounds().width;
                    float sY = 40.f / itemSprite.getLocalBounds().height;
                    itemSprite.setScale(sX, sY);
                    itemSprite.setPosition(slotX + (i * spacing) + 5.f, slotY + 5.f);
                    itemSprite.setColor(sf::Color::White);
                    window.draw(itemSprite);
                }

                sf::Text countText(std::to_string(items[itemName]), font, 11);
                countText.setFillColor(sf::Color::White);
                countText.setPosition(slotX + (i * spacing) + 35.f, slotY + 35.f);
                window.draw(countText);
            }
        }
    }
};

#endif
