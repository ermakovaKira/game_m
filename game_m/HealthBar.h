#ifndef HEALTHBAR_H
#define HEALTHBAR_H

#include <SFML/Graphics.hpp>

class HealthBar {
private:
    sf::RectangleShape back;
    sf::RectangleShape front;
    float maxWidth;

public:
    HealthBar(float width, float height, sf::Color color);


    void update(float currentHp, float maxHp, sf::Vector2f position);

    // Отрисовка
    void draw(sf::RenderWindow& window);
};

#endif
