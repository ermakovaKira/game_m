#ifndef ENEMY_H
#define ENEMY_H

#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <cmath>
#include <string>

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
    float hitCooldown; 
    bool showHint;

    int currentState;

    Enemy() : isWalkLoaded(false), isAttackLoaded(false), isLoaded(false), health(60.f), hitCooldown(0.f), showHint(false), currentState(0) {}

    void init(std::string textureStaticFile, std::string textureWalkFile, std::string textureAttackFile, sf::Vector2f startPos) {
        if (!staticTex.loadFromFile(textureStaticFile)) {
            std::cout << "Warning: " << textureStaticFile << " not found. Using " << textureWalkFile << " as fallback for static state." << std::endl;
            if (!staticTex.loadFromFile(textureWalkFile)) {
                std::cout << "Error: Failed to load fallback texture!" << std::endl;
            }
        }

        if (walkTex.loadFromFile(textureWalkFile)) {
            isWalkLoaded = true;
        }
        else {
            std::cout << "Error: " << textureWalkFile << " not found!" << std::endl;
        }

        if (attackTex.loadFromFile(textureAttackFile)) {
            isAttackLoaded = true;
        }
        else {
            std::cout << "Error: " << textureAttackFile << " not found!" << std::endl;
        }

        staticTex.setSmooth(false);
        walkTex.setSmooth(false);
        attackTex.setSmooth(false);

        float targetHeight = 189.f;
        staticSprite.setTexture(staticTex);

        int staticW = static_cast<int>(staticTex.getSize().x);
        int staticH = static_cast<int>(staticTex.getSize().y);

        if (staticW > 200) {
            staticSprite.setTextureRect(sf::IntRect(0, 0, 150, 234));
            staticSprite.setOrigin(75.f, 234.f);
        }
        else {
            staticSprite.setTextureRect(sf::IntRect(0, 0, staticW, staticH));
            staticSprite.setOrigin(static_cast<float>(staticW) / 2.f, static_cast<float>(staticH));
        }

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
        if (hitCooldown > 0.f) hitCooldown -= 0.4f; 

        if (offsetX != 0.f || offsetY != 0.f) {
            static float animFrame = 0.f;
            animFrame += 0.05f;
            if (animFrame >= 4.f) animFrame = 0.f;
            setState(1, static_cast<int>(animFrame));
            setFacing(offsetX > 0);
        }
        else {
            setState(0, 0);
        }

        staticSprite.move(offsetX, offsetY);
        walkSprite.move(offsetX, offsetY);
        attackSprite.move(offsetX, offsetY);
    }

    void checkPlayerCollision(Player& hero, float baseDamage, float difficultyModifier, bool lookOpenDialogue, sf::Sound& hitSound) {
        if (health > 0 && !lookOpenDialogue) {

            if (hitCooldown > 0.f) {
                hitCooldown -= 1.0f;
                setState(2, 0);
            }
            else {
                hitCooldown = 0.f;
            }

            if (hero.sprite.getGlobalBounds().intersects(getGlobalBounds())) {

                if (hitCooldown <= 0.f) {
                    setState(2, 0);

                    float cleanDamage = 10.f;
                    if (difficultyModifier < 0.8f) cleanDamage = 5.f;
                    else if (difficultyModifier > 1.2f) cleanDamage = 15.f;

                    hero.stats.health -= cleanDamage;
                    hero.health = hero.stats.health;

                    hitSound.play();

                    hero.showMessage(L"ÂAÑ ÓÄAÐÈËÈ!", sf::Color::Red);
                    hitCooldown = 45.f;
                }

                float currentX = getPosition().x;
                float currentY = getPosition().y;
                if (currentX > hero.sprite.getPosition().x) {
                    setPosition(currentX + 110.f, currentY);
                }
                else {
                    setPosition(currentX - 110.f, currentY);
                }
            }
        }
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
