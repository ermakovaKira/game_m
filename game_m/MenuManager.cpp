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
        btn.setFillColor(sf::Color(60, 60, 65));
        btn.setOutlineColor(sf::Color(100, 100, 100));
    }
    else if (activeTheme == 0) {
        btn.setFillColor(isHovered ? sf::Color(100, 30, 180) : sf::Color(45, 15, 75));
        btn.setOutlineColor(sf::Color(255, 215, 0));
    }
    else {
        btn.setFillColor(isHovered ? sf::Color(200, 200, 240) : sf::Color(240, 240, 240));
        btn.setOutlineColor(sf::Color(50, 50, 150));
    }
    window.draw(btn);
    sf::Text txt(text, menuFont, 11);
    txt.setFillColor(isDisabled ? sf::Color(150, 150, 150) : (activeTheme == 0 ? sf::Color::White : sf::Color::Black));
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

            // ДОБАВЛЕНО: Если логин существует, но пароль не подошёл — выводим ошибку и блокируем вход
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

void MenuManager::draw(sf::RenderWindow& window) {
    sf::Color bgCol = (activeTheme == 0) ? sf::Color(10, 10, 25) : sf::Color(225, 230, 240);
    window.clear(bgCol);
    sf::Vector2f mPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    if (currentState == AUTH_SCREEN) {
        sf::Text title(L"FIDES: LOGIN", menuFont, 16);
        title.setFillColor(activeTheme == 0 ? sf::Color::Yellow : sf::Color::Black);
        title.setPosition(330.f, 50.f);
        window.draw(title);

        sf::RectangleShape uBox(sf::Vector2f(280.f, 25.f));
        uBox.setPosition(260.f, 120.f);
        uBox.setFillColor(isUsernameInputActive ? sf::Color(60, 60, 85) : sf::Color(30, 30, 45));
        window.draw(uBox);

        sf::Text uText(inputUsername.empty() ? L"Логин..." : inputUsername, menuFont, 10);
        uText.setPosition(270.f, 125.f);
        window.draw(uText);

        sf::RectangleShape pBox(sf::Vector2f(280.f, 25.f));
        pBox.setPosition(260.f, 170.f);
        pBox.setFillColor(isPasswordInputActive ? sf::Color(60, 60, 85) : sf::Color(30, 30, 45));
        window.draw(pBox);

        std::wstring mPass(inputPassword.size(), L'*');
        sf::Text pText(inputPassword.empty() ? L"Пароль..." : mPass, menuFont, 10);
        pText.setPosition(270.f, 175.f);
        window.draw(pText);

        drawButton(window, sf::Vector2f(300.f, 230.f), L"Войти / Создать", sf::FloatRect(300.f, 230.f, 200.f, 30.f).contains(mPos));
        if (!authErrorMessage.empty()) {
            sf::Text errorText(authErrorMessage, menuFont, 12);
            errorText.setFillColor(sf::Color(220, 60, 60));
            errorText.setPosition(300.f, 280.f);
            window.draw(errorText);
        }

    }
    else if (currentState == MAIN_MENU) {
        sf::Text title(L"FIDES: MAIN MENU", menuFont, 16);
        title.setFillColor(sf::Color::Red);
        title.setPosition(310.f, 30.f);
        window.draw(title);

        bool hasSave = (currentUser.lastScene > 0);
        drawButton(window, sf::Vector2f(300.f, 90.f), L"Начать игру", sf::FloatRect(300.f, 90.f, 200.f, 30.f).contains(mPos));
        drawButton(window, sf::Vector2f(300.f, 130.f), L"Продолжить игру", sf::FloatRect(300.f, 130.f, 200.f, 30.f).contains(mPos), !hasSave);
        drawButton(window, sf::Vector2f(300.f, 170.f), L"Уровни сложности", sf::FloatRect(300.f, 170.f, 200.f, 30.f).contains(mPos));
        drawButton(window, sf::Vector2f(300.f, 210.f), L"Топ и Достижения", sf::FloatRect(300.f, 210.f, 200.f, 30.f).contains(mPos));
        drawButton(window, sf::Vector2f(300.f, 250.f), L"Мой аккаунт", sf::FloatRect(300.f, 250.f, 200.f, 30.f).contains(mPos));
    }
    else if (currentState == DIFFICULTY_CHOICE) {
        sf::Text title(L"УРОВЕНЬ СЛОЖНОСТИ", menuFont, 14);
        title.setPosition(320.f, 40.f);
        title.setFillColor(sf::Color::Cyan);
        window.draw(title);

        drawButton(window, sf::Vector2f(300.f, 100.f), L"Легкий", sf::FloatRect(300.f, 100.f, 200.f, 30.f).contains(mPos));
        drawButton(window, sf::Vector2f(300.f, 150.f), L"Базовый", sf::FloatRect(300.f, 150.f, 200.f, 30.f).contains(mPos));
        drawButton(window, sf::Vector2f(300.f, 200.f), L"Сложный", sf::FloatRect(300.f, 200.f, 200.f, 30.f).contains(mPos));
        drawButton(window, sf::Vector2f(300.f, 270.f), L"Назад", sf::FloatRect(300.f, 270.f, 200.f, 30.f).contains(mPos));
    }
    else if (currentState == ACCOUNT_CABINET) {
        sf::Text title(L"FIDES: ACCOUNT CABINET", menuFont, 16);
        title.setFillColor(sf::Color::Cyan);
        title.setPosition(300.f, 30.f);
        window.draw(title);

        drawButton(window, sf::Vector2f(300.f, 90.f), L"Изменить логин", sf::FloatRect(300.f, 90.f, 200.f, 30.f).contains(mPos));
        drawButton(window, sf::Vector2f(300.f, 130.f), L"Изменить пароль", sf::FloatRect(300.f, 130.f, 200.f, 30.f).contains(mPos));
        drawButton(window, sf::Vector2f(300.f, 170.f), L"Оформление", sf::FloatRect(300.f, 170.f, 200.f, 30.f).contains(mPos));
        drawButton(window, sf::Vector2f(300.f, 210.f), L"Выйти из аккаунта", sf::FloatRect(300.f, 210.f, 200.f, 30.f).contains(mPos));
        drawButton(window, sf::Vector2f(300.f, 260.f), L"Назад в меню", sf::FloatRect(300.f, 260.f, 200.f, 30.f).contains(mPos));
    }


    else if (currentState == THEME_CHOICE) {
        sf::Text tTitle(L"НАСТРОЙКИ ЗВУКА И ИНТЕРФЕЙСА", menuFont, 14);
        tTitle.setPosition(260.f, 40.f);
        tTitle.setFillColor(activeTheme == 0 ? sf::Color::Cyan : sf::Color::Black);
        window.draw(tTitle);


        drawVolumeSlider(window, L"Громкость музыки", currentUser.musicVolume, sf::Vector2f(300.f, 140.f));
        drawVolumeSlider(window, L"Громкость звуков", currentUser.soundVolume, sf::Vector2f(300.f, 190.f));


        drawButton(window, sf::Vector2f(300.f, 180.f), L"Тёмная тема", sf::FloatRect(300.f, 180.f, 200.f, 30.f).contains(mPos));
        drawButton(window, sf::Vector2f(300.f, 220.f), L"Светлая тема", sf::FloatRect(300.f, 220.f, 200.f, 30.f).contains(mPos));
        drawButton(window, sf::Vector2f(300.f, 280.f), L"Назад", sf::FloatRect(300.f, 280.f, 200.f, 30.f).contains(mPos));
    }

    else if (currentState == THEME_CHOICE) {
        sf::Text tTitle(L"НАСТРОЙКИ ЗВУКА И ИНТЕРФЕЙСА", menuFont, 14);
        tTitle.setPosition(260.f, 40.f);
        tTitle.setFillColor(activeTheme == 0 ? sf::Color::Cyan : sf::Color::Black);
        window.draw(tTitle);

        float musVol = currentUser.musicVolume;
        sf::Text mText(L"Музыка: " + std::to_wstring(int(musVol)) + L"%", menuFont, 10);
        mText.setPosition(300.f, 75.f);
        mText.setFillColor(activeTheme == 0 ? sf::Color::White : sf::Color::Black);
        window.draw(mText);

        sf::RectangleShape mTrack(sf::Vector2f(200.f, 4.f));
        mTrack.setPosition(300.f, 95.f);
        mTrack.setFillColor(sf::Color(80, 80, 80));
        window.draw(mTrack);

        sf::CircleShape mHandle(6.f);
        mHandle.setOrigin(6.f, 6.f);
        mHandle.setPosition(300.f + (musVol / 100.f) * 200.f, 97.f);
        mHandle.setFillColor(sf::Color(100, 30, 180));
        window.draw(mHandle);

        float sndVol = currentUser.soundVolume;
        sf::Text sText(L"Звуки: " + std::to_wstring(int(sndVol)) + L"%", menuFont, 10);
        sText.setPosition(300.f, 125.f);
        sText.setFillColor(activeTheme == 0 ? sf::Color::White : sf::Color::Black);
        window.draw(sText);

        sf::RectangleShape sTrack(sf::Vector2f(200.f, 4.f));
        sTrack.setPosition(300.f, 145.f);
        sTrack.setFillColor(sf::Color(80, 80, 80));
        window.draw(sTrack);

        sf::CircleShape sHandle(6.f);
        sHandle.setOrigin(6.f, 6.f);
        sHandle.setPosition(300.f + (sndVol / 100.f) * 200.f, 147.f);
        sHandle.setFillColor(sf::Color(100, 30, 180));
        window.draw(sHandle);

        drawButton(window, sf::Vector2f(300.f, 180.f), L"Тёмная тема", sf::FloatRect(300.f, 180.f, 200.f, 30.f).contains(mPos));
        drawButton(window, sf::Vector2f(300.f, 220.f), L"Светлая тема", sf::FloatRect(300.f, 220.f, 200.f, 30.f).contains(mPos));
        drawButton(window, sf::Vector2f(300.f, 280.f), L"Назад", sf::FloatRect(300.f, 280.f, 200.f, 30.f).contains(mPos));
    }

    else if (currentState == ACHIEVEMENTS_SCREEN) {
        sf::Text title(L"ДОСТИЖЕНИЯ И ТОП ИГРОКОВ", menuFont, 14);
        title.setPosition(50.f, 30.f);
        title.setFillColor(sf::Color::Yellow);
        window.draw(title);

        std::wstring a[8];
        a[0] = L"1. Коллекционер (Легкий): " + std::wstring(currentUser.achievements[0] ? L"ОТКРЫТО" : L"ЗАКРЫТО");
        a[1] = L"2. Архивариус (Базовый): " + std::wstring(currentUser.achievements[1] ? L"ОТКРЫТО" : L"ЗАКРЫТО");
        a[2] = L"3. Историк бункера (Сложный): " + std::wstring(currentUser.achievements[2] ? L"ОТКРЫТО" : L"ЗАКРЫТО");
        a[3] = L"4. В упор!: " + std::wstring(currentUser.achievements[3] ? L"ОТКРЫТО" : L"ЗАКРЫТО");
        a[4] = L"5. На волоске (10% HP): " + std::wstring(currentUser.achievements[4] ? L"ОТКРЫТО" : L"ЗАКРЫТО");
        a[5] = L"6. Медицинский аскетизм: " + std::wstring(currentUser.achievements[5] ? L"ОТКРЫТО" : L"ЗАКРЫТО");
        a[6] = L"7. Снайпер сектора: " + std::wstring(currentUser.achievements[6] ? L"ОТКРЫТО" : L"ЗАКРЫТО");
        a[7] = L"8. Спринтер: " + std::wstring(currentUser.achievements[7] ? L"ОТКРЫТО" : L"ЗАКРЫТО");

        for (int i = 0; i < 8; i++) {
            sf::Text ta(a[i], menuFont, 9);
            ta.setPosition(40.f, 75.f + (i * 28.f));
            window.draw(ta);
        }

        sf::Text tLeader(L"РЕЙТИНГ ПО ЛИДЕРАМ:", menuFont, 12);
        tLeader.setPosition(480.f, 50.f);
        tLeader.setFillColor(sf::Color::Magenta);
        window.draw(tLeader);

        size_t maxRows = std::min(leaderboard.size(), size_t(7));
        for (size_t i = 0; i < maxRows; i++) {
            std::wstring rowStr = std::to_wstring(i + 1) + L". " + leaderboard[i].name + L" (" + std::to_wstring(leaderboard[i].totalAchievements) + L" ачивок)";
            sf::Text tRow(rowStr, menuFont, 10);
            tRow.setPosition(480.f, 85.f + (i * 24.f));
            window.draw(tRow);
        }
        drawButton(window, sf::Vector2f(300.f, 340.f), L"Вернуться в меню", sf::FloatRect(300.f, 340.f, 200.f, 30.f).contains(mPos));
    }
    else if (currentState == CHANGE_NAME_SCREEN || currentState == CHANGE_PASS_SCREEN) {
        sf::Text title(currentState == CHANGE_NAME_SCREEN ? L"ИЗМЕНЕНИЕ ЛОГИНА" : L"ИЗМЕНЕНИЕ ПАРОЛЯ", menuFont, 14);
        title.setPosition(320.f, 50.f);
        title.setFillColor(activeTheme == 0 ? sf::Color::Cyan : sf::Color::Black);
        window.draw(title);

        sf::RectangleShape eBox(sf::Vector2f(280.f, 25.f));
        eBox.setPosition(260.f, 130.f);
        eBox.setFillColor(sf::Color(60, 60, 85));
        window.draw(eBox);

        sf::Text eText(editBuffer.empty() ? L"Введите новые данные..." : editBuffer, menuFont, 10);
        eText.setPosition(270.f, 135.f);
        window.draw(eText);

        drawButton(window, sf::Vector2f(300.f, 210.f), L"Сохранить", sf::FloatRect(300.f, 210.f, 200.f, 30.f).contains(mPos));
        drawButton(window, sf::Vector2f(300.f, 260.f), L"Назад", sf::FloatRect(300.f, 260.f, 200.f, 30.f).contains(mPos));
        }

    window.display();
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


