#include "HealthBar.h"

HealthBar::HealthBar(float width, float height, sf::Color color) {
    maxWidth = width;

    back.setSize({ width, height });
    back.setFillColor(sf::Color(50, 50, 50, 200)); 

    front.setSize({ width, height });
    front.setFillColor(color);
}

void HealthBar::update(float currentHp, float maxHp, sf::Vector2f position) {

    back.setPosition(position.x, position.y - 20);
    front.setPosition(position.x, position.y - 20);


    float percentage = currentHp / maxHp;
    if (percentage < 0) percentage = 0;
    front.setSize({ maxWidth * percentage, front.getSize().y });
}

void HealthBar::draw(sf::RenderWindow& window) {
    window.draw(back);
    window.draw(front);
}
