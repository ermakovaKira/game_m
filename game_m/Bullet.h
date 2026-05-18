#ifndef BULLET_H
#define BULLET_H

#include <SFML/Graphics.hpp>

class Bullet {
public:
    sf::Sprite sprite;
    sf::Vector2f direction;
    float speed;


    Bullet(sf::Texture& t, sf::Vector2f pos, bool faceRight) {
        speed = 0.8f;
        sprite.setTexture(t);
        sprite.setPosition(pos);


        sprite.setScale(0.05f, 0.05f);

        if (faceRight) {
            direction = { 1.f, 0.f };
        }
        else {
            direction = { -1.f, 0.f };
            sprite.setScale(-0.05f, 0.05f); 
        }
    }

    void update(float time) {
        sprite.move(direction.x * speed * time, 0);
    }

    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }
};

#endif

