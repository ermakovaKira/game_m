#include "Player.h"
#include "ItemDatabase.h"
#include <iostream>

Player::Player(std::string pathIdle, std::string pathShoot, int width, int height) {
    textureIdle.loadFromFile(pathIdle);
    textureShoot.loadFromFile(pathShoot);


    textureIdle.setSmooth(false);
    textureShoot.setSmooth(false);

    w = width;
    h = height;

    currentFrame = 0.f;
    speed = 0.10f; 
    isShooting = false;
    faceRight = true;
    db = nullptr;

    sprite.setTexture(textureIdle);
    sprite.setTextureRect(sf::IntRect(0, 0, w - 1, h));

    float targetH = 180.0f;
    float currentScale = targetH / (float)h;
    sprite.setScale(currentScale, currentScale);
    messageTimer = 0.f;
    if (messageFont.loadFromFile("PixeloidSans.ttf")) {
        messageText.setFont(messageFont);
        messageText.setCharacterSize(16); 
        messageText.setOutlineThickness(1.5f);
        messageText.setOutlineColor(sf::Color::Black);
    }
    isFlashActive = false;
    flashTimer = 0.f;
}



void Player::handleInput(sf::Event& event) {

    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Num1) {
        if (inventory.items["Medkit"] > 0) {
            if (stats.health >= 100.f) {
                showMessage(L"Здоровье уже на максимуме!", sf::Color::Red);
                return;
            }
            inventory.items["Medkit"]--;
            float healAmount = 30.f;
            if (db != nullptr) {
                healAmount = db->data["Medkit"].effectValue;
            }
            stats.health = std::min(100.f, stats.health + healAmount);
            health = stats.health;
            std::cout << "Ева использовала аптечку. HP: " << stats.health << std::endl;
        }
    }


    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space && !isShooting) {
        if (inventory.items["Ammo"] > 0) {
            inventory.items["Ammo"]--;
            isShooting = true;
            currentFrame = 0.f;
            sprite.setTexture(textureShoot);
            animationClock.restart();
            std::cout << "Выстрел! Патронов осталось: " << inventory.items["Ammo"] << std::endl;
        }
        else {
            showMessage(L"НЕТ ПАТРОНОВ!", sf::Color::Red);
        }
    }
}



void Player::update(float time) {

    if (isFlashActive) {
        speed = 0.15f; 
        flashTimer -= time;
        if (flashTimer <= 0.f) {
            isFlashActive = false;
            speed = 0.10f;
            showMessage(L"Действие бодрящего напитка закончилось.", sf::Color::White);
        }
    }
    else {
        speed = 0.10f; 
    }


    if (!isShooting) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            sprite.move(-speed * time, 0.f);
            faceRight = false;


            if (animationClock.getElapsedTime().asMilliseconds() > 110) {
                currentFrame += 1.f;
                if (currentFrame >= 4.f) currentFrame = 0.f;
                animationClock.restart();
            }
            sprite.setTexture(textureIdle);
            sprite.setTextureRect(sf::IntRect(int(currentFrame) * w + w, 0, -w, h));
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            sprite.move(speed * time, 0.f);
            faceRight = true; 

            if (animationClock.getElapsedTime().asMilliseconds() > 110) {
                currentFrame += 1.f;
                if (currentFrame >= 4.f) currentFrame = 0.f;
                animationClock.restart();
            }

            sprite.setTexture(textureIdle);
            sprite.setTextureRect(sf::IntRect(int(currentFrame) * w, 0, w, h));
        }
        else {

            sprite.setTexture(textureIdle);
            if (faceRight) sprite.setTextureRect(sf::IntRect(0, 0, w, h));
            else sprite.setTextureRect(sf::IntRect(w, 0, -w, h));
        }
    }


    if (isShooting) {
        sprite.setTexture(textureShoot);

        if (animationClock.getElapsedTime().asMilliseconds() > 70) {
            currentFrame += 1.f;
            animationClock.restart();
        }


        if (currentFrame < 4.f) {
            int frameOffset = int(currentFrame) * w;
            if (faceRight) sprite.setTextureRect(sf::IntRect(frameOffset, 0, w, h));
            else sprite.setTextureRect(sf::IntRect(frameOffset + w, 0, -w, h));
        }
        else {
  
            isShooting = false;
            currentFrame = 0.f;
        }
    }


    if (messageTimer > 0.f) {
        messageTimer -= 0.5f * time;
    }

 
    sprite.setPosition(sprite.getPosition().x, 210.f);


    if (sprite.getPosition().x < 0.f) sprite.setPosition(0.f, 210.f);
    if (sprite.getPosition().x > 1530.f) sprite.setPosition(1530.f, 210.f);
}


void Player::draw(sf::RenderWindow& window) {
    window.draw(sprite);
    drawMessage(window);
}

void Player::showMessage(std::wstring message, sf::Color color) {
    messageText.setString(message);
    messageText.setFillColor(color);
    messageTimer = 250.f;
}

void Player::drawMessage(sf::RenderWindow& window) {
    if (messageTimer > 0.f) {

        float centerX = sprite.getPosition().x + (w * sprite.getScale().x) / 2.f;
        float topY = sprite.getPosition().y - 40.f;

        messageText.setOrigin(messageText.getLocalBounds().width / 2.f, 0);
        messageText.setPosition(centerX, topY);

        window.draw(messageText);
    }
}