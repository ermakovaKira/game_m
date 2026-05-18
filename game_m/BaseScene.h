#ifndef BASE_SCENE_H
#define BASE_SCENE_H
#pragma once

#include <SFML/Graphics.hpp>
#include "Player.h"
#include "StoryManager.h"
#include "DialogueSystem.h"
#include "DialogueDatabase.h"

class BaseScene {
public:
    virtual ~BaseScene() {}
    virtual void init() = 0;
    virtual void update(float time, Player& hero, StoryManager& story, DialogueSystem& dialogue, DialogueDatabase& dialogueDb) = 0;
    virtual void handleInteraction(Player& hero, StoryManager& story, DialogueSystem& dialogue, DialogueDatabase& dialogueDb) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;
};

#endif

