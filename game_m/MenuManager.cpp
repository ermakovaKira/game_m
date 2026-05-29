#define _CRT_SECURE_NO_WARNINGS
#include "MenuManager.h"

MenuManager::MenuManager() {
    currentState = AUTH_SCREEN;
    isUsernameInputActive = true;
    isPasswordInputActive = false;
    isUserLoggedIn = false;
    activeTheme = 0;
    std::memset(&currentUser, 0, sizeof(UserData));
    currentUser.musicVolume = 100.f;
    currentUser.soundVolume = 100.f;
    if (!menuFont.loadFromFile("PixeloidSans.ttf")) {
        std::cout << "CRITICAL ERROR: PixeloidSans.ttf not found in menu!" << std::endl;
    }
    if (!backgroundTex.loadFromFile("menu_bg.png")) {
        std::cout << "Warning: menu_bg.png not found!" << std::endl;
    }
    else {
        backgroundSprite.setTexture(backgroundTex);
        backgroundSprite.setScale(800.f / backgroundTex.getSize().x, 400.f / backgroundTex.getSize().y);
    }

}

MenuManager::MenuState MenuManager::getState() const {
    return currentState;
}

void MenuManager::setState(MenuState state) {
    currentState = state;
    if (state == ACHIEVEMENTS_SCREEN) rebuildLeaderboard();
}

bool MenuManager::isLoggedIn() const {
    return isUserLoggedIn;
}

int MenuManager::getTheme() const {
    return activeTheme;
}

UserData& MenuManager::getCurrentUser() {
    return currentUser;
}

void MenuManager::rebuildLeaderboard() {
    leaderboard.clear();
    std::ifstream file("users_database.bin", std::ios::binary);
    if (!file.is_open()) return;
    UserData temp;
    while (file.read(reinterpret_cast<char*>(&temp), sizeof(UserData))) {
        int count = 0;
        for (int i = 0; i < 8; i++) {
            if (temp.achievements[i]) count++;
        }
        std::wstring uName((wchar_t*)temp.username);
        leaderboard.push_back({ uName, count });
    }
    file.close();
    std::sort(leaderboard.begin(), leaderboard.end(), [](const LeaderboardRow& a, const LeaderboardRow& b) {
        return a.totalAchievements > b.totalAchievements;
        });
}

void MenuManager::drawButton(sf::RenderWindow& window, sf::Vector2f pos, const std::wstring& text, bool isHovered, bool isDisabled) {
    sf::Vector2f btnSize(200.f, 30.f);
    sf::RectangleShape btn(btnSize);
    btn.setPosition(pos);
    btn.setOutlineThickness(1.5f);

    if (isDisabled) {
        btn.setFillColor(sf::Color(25, 25, 35, 200));
        btn.setOutlineColor(sf::Color(60, 20, 20));
    }
    else {
        btn.setFillColor(isHovered ? sf::Color(45, 25, 85, 220) : sf::Color(25, 20, 55, 180));
        btn.setOutlineColor(isHovered ? sf::Color(220, 20, 20) : sf::Color(130, 10, 10));
    }
    window.draw(btn);

    sf::Text txt(text, menuFont, 12);
    txt.setStyle(sf::Text::Bold);


    txt.setFillColor(isDisabled ? sf::Color(100, 100, 70) : sf::Color::Yellow);
    txt.setOutlineColor(sf::Color::Black);
    txt.setOutlineThickness(1.5f);

    sf::FloatRect bounds = txt.getLocalBounds();
    txt.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
    txt.setPosition(pos.x + btnSize.x / 2.f, pos.y + btnSize.y / 2.f);
    window.draw(txt);
}


