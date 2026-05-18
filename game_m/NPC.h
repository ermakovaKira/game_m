#ifndef NPC_H
#define NPC_H

#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>

class NPC {
public:
    sf::Sprite staticSprite;
    sf::Sprite moveSprite;

    sf::Texture staticTex;
    sf::Texture moveTex;

    bool isMoveTexLoaded;
    bool showHint;
    float health;
    bool isMovingNow;

    NPC() : isMoveTexLoaded(false), showHint(false), health(100.f), isMovingNow(false) {}

    void init(std::string staticFile, std::string moveFile, sf::Vector2f startPos) {
        if (!staticTex.loadFromFile(staticFile)) {
            std::cout << "Error: " << staticFile << " not found!" << std::endl;
        }
        staticTex.setSmooth(false);

        if (!moveTex.loadFromFile(moveFile)) {
            std::cout << "Error: " << moveFile << " not found!" << std::endl;
            isMoveTexLoaded = false;
        }
        else {
            moveTex.setSmooth(false);
            isMoveTexLoaded = true;
        }

        float targetHeight = 189.f;

        staticSprite.setTexture(staticTex);
        int staticW = static_cast<int>(staticTex.getSize().x);
        int staticH = static_cast<int>(staticTex.getSize().y);
        staticSprite.setTextureRect(sf::IntRect(0, 0, staticW, staticH));

        staticSprite.setOrigin(static_cast<float>(staticW) / 2.f, static_cast<float>(staticH));

        float scaleFactorStatic = targetHeight / staticSprite.getLocalBounds().height;
        staticSprite.setScale(scaleFactorStatic, scaleFactorStatic);
        staticSprite.setPosition(startPos);

        if (isMoveTexLoaded) {
            moveSprite.setTexture(moveTex);
            moveSprite.setTextureRect(sf::IntRect(0, 0, 150, 234));
            moveSprite.setOrigin(75.f, 234.f);
            float scaleFactorMove = targetHeight / 234.f;
            moveSprite.setScale(scaleFactorMove, scaleFactorMove);
        }
        moveSprite.setPosition(startPos);

        isMovingNow = false;
    }

    void setState(bool isMoving, int frameIndex = 0) {
        isMovingNow = isMoving;
        if (isMoving && isMoveTexLoaded) {
            moveSprite.setTextureRect(sf::IntRect(frameIndex * 150, 0, 150, 234));
        }
    }

    void setPosition(float x, float y) {
        staticSprite.setPosition(x, y);
        moveSprite.setPosition(x, y);
    }

    sf::Vector2f getPosition() {
        return isMovingNow ? moveSprite.getPosition() : staticSprite.getPosition();
    }

    void move(float offsetX, float offsetY) {
        staticSprite.move(offsetX, offsetY);
        moveSprite.move(offsetX, offsetY);
    }

    void draw(sf::RenderWindow& window) {
        if (isMovingNow && isMoveTexLoaded) {
            window.draw(moveSprite);
        }
        else {
            window.draw(staticSprite);
        }
    }
};

#endif
