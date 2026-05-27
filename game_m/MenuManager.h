#ifndef MENU_MANAGER_H
#define MENU_MANAGER_H
#pragma once
#include <SFML/Graphics.hpp>
#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <cstring>
#include <algorithm>
#include "Config.h"

struct UserData {
    char username[32];
    char password[32];
    int lastScene;
    float playerHealth;
    int ammoCount;
    int medkitCount;
    int keysCount;
    int laptopCount;
    int noteCount;
    int note2Count;
    int pdaCount;
    int currentTheme;
    int difficultySetting;
    float musicVolume;
    float soundVolume;
    bool achievements[8];
};

struct LeaderboardRow {
    std::wstring name;
    int totalAchievements;
};

class MenuManager {
public:
    enum MenuState {
        AUTH_SCREEN, MAIN_MENU, ACCOUNT_CABINET,
        THEME_CHOICE, ACHIEVEMENTS_SCREEN, DIFFICULTY_CHOICE,
        CHANGE_NAME_SCREEN, CHANGE_PASS_SCREEN, GAME_ACTIVE
    };
    void unlockAchievement(int index) {
        if (!isUserLoggedIn || index < 0 || index >= 8) return;
        if (!currentUser.achievements[index]) {
            currentUser.achievements[index] = true;
            saveUserToBinary();
        }
    }

private:
    MenuState currentState;
    std::wstring inputUsername;
    std::wstring inputPassword;
    std::wstring editBuffer;
    bool isUsernameInputActive;
    bool isPasswordInputActive;
    UserData currentUser;
    bool isUserLoggedIn;
    int activeTheme;
    sf::Font menuFont;
    std::vector<LeaderboardRow> leaderboard;

    void rebuildLeaderboard();
    void drawButton(sf::RenderWindow& window, sf::Vector2f pos, const std::wstring& text, bool isHovered, bool isDisabled = false);

public:
    MenuManager();
    MenuState getState() const;
    void setState(MenuState state);
    bool isLoggedIn() const;
    int getTheme() const;
    UserData& getCurrentUser();
    bool loadUserFromFile(const std::wstring& username, const std::wstring& password);
    void saveUserToBinary(std::wstring oldName = L"");
    void handleTextEvent(sf::Event event);
    void handleMouseClick(sf::Vector2f mPos);
    void draw(sf::RenderWindow& window);
};
#endif