bool MenuManager::loadUserFromFile(const std::wstring& username, const std::wstring& password) {
    std::ifstream file("users_database.bin", std::ios::binary);
    if (!file.is_open()) return false;
    UserData temp;
    while (file.read(reinterpret_cast<char*>(&temp), sizeof(UserData))) {
        std::wstring fUser((wchar_t*)temp.username);
        std::wstring fPass((wchar_t*)temp.password);
        if (fUser == username && fPass == password) {
            currentUser = temp;
            activeTheme = temp.currentTheme;
            isUserLoggedIn = true;
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}

void MenuManager::saveUserToBinary(std::wstring oldName) {
    if (!isUserLoggedIn) return;
    std::vector<UserData> allUsers;
    std::ifstream inFile("users_database.bin", std::ios::binary);
    std::wstring nameToFind = oldName.empty() ? (wchar_t*)currentUser.username : oldName;
    bool found = false;
    if (inFile.is_open()) {
        UserData temp;
        while (inFile.read(reinterpret_cast<char*>(&temp), sizeof(UserData))) {
            if (std::wcscmp((wchar_t*)temp.username, nameToFind.c_str()) == 0) {
                allUsers.push_back(currentUser);
                found = true;
            }
            else {
                allUsers.push_back(temp);
            }
        }
        inFile.close();
    }
    if (!found) {
        allUsers.push_back(currentUser);
    }
    std::ofstream outFile("users_database.bin", std::ios::binary | std::ios::trunc);
    for (const auto& user : allUsers) {
        outFile.write(reinterpret_cast<const char*>(&user), sizeof(UserData));
    }
    outFile.close();
}

void MenuManager::handleMouseClick(sf::Vector2f mPos) {
    if (currentState == AUTH_SCREEN) {
        if (isButtonClicked(mPos, sf::Vector2f(260.f, 120.f), sf::Vector2f(280.f, 25.f))) {
            isUsernameInputActive = true;
            isPasswordInputActive = false;
            authErrorMessage = L"";
            return;
        }
        if (isButtonClicked(mPos, sf::Vector2f(260.f, 170.f), sf::Vector2f(280.f, 25.f))) {
            isUsernameInputActive = false;
            isPasswordInputActive = true;
            authErrorMessage = L"";
            return;
        }
        if (isButtonClicked(mPos, sf::Vector2f(300.f, 230.f), sf::Vector2f(200.f, 30.f))) {
            authErrorMessage = L"";


            int dbStatus = checkUserInDatabase(inputUsername, inputPassword, true);
            if (dbStatus == 2) {
                authErrorMessage = L"Неверный пароль. Попробуйте ещё раз.";
                return;
            }

            if (loadUserFromFile(inputUsername, inputPassword)) {
                isUserLoggedIn = true;
                currentState = MAIN_MENU;
            }
            else {
                std::string sName(inputUsername.begin(), inputUsername.end());
                std::string sPass(inputPassword.begin(), inputPassword.end());
                std::memset(&currentUser, 0, sizeof(UserData));
                std::strncpy(currentUser.username, sName.c_str(), 31);
                std::strncpy(currentUser.password, sPass.c_str(), 31);
                currentUser.playerHealth = 100.f;
                currentUser.musicVolume = 100.f;
                currentUser.soundVolume = 100.f;
                currentUser.laptopCount = 1;
                saveUserToBinary();
                isUserLoggedIn = true;
                currentState = MAIN_MENU;
            }
            return;
        }
    }
    else if (currentState == MAIN_MENU) {
        if (sf::FloatRect(300.f, 130.f, 200.f, 30.f).contains(mPos) && currentUser.lastScene == 0) return;
        if (sf::FloatRect(300.f, 90.f, 200.f, 30.f).contains(mPos)) return;
        if (sf::FloatRect(300.f, 170.f, 200.f, 30.f).contains(mPos)) currentState = DIFFICULTY_CHOICE;
        if (sf::FloatRect(300.f, 210.f, 200.f, 30.f).contains(mPos)) currentState = ACHIEVEMENTS_SCREEN;
        if (sf::FloatRect(300.f, 250.f, 200.f, 30.f).contains(mPos)) {
            currentState = ACCOUNT_CABINET;
            editBuffer.clear();
            return; 
        }
    }

    else if (currentState == DIFFICULTY_CHOICE) {
        if (sf::FloatRect(300.f, 100.f, 200.f, 30.f).contains(mPos)) {
            currentUser.difficultySetting = 0;
            currentUser.lastScene = 0;
            saveUserToBinary();
            currentState = MAIN_MENU;
        }
        if (sf::FloatRect(300.f, 150.f, 200.f, 30.f).contains(mPos)) {
            currentUser.difficultySetting = 1;
            currentUser.lastScene = 0;
            saveUserToBinary();
            currentState = MAIN_MENU;
        }
        if (sf::FloatRect(300.f, 200.f, 200.f, 30.f).contains(mPos)) {
            currentUser.difficultySetting = 2;
            currentUser.lastScene = 0;
            saveUserToBinary();
            currentState = MAIN_MENU;
        }
        if (sf::FloatRect(300.f, 270.f, 200.f, 30.f).contains(mPos)) currentState = MAIN_MENU;
    }
    else if (currentState == ACCOUNT_CABINET) {
        if (isButtonClicked(mPos, sf::Vector2f(300.f, 90.f), sf::Vector2f(200.f, 30.f))) {
            currentState = CHANGE_NAME_SCREEN;
            editBuffer.clear();
            return;
        }
        if (sf::FloatRect(300.f, 130.f, 200.f, 30.f).contains(mPos)) {
            currentState = CHANGE_PASS_SCREEN;
            editBuffer.clear();
        }
        if (sf::FloatRect(300.f, 170.f, 200.f, 30.f).contains(mPos)) currentState = THEME_CHOICE;
        if (sf::FloatRect(300.f, 210.f, 200.f, 30.f).contains(mPos)) {
            isUserLoggedIn = false;
            inputUsername.clear();
            inputPassword.clear();
            currentState = AUTH_SCREEN;
        }
        if (sf::FloatRect(300.f, 260.f, 200.f, 30.f).contains(mPos)) currentState = MAIN_MENU;
    }
    else if (currentState == THEME_CHOICE) {
        if (sf::FloatRect(300.f, 90.f, 200.f, 20.f).contains(mPos)) {
            float newVol = ((mPos.x - 300.f) / 200.f) * 100.f;
            currentUser.musicVolume = std::max(0.f, std::min(100.f, newVol));
            saveUserToBinary();
            return;
        }
        if (sf::FloatRect(300.f, 140.f, 200.f, 20.f).contains(mPos)) {
            float newVol = ((mPos.x - 300.f) / 200.f) * 100.f;
            currentUser.soundVolume = std::max(0.f, std::min(100.f, newVol));
            saveUserToBinary();
            return;
        }
        if (sf::FloatRect(300.f, 180.f, 200.f, 30.f).contains(mPos)) {
            activeTheme = 0;
            currentUser.currentTheme = 0;
            saveUserToBinary();
            return;
        }
        if (sf::FloatRect(300.f, 220.f, 200.f, 30.f).contains(mPos)) {
            activeTheme = 1;
            currentUser.currentTheme = 1;
            saveUserToBinary();
            return;
        }
        if (sf::FloatRect(300.f, 280.f, 200.f, 30.f).contains(mPos)) {
            currentState = ACCOUNT_CABINET;
            return;
        }
    }
    else if (currentState == ACHIEVEMENTS_SCREEN) {
        if (sf::FloatRect(300.f, 340.f, 200.f, 30.f).contains(mPos)) currentState = MAIN_MENU;
    }
    else if (currentState == CHANGE_NAME_SCREEN || currentState == CHANGE_PASS_SCREEN) {
        if (sf::FloatRect(300.f, 210.f, 200.f, 30.f).contains(mPos)) {
            if (!editBuffer.empty()) {
                if (currentState == CHANGE_NAME_SCREEN) {
                    std::string sOldName(currentUser.username);
                    std::wstring old(sOldName.begin(), sOldName.end());
                    if (editBuffer == old) {
                        return;
                    }


                    if (checkUserInDatabase(editBuffer, L"", false) == 3) {
                        return;
                    }

                    inputUsername = editBuffer;
                    std::wcscpy((wchar_t*)currentUser.username, editBuffer.c_str());
                    saveUserToBinary(old);
                }
                else {
                    inputPassword = editBuffer;
                    std::wcscpy((wchar_t*)currentUser.password, editBuffer.c_str());
                    saveUserToBinary();
                }
            }
            currentState = ACCOUNT_CABINET;
        }
        if (sf::FloatRect(300.f, 260.f, 200.f, 30.f).contains(mPos)) currentState = ACCOUNT_CABINET;
    }
}

void MenuManager::draw(sf::RenderWindow& window, sf::View& uiView) {
    window.setView(uiView); 
    window.clear(sf::Color(15, 15, 25));

    static sf::Texture staticBgTex;
    static bool isBgLoaded = false;

    if (!isBgLoaded) {
        if (staticBgTex.loadFromFile("menu_bg.png")) {
            if (staticBgTex.getSize().x > 0 && staticBgTex.getSize().y > 0) {
                staticBgTex.setSmooth(false);
                backgroundSprite.setTexture(staticBgTex, true);
                backgroundSprite.setScale(800.f / staticBgTex.getSize().x, 400.f / staticBgTex.getSize().y);
                isBgLoaded = true;
            }
        }
    }

    if (isBgLoaded && staticBgTex.getSize().x > 0) {
        backgroundSprite.setTexture(staticBgTex);
        window.draw(backgroundSprite);
    }

    sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
    sf::Vector2f mPos = window.mapPixelToCoords(pixelPos, uiView);


    if (currentState == AUTH_SCREEN) {
        sf::Text title(L"ВХОД В БУНКЕР", menuFont, 24);
        title.setStyle(sf::Text::Bold);
        title.setFillColor(sf::Color::White);
        title.setOutlineColor(sf::Color::Black);
        title.setOutlineThickness(2.f);
        sf::FloatRect tBounds = title.getLocalBounds();
        title.setOrigin(tBounds.left + tBounds.width / 2.f, tBounds.top + tBounds.height / 2.f);
        title.setPosition(400.f, 60.f);
        window.draw(title);

        sf::RectangleShape uBox(sf::Vector2f(280.f, 25.f));
        uBox.setPosition(260.f, 120.f);
        uBox.setFillColor(isUsernameInputActive ? sf::Color(60, 60, 85, 200) : sf::Color(30, 30, 45, 180));
        window.draw(uBox);

        sf::Text uText(inputUsername.empty() ? L"Логин..." : inputUsername, menuFont, 11);
        uText.setPosition(270.f, 125.f);
        window.draw(uText);

        sf::RectangleShape pBox(sf::Vector2f(280.f, 25.f));
        pBox.setPosition(260.f, 170.f);
        pBox.setFillColor(isPasswordInputActive ? sf::Color(60, 60, 85, 200) : sf::Color(30, 30, 45, 180));
        window.draw(pBox);

        std::wstring mPass(inputPassword.size(), L'*');
        sf::Text pText(inputPassword.empty() ? L"Пароль..." : mPass, menuFont, 11);
        pText.setPosition(270.f, 175.f);
        window.draw(pText);

        drawButton(window, sf::Vector2f(300.f, 230.f), L"Войти / Создать", sf::FloatRect(300.f, 230.f, 200.f, 30.f).contains(mPos));

        if (!authErrorMessage.empty()) {
            sf::Text errorText(authErrorMessage, menuFont, 14);
            errorText.setStyle(sf::Text::Bold);
            errorText.setFillColor(sf::Color(220, 60, 60));
            errorText.setOutlineColor(sf::Color::Black);
            errorText.setOutlineThickness(1.5f);
            sf::FloatRect eBounds = errorText.getLocalBounds();
            errorText.setOrigin(eBounds.left + eBounds.width / 2.f, eBounds.top + eBounds.height / 2.f);
            errorText.setPosition(400.f, 290.f);
            window.draw(errorText);
        }
    }
    else if (currentState == MAIN_MENU) {
        sf::Text title(L"ФИДЕС", menuFont, 28);
        title.setStyle(sf::Text::Bold);
        title.setFillColor(sf::Color::White);
        title.setOutlineColor(sf::Color::Black);
        title.setOutlineThickness(2.5f);
        title.setPosition(60.f, 35.f);
        window.draw(title);

        bool hasSave = (currentUser.lastScene > 0);
        drawButton(window, sf::Vector2f(300.f, 90.f), L"Новая игра", sf::FloatRect(300.f, 90.f, 200.f, 30.f).contains(mPos));
        drawButton(window, sf::Vector2f(300.f, 130.f), L"Продолжить игру", sf::FloatRect(300.f, 130.f, 200.f, 30.f).contains(mPos), !hasSave);
        drawButton(window, sf::Vector2f(300.f, 170.f), L"Уровни сложности", sf::FloatRect(300.f, 170.f, 200.f, 30.f).contains(mPos));
        drawButton(window, sf::Vector2f(300.f, 210.f), L"Топ и Достижения", sf::FloatRect(300.f, 210.f, 200.f, 30.f).contains(mPos));
        drawButton(window, sf::Vector2f(300.f, 250.f), L"Мой аккаунт", sf::FloatRect(300.f, 250.f, 200.f, 30.f).contains(mPos));
    }
    else if (currentState == DIFFICULTY_CHOICE) {
        sf::Text title(L"ВЫБОР СЛОЖНОСТИ", menuFont, 24);
        title.setStyle(sf::Text::Bold);
        title.setFillColor(sf::Color::White);
        title.setOutlineColor(sf::Color::Black);
        title.setOutlineThickness(2.f);
        sf::FloatRect tBounds = title.getLocalBounds();
        title.setOrigin(tBounds.left + tBounds.width / 2.f, tBounds.top + tBounds.height / 2.f);
        title.setPosition(400.f, 50.f);
        window.draw(title);

        drawButton(window, sf::Vector2f(300.f, 100.f), L"Легко", sf::FloatRect(300.f, 100.f, 200.f, 30.f).contains(mPos));
        drawButton(window, sf::Vector2f(300.f, 150.f), L"Нормально", sf::FloatRect(300.f, 150.f, 200.f, 30.f).contains(mPos));
        drawButton(window, sf::Vector2f(300.f, 200.f), L"Выживание", sf::FloatRect(300.f, 200.f, 200.f, 30.f).contains(mPos));
        drawButton(window, sf::Vector2f(300.f, 270.f), L"Назад", sf::FloatRect(300.f, 270.f, 200.f, 30.f).contains(mPos));
    }
    else if (currentState == ACCOUNT_CABINET) {
        sf::Text title(L"ЛИЧНЫЙ КАБИНЕТ", menuFont, 24);
        title.setStyle(sf::Text::Bold);
        title.setFillColor(sf::Color::White);
        title.setOutlineColor(sf::Color::Black);
        title.setOutlineThickness(2.f);
        sf::FloatRect tBounds = title.getLocalBounds();
        title.setOrigin(tBounds.left + tBounds.width / 2.f, tBounds.top + tBounds.height / 2.f);
        title.setPosition(400.f, 40.f);
        window.draw(title);

        drawButton(window, sf::Vector2f(300.f, 90.f), L"Изменить логин", sf::FloatRect(300.f, 90.f, 200.f, 30.f).contains(mPos));
        drawButton(window, sf::Vector2f(300.f, 130.f), L"Изменить пароль", sf::FloatRect(300.f, 130.f, 200.f, 30.f).contains(mPos));
        drawButton(window, sf::Vector2f(300.f, 170.f), L"Оформление", sf::FloatRect(300.f, 170.f, 200.f, 30.f).contains(mPos));
        drawButton(window, sf::Vector2f(300.f, 210.f), L"Выйти из аккаунта", sf::FloatRect(300.f, 210.f, 200.f, 30.f).contains(mPos));
        drawButton(window, sf::Vector2f(300.f, 260.f), L"Назад в меню", sf::FloatRect(300.f, 260.f, 200.f, 30.f).contains(mPos));
    }
    else if (currentState == THEME_CHOICE) {
        sf::Text title(L"НАСТРОЙКИ ЗВУКА И ТЕМ", menuFont, 24);
        title.setStyle(sf::Text::Bold);
        title.setFillColor(sf::Color::White);
        title.setOutlineColor(sf::Color::Black);
        title.setOutlineThickness(2.f);
        sf::FloatRect tBounds = title.getLocalBounds();
        title.setOrigin(tBounds.left + tBounds.width / 2.f, tBounds.top + tBounds.height / 2.f);
        title.setPosition(400.f, 40.f);
        window.draw(title);

        sf::Text mLabel(L"Громкость музыки:", menuFont, 12);
        mLabel.setStyle(sf::Text::Bold);
        mLabel.setFillColor(sf::Color::White);
        mLabel.setOutlineColor(sf::Color::Black);
        mLabel.setOutlineThickness(1.5f);
        mLabel.setPosition(300.f, 75.f);
        window.draw(mLabel);

        sf::RectangleShape mTrack(sf::Vector2f(200.f, 4.f));
        mTrack.setPosition(300.f, 95.f);
        mTrack.setFillColor(sf::Color(80, 20, 20));
        window.draw(mTrack);

        sf::CircleShape mSlider(6.f);
        mSlider.setOrigin(6.f, 6.f);
        mSlider.setPosition(300.f + (currentUser.musicVolume / 100.f) * 200.f, 97.f);
        mSlider.setFillColor(sf::Color::Red);
        window.draw(mSlider);

        sf::Text sLabel(L"Громкость звуков:", menuFont, 12);
        sLabel.setStyle(sf::Text::Bold);
        sLabel.setFillColor(sf::Color::White);
        sLabel.setOutlineColor(sf::Color::Black);
        sLabel.setOutlineThickness(1.5f);
        sLabel.setPosition(300.f, 125.f);
        window.draw(sLabel);

        sf::RectangleShape sTrack(sf::Vector2f(200.f, 4.f));
        sTrack.setPosition(300.f, 145.f);
        sTrack.setFillColor(sf::Color(80, 20, 20));
        window.draw(sTrack);

        sf::CircleShape sSlider(6.f);
        sSlider.setOrigin(6.f, 6.f);
        sSlider.setPosition(300.f + (currentUser.soundVolume / 100.f) * 200.f, 147.f);
        sSlider.setFillColor(sf::Color::Red);
        window.draw(sSlider);

        drawButton(window, sf::Vector2f(300.f, 180.f), L"Темная тема", sf::FloatRect(300.f, 180.f, 200.f, 30.f).contains(mPos));
        drawButton(window, sf::Vector2f(300.f, 220.f), L"Светлая тема", sf::FloatRect(300.f, 220.f, 200.f, 30.f).contains(mPos));
        drawButton(window, sf::Vector2f(300.f, 280.f), L"Назад", sf::FloatRect(300.f, 280.f, 200.f, 30.f).contains(mPos));
    }
    else if (currentState == ACHIEVEMENTS_SCREEN) {
        sf::Text title(L"ДОСТИЖЕНИЯ ИГРОКА", menuFont, 24);
        title.setStyle(sf::Text::Bold);
        title.setFillColor(sf::Color::White);
        title.setOutlineColor(sf::Color::Black);
        title.setOutlineThickness(2.f);
        sf::FloatRect tBounds = title.getLocalBounds();
        title.setOrigin(tBounds.left + tBounds.width / 2.f, tBounds.top + tBounds.height / 2.f);
        title.setPosition(400.f, 40.f);
        window.draw(title);

        std::wstring achNames[8] = {
            L"Первый шаг", L"Защитник Марка", L"Оружейник", L"Взломщик",
            L"Ветеран бункера", L"Бессмертный", L"Коллекционер", L"Финал истории"
        };

        for (int i = 0; i < 8; ++i) {
            float xPos = (i < 4) ? 150.f : 450.f;
            float yPos = 90.f + (i % 4) * 55.f;

            sf::RectangleShape box(sf::Vector2f(220.f, 40.f));
            box.setPosition(xPos, yPos);
            box.setOutlineThickness(1.f);

            if (currentUser.achievements[i]) {
                box.setFillColor(sf::Color(45, 25, 85, 200));
                box.setOutlineColor(sf::Color::Yellow);
            }
            else {
                box.setFillColor(sf::Color(20, 20, 25, 180));
                box.setOutlineColor(sf::Color(80, 80, 80));
            }
            window.draw(box);

            sf::Text aText(achNames[i], menuFont, 11);
            aText.setStyle(sf::Text::Bold);
            aText.setFillColor(currentUser.achievements[i] ? sf::Color::Yellow : sf::Color(140, 140, 140));
            aText.setPosition(xPos + 15.f, yPos + 12.f);
            window.draw(aText);
        }

        drawButton(window, sf::Vector2f(300.f, 340.f), L"Назад в меню", sf::FloatRect(300.f, 340.f, 200.f, 30.f).contains(mPos));
    }
    else if (currentState == CHANGE_NAME_SCREEN || currentState == CHANGE_PASS_SCREEN) {
        sf::Text title(currentState == CHANGE_NAME_SCREEN ? L"ИЗМЕНЕНИЕ ЛОГИНА" : L"ИЗМЕНЕНИЕ ПАРОЛЯ", menuFont, 24);
        title.setStyle(sf::Text::Bold);
        title.setFillColor(sf::Color::White);
        title.setOutlineColor(sf::Color::Black);
        title.setOutlineThickness(2.f);
        sf::FloatRect tBounds = title.getLocalBounds();
        title.setOrigin(tBounds.left + tBounds.width / 2.f, tBounds.top + tBounds.height / 2.f);
        title.setPosition(400.f, 50.f);
        window.draw(title);
        sf::RectangleShape eBox(sf::Vector2f(280.f, 25.f));
        eBox.setPosition(260.f, 130.f);
        eBox.setFillColor(sf::Color(60, 60, 85, 200));
        window.draw(eBox);
        sf::Text eText(editBuffer.empty() ? L"Введите новые данные..." : editBuffer, menuFont, 11);
        eText.setPosition(270.f, 135.f);
        window.draw(eText);
        drawButton(window, sf::Vector2f(300.f, 210.f), L"Сохранить", sf::FloatRect(300.f, 210.f, 200.f, 30.f).contains(mPos));
        drawButton(window, sf::Vector2f(300.f, 260.f), L"Назад", sf::FloatRect(300.f, 260.f, 200.f, 30.f).contains(mPos));
    }
}


bool MenuManager::isButtonClicked(sf::Vector2f mousePos, sf::Vector2f btnPos, sf::Vector2f btnSize) {
    return sf::FloatRect(btnPos, btnSize).contains(mousePos);
}

float MenuManager::handleSliderLogic(sf::Vector2f mousePos, sf::Vector2f trackPos, float trackWidth) {
    float relativeX = mousePos.x - trackPos.x;
    float percentage = (relativeX / trackWidth) * 100.f;
    return std::max(0.f, std::min(100.f, percentage));
}

void MenuManager::drawVolumeSlider(sf::RenderWindow& window, const std::wstring& title, float volume, sf::Vector2f pos) {
    sf::Text text(title + L": " + std::to_wstring(int(volume)) + L"%", menuFont, 10);
    text.setPosition(pos.x, pos.y);
    text.setFillColor(activeTheme == 0 ? sf::Color::White : sf::Color::Black);
    window.draw(text);

    sf::RectangleShape track(sf::Vector2f(200.f, 4.f));
    track.setPosition(pos.x, pos.y + 25.f);
    track.setFillColor(sf::Color(80, 80, 80));
    window.draw(track);

    sf::CircleShape handle(6.f);
    handle.setOrigin(6.f, 6.f);
    handle.setPosition(pos.x + (volume / 100.f) * 200.f, pos.y + 27.f);
    handle.setFillColor(sf::Color(100, 30, 180));
    window.draw(handle);
}

void MenuManager::updateMenu(sf::RenderWindow& window, sf::View& uiView) {
    if (currentState == THEME_CHOICE) {
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            sf::Vector2f mUI = window.mapPixelToCoords(mousePos, uiView);

            if (isButtonClicked(mUI, sf::Vector2f(300.f, 140.f), sf::Vector2f(200.f, 25.f))) {
                currentUser.musicVolume = handleSliderLogic(mUI, sf::Vector2f(300.f, 165.f), 200.f);
                saveUserToBinary();
            }
            else if (isButtonClicked(mUI, sf::Vector2f(300.f, 190.f), sf::Vector2f(200.f, 25.f))) {
                currentUser.soundVolume = handleSliderLogic(mUI, sf::Vector2f(300.f, 215.f), 200.f);
                saveUserToBinary();
            }
        }
    }
}


int MenuManager::checkUserInDatabase(const std::wstring& checkName, const std::wstring& checkPass, bool verifyPassword) {
    std::ifstream in("users_database.bin", std::ios::binary);
    if (!in.is_open()) return 0;

    UserData tempUser;
    while (in.read(reinterpret_cast<char*>(&tempUser), sizeof(UserData))) {
        std::string sName(tempUser.username);
        std::wstring storedName(sName.begin(), sName.end());

        std::string sPass(tempUser.password);
        std::wstring storedPass(sPass.begin(), sPass.end());

        if (storedName == checkName) {
            in.close();
            if (verifyPassword) {
                return (storedPass == checkPass) ? 1 : 2;
            }
            return 3;
        }
    }
    in.close();
    return 0;
}

void MenuManager::handleTextEvent(sf::Event event) {
    if (event.type == sf::Event::TextEntered) {
        wchar_t charEntered = static_cast<wchar_t>(event.text.unicode);

        if (currentState == AUTH_SCREEN) {
            if (isUsernameInputActive) {
                if (charEntered == L'\b') {
                    if (!inputUsername.empty()) inputUsername.pop_back();
                }
                else if (charEntered >= 32 && inputUsername.size() < 30) {
                    inputUsername += charEntered;
                }
            }
            else if (isPasswordInputActive) {
                if (charEntered == L'\b') {
                    if (!inputPassword.empty()) inputPassword.pop_back();
                }
                else if (charEntered >= 32 && inputPassword.size() < 30) {
                    inputPassword += charEntered;
                }
            }
        }
        else if (currentState == CHANGE_NAME_SCREEN || currentState == CHANGE_PASS_SCREEN) {
            if (charEntered == L'\b') {
                if (!editBuffer.empty()) editBuffer.pop_back();
            }
            else if (charEntered >= 32 && editBuffer.size() < 30) {
                editBuffer += charEntered;
            }
        }
    }
}


