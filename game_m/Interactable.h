#ifndef INTERACTABLE_H
#define INTERACTABLE_H

#include <SFML/Graphics.hpp>
#include <string>

class Interactable {
public:
    sf::Sprite sprite;
    sf::Texture texture;
    sf::Text hintText;
    sf::Font font;
    bool showHint;
    std::string id;

    Interactable(std::string texturePath, sf::Vector2f pos, std::string objectID, float width, float height) {
        id = objectID;
        showHint = false;

        if (texture.loadFromFile(texturePath)) {
            texture.setSmooth(false);
            sprite.setTexture(texture);
            sprite.setScale(width / sprite.getLocalBounds().width, height / sprite.getLocalBounds().height);
            sprite.setPosition(pos);
        }

        if (font.loadFromFile("PixeloidSans.ttf")) {
            hintText.setFont(font);
            hintText.setString(L"ֽאזלטעו E");
            hintText.setCharacterSize(12);
            hintText.setFillColor(sf::Color::Yellow);
            hintText.setOrigin(hintText.getLocalBounds().width / 2, 0);
        }
    }

    void draw(sf::RenderWindow& window) {
        if (showHint) {
            float centerX = sprite.getPosition().x + (sprite.getGlobalBounds().width / 2.0f);
            hintText.setPosition(centerX, sprite.getPosition().y - 20);
            window.draw(hintText);
        }
    }
};

#endif

