#ifndef ENEMY_H
#define ENEMY_H

#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>

class Enemy {
public:
    sf::Sprite staticSprite;
    sf::Sprite walkSprite;
    sf::Sprite attackSprite;

    sf::Texture staticTex;
    sf::Texture walkTex;
    sf::Texture attackTex;

    bool isWalkLoaded;
    bool isAttackLoaded;
    bool isLoaded;

    float health;
    bool showHint;
    int currentState;

    Enemy() : isWalkLoaded(false), isAttackLoaded(false), isLoaded(false), health(60.f), showHint(false), currentState(0) {}

    void init(std::string staticFile, std::string walkFile, std::string attackFile, sf::Vector2f startPos) {
        if (!staticTex.loadFromFile(staticFile)) {
            std::cout << "Error: " << staticFile << " not found!" << std::endl;
        }
        staticTex.setSmooth(false);

        if (!walkTex.loadFromFile(walkFile)) {
            std::cout << "Warning: " << walkFile << " not found!" << std::endl;
            isWalkLoaded = false;
        }
        else {
            walkTex.setSmooth(false);
            isWalkLoaded = true;
        }

        if (!attackTex.loadFromFile(attackFile)) {
            std::cout << "Warning: " << attackFile << " not found!" << std::endl;
            isAttackLoaded = false;
        }
        else {
            attackTex.setSmooth(false);
            isAttackLoaded = true;
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

        if (isWalkLoaded) {
            walkSprite.setTexture(walkTex);
            walkSprite.setTextureRect(sf::IntRect(0, 0, 150, 234));
            walkSprite.setOrigin(75.f, 234.f);
            float scaleFactorWalk = targetHeight / 234.f;
            walkSprite.setScale(scaleFactorWalk, scaleFactorWalk);
        }
        walkSprite.setPosition(startPos);

        if (isAttackLoaded) {
            attackSprite.setTexture(attackTex);
            attackSprite.setTextureRect(sf::IntRect(0, 0, 150, 234));
            attackSprite.setOrigin(75.f, 234.f);
            float scaleFactorAttack = targetHeight / 234.f;
            attackSprite.setScale(scaleFactorAttack, scaleFactorAttack);
        }
        attackSprite.setPosition(startPos);

        currentState = 0;
        isLoaded = true;
    }

    void setState(int state, int frameIndex = 0) {
        currentState = state;
        if (state == 1 && isWalkLoaded) {
            walkSprite.setTextureRect(sf::IntRect(frameIndex * 150, 0, 150, 234));
        }
        else if (state == 2 && isAttackLoaded) {
            attackSprite.setTextureRect(sf::IntRect(frameIndex * 150, 0, 150, 234));
        }
    }

    void setFacing(bool lookRight) {
        float scaleX = lookRight ? std::abs(staticSprite.getScale().x) : -std::abs(staticSprite.getScale().x);
        staticSprite.setScale(scaleX, staticSprite.getScale().y);

        if (isWalkLoaded) {
            float walkScaleX = lookRight ? std::abs(walkSprite.getScale().x) : -std::abs(walkSprite.getScale().x);
            walkSprite.setScale(walkScaleX, walkSprite.getScale().y);
        }
        if (isAttackLoaded) {
            float attackScaleX = lookRight ? std::abs(attackSprite.getScale().x) : -std::abs(attackSprite.getScale().x);
            attackSprite.setScale(attackScaleX, attackSprite.getScale().y);
        }
    }

    void move(float offsetX, float offsetY) {
        staticSprite.move(offsetX, offsetY);
        walkSprite.move(offsetX, offsetY);
        attackSprite.move(offsetX, offsetY);
    }

    void setPosition(float x, float y) {
        staticSprite.setPosition(x, y);
        walkSprite.setPosition(x, y);
        attackSprite.setPosition(x, y);
    }

    sf::Vector2f getPosition() {
        if (currentState == 1 && isWalkLoaded) return walkSprite.getPosition();
        if (currentState == 2 && isAttackLoaded) return attackSprite.getPosition();
        return staticSprite.getPosition();
    }

    sf::FloatRect getGlobalBounds() {
        if (currentState == 1 && isWalkLoaded) return walkSprite.getGlobalBounds();
        if (currentState == 2 && isAttackLoaded) return attackSprite.getGlobalBounds();
        return staticSprite.getGlobalBounds();
    }

    void draw(sf::RenderWindow& window) {
        if (!isLoaded) return;

        if (health > 0) {
            if (currentState == 1 && isWalkLoaded) {
                window.draw(walkSprite);
            }
            else if (currentState == 2 && isAttackLoaded) {
                window.draw(attackSprite);
            }
            else {
                window.draw(staticSprite);
            }
        }
        else {
            staticSprite.setColor(sf::Color(60, 60, 70, 140));
            window.draw(staticSprite);
        }
    }
};

#endif
