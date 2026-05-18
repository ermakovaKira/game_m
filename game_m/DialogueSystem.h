#ifndef DIALOGUE_SYSTEM_H
#define DIALOGUE_SYSTEM_H

#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

struct DialogueLine {
    std::wstring text;
};

class DialogueSystem {
public:
    sf::RectangleShape box;
    sf::Font font;
    sf::Text uiText;
    sf::Sprite portraitSprite;
    bool isOpen;

    sf::Texture evaFaceTex;
    sf::Texture markFaceTex;
    sf::Texture strangerFaceTex;

    std::vector<DialogueLine> lines;
    int currentLine;

    std::wstring fullText;
    std::wstring displayedText;
    float textTimer;
    float textSpeed;
    size_t charIndex;

    DialogueSystem() {
        isOpen = false;
        currentLine = 0;
        textTimer = 0.f;
        textSpeed = 20.f;
        charIndex = 0;

        box.setSize(sf::Vector2f(630, 100));
        box.setFillColor(sf::Color(20, 20, 20, 240));
        box.setOutlineThickness(1);
        box.setOutlineColor(sf::Color(100, 100, 100));
        box.setPosition(160, 290);

        font.loadFromFile("PixeloidSans.ttf");
        uiText.setFont(font);
        uiText.setCharacterSize(12);
        uiText.setFillColor(sf::Color::White);
        uiText.setPosition(180, 305);

        portraitSprite.setPosition(40, 285);

        evaFaceTex.loadFromFile("icons/eva_face.png");
        markFaceTex.loadFromFile("icons/mark_face.png");
        strangerFaceTex.loadFromFile("icons/stranger_face.png");

        evaFaceTex.setSmooth(false);
        markFaceTex.setSmooth(false);
        strangerFaceTex.setSmooth(false);
    }

    void startDialogue(std::vector<DialogueLine> newLines) {
        if (newLines.empty()) return;
        lines = newLines;
        currentLine = 0;
        isOpen = true;
        applyLine();
    }

    void nextLine() {
        currentLine++;
        if (currentLine < lines.size()) {
            applyLine();
        }
        else {
            isOpen = false;
        }
    }

    bool isPrinting() {
        return charIndex < fullText.length();
    }

    void forceComplete() {
        displayedText = fullText;
        uiText.setString(displayedText);
        charIndex = fullText.length();
    }

    void update(float time) {
        if (!isOpen) return;

        if (charIndex < fullText.length()) {
            textTimer += time;
            if (textTimer >= textSpeed) {
                displayedText += fullText[charIndex];
                uiText.setString(displayedText);
                charIndex++;
                textTimer = 0.f;
            }
        }
    }

    void draw(sf::RenderWindow& window) {
        if (isOpen) {
            window.draw(box);
            window.draw(uiText);
            if (portraitSprite.getTexture() != nullptr) {
                window.draw(portraitSprite);
            }
        }
    }

private:
    void wrapText(sf::Text& text, float maxWidth) {
        std::wstring source = text.getString();
        std::wstring wrapped = L"";
        std::wstring line = L"";

        std::size_t start = 0;
        std::size_t end = source.find(L' ');

        while (end != std::wstring::npos) {
            std::wstring word = source.substr(start, end - start);
            text.setString(line + word + L" ");
            if (text.getGlobalBounds().width > maxWidth) {
                wrapped += line + L"\n";
                line = word + L" ";
            }
            else {
                line += word + L" ";
            }
            start = end + 1;
            end = source.find(L' ', start);
        }

        std::wstring lastWord = source.substr(start);
        text.setString(line + lastWord);
        if (text.getGlobalBounds().width > maxWidth) {
            wrapped += line + L"\n" + lastWord;
        }
        else {
            line += lastWord;
            wrapped += line;
        }
        text.setString(wrapped);
    }

    void applyLine() {
        fullText = lines[currentLine].text;

        if (fullText.find(L"≈¬¿") != std::wstring::npos ||
            fullText.find(L"EVA") != std::wstring::npos ||
            fullText.find(L"«¿œ»— ¿") != std::wstring::npos) {
            portraitSprite.setTexture(evaFaceTex, true);
        }
        else if (fullText.find(L"Ã¿– ") != std::wstring::npos ||
            fullText.find(L"MARK") != std::wstring::npos) {
            portraitSprite.setTexture(markFaceTex, true);
        }
        else if (fullText.find(L"ƒŒ “Œ–") != std::wstring::npos ||
            fullText.find(L"Õ≈«Õ¿ ŒÃ≈÷") != std::wstring::npos ||
            fullText.find(L"CARTER") != std::wstring::npos) {
            portraitSprite.setTexture(strangerFaceTex, true);
        }
        else {
            portraitSprite.setTextureRect(sf::IntRect());
        }

        if (portraitSprite.getTexture() != nullptr) {
            float targetSize = 110.0f;
            portraitSprite.setScale(targetSize / portraitSprite.getLocalBounds().width, targetSize / portraitSprite.getLocalBounds().height);
        }

        sf::Text tempText = uiText;
        tempText.setString(fullText);
        wrapText(tempText, 580.f);
        fullText = tempText.getString();

        displayedText = L"";
        charIndex = 0;
        textTimer = 0.f;
        uiText.setString(displayedText);
    }
};

#endif
