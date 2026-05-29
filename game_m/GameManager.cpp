#include "GameManager.h"
#include "ApartmentScene.h"
#include "HallwayScene.h"
#include "ElevatorScene.h"
#include "TechScene.h"
#include "MedScene.h"
#include <algorithm>
//2300
GameManager::GameManager()
    : window(sf::VideoMode(800, 400), "Survival RPG - Fides: Point of No Return"),
    gameView(sf::FloatRect(0.f, 0.f, 800.f, 400.f)),
    uiView(sf::FloatRect(0.f, 0.f, 800.f, 400.f)),
    gameDb(NORMAL), story(NORMAL),
    hero("sprite_main.png", "spr_streilba_m2.png", 150, 234),
    playerHPBar(150.f, 12.f, sf::Color::Green),
    zombieHPBar(60.f, 7.f, sf::Color::Red),
    wasShootingLastFrame(false)
{
    isSoundMenuOpen = false;

    if (menuMusic.openFromFile("audio/menu_theme.ogg")) {
        menuMusic.setLoop(true);
        menuMusic.setVolume(menu.getCurrentUser().musicVolume);
        menuMusic.play();
    }
    if (gameMusic.openFromFile("audio/game_theme.ogg")) {
        gameMusic.setLoop(true);
        gameMusic.setVolume(menu.getCurrentUser().musicVolume);
    }
    if (shootBuffer.loadFromFile("audio/shoot.wav")) {
        shootSound.setBuffer(shootBuffer);
    }
    if (hitBuffer.loadFromFile("audio/hit.wav")) {
        hitSound.setBuffer(hitBuffer);
    }

    isGamePassed = false;
    achievementPopupTimer = 0.f;
    showAchievementPopup = false;
    achievementPopupText.setFont(questFont);
    achievementPopupText.setCharacterSize(10);
    achievementPopupText.setFillColor(sf::Color::Yellow);
    achievementPopupText.setOutlineThickness(1.f);
    achievementPopupText.setOutlineColor(sf::Color::Black);


    menu.saveUserToBinary();
    techClockStarted = false;
    failedCloseRange = false;
    totalShots = 0;
    totalHits = 0;
    usedMedkitOnHard = false;

    showInventory = false;
    isGamePaused = false;
    isHeroDead = false;
    pendingMedkitMessage = false;

    window.setFramerateLimit(60);
    window.setView(gameView);

    hero.sprite.setPosition(1150.f, 210.f);
    hero.db = &gameDb;

    hero.inventory.loadItemTexture("Laptop", "icons/laptop.png");
    hero.inventory.loadItemTexture("Note", "icons/note.png");
    hero.inventory.loadItemTexture("Note2", "icons/note_tolya.png");
    hero.inventory.loadItemTexture("PDA", "icons/pda.png");
    hero.inventory.loadItemTexture("Medkit", "icons/medkit.png");
    hero.inventory.loadItemTexture("Ammo", "icons/ammo.png");
    hero.inventory.loadItemTexture("Keys", "icons/ammo.png");
    hero.inventory.addItem("Laptop", 1);

    if (!bulletTex.loadFromFile("icons/ammo.png")) {
        std::cout << "Warning: icons/ammo.png not found for bullet texture" << std::endl;
    }
    if (!tunnelTex.loadFromFile("tunnel_escape.png")) {
        std::cout << "Warning: tunnel_escape.png not found!" << std::endl;
    }
    if (!graveyardTex.loadFromFile("graveyard_final.png")) {
        std::cout << "Warning: graveyard_final.png not found!" << std::endl;
    }

    showInventory = false;
    pendingMedkitMessage = false;
    pendingGlockMessage = false;

    playerHPBar.update(100.f, 100.f, sf::Vector2f(20.f, 20.f));

    if (!questBoxTex.loadFromFile("icons/quest_bg.png")) {
        std::cout << "Error: icons/quest_bg.png not found!" << std::endl;
    }
    questBoxTex.setSmooth(false);
    questBoxSprite.setTexture(questBoxTex);

    float targetWidth = 440.f;
    float targetHeight = 32.f;
    questBoxSprite.setScale(targetWidth / questBoxSprite.getLocalBounds().width, targetHeight / questBoxSprite.getLocalBounds().height);
    questBoxSprite.setColor(sf::Color(255, 255, 255, 160));
    questBoxSprite.setPosition(180.f, 15.f);

    questFont.loadFromFile("PixeloidSans.ttf");
    questText.setFont(questFont);
    questText.setCharacterSize(11);
    questText.setFillColor(sf::Color(255, 215, 0));
}

void GameManager::checkBulletCollisions(float time, Enemy& enemy, float cameraX, const std::wstring& hitText, sf::Color textColor) {
    for (auto it = activeBullets.begin(); it != activeBullets.end(); ) {

        it->update(time);

        if (it->sprite.getGlobalBounds().intersects(enemy.getGlobalBounds()) && enemy.health > 0) {
            totalHits++;
            float dist = std::abs(hero.sprite.getPosition().x - enemy.getPosition().x);
            if (dist > 300.f) {
                failedCloseRange = true;
            }
            enemy.health -= 25.f;
            hero.showMessage(hitText, textColor);
            it = activeBullets.erase(it);
        }
        else {
            float bX = it->sprite.getPosition().x;
            if (bX < cameraX - 450.f || bX > cameraX + 450.f) {
                it = activeBullets.erase(it);
            }
            else {
                ++it;
            }
        }
    }
}


void GameManager::saveCurrentProgress(int nextSceneNum) {
    UserData& u = menu.getCurrentUser();
    u.lastScene = nextSceneNum;
    u.playerHealth = hero.stats.health;
    u.laptopCount = hero.inventory.items["Laptop"];
    u.ammoCount = hero.inventory.items["Ammo"];
    u.medkitCount = hero.inventory.items["Medkit"];
    u.keysCount = hero.inventory.items["Keys"];
    u.noteCount = hero.inventory.items["Note"];
    u.note2Count = hero.inventory.items["Note2"];
    u.pdaCount = hero.inventory.items["PDA"];

    menu.saveUserToBinary();
}


void GameManager::run() {
    while (window.isOpen()) {
        float time = clock.getElapsedTime().asMicroseconds() / 700.0f;
        clock.restart();

        if (menu.getState() != MenuManager::GAME_ACTIVE) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();

                menu.handleTextEvent(event);

                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    sf::Vector2f mUI(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
                    MenuManager::MenuState prevState = menu.getState();
                    menu.handleMouseClick(mUI);

                    if (prevState == MenuManager::MAIN_MENU && sf::FloatRect(300.f, 90.f, 200.f, 28.f).contains(mUI)) {
                        menuMusic.stop();
                        gameMusic.setVolume(menu.getCurrentUser().musicVolume);
                        gameMusic.play();
                        menu.setState(MenuManager::GAME_ACTIVE);
                        initNewGameSession();
                    }
                    if (prevState == MenuManager::MAIN_MENU && sf::FloatRect(300.f, 130.f, 200.f, 28.f).contains(mUI)) {
                        menuMusic.stop();
                        gameMusic.setVolume(menu.getCurrentUser().musicVolume);
                        gameMusic.play();
                        applyLoadedUserData();
                        menu.setState(MenuManager::GAME_ACTIVE);
                    }
                }
            }

            menu.updateMenu(window, uiView);
            menuMusic.setVolume(menu.getCurrentUser().musicVolume);

            window.clear(sf::Color(15, 15, 25));
            menu.draw(window, uiView); 
            window.display();

            continue;

        }
        else {
            processEvents();
            if (!isHeroDead) {
                update(time);
            }
            render();
        }
    }
}

void GameManager::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();

        sf::Vector2f m = window.mapPixelToCoords(sf::Mouse::getPosition(window), uiView);

        if (isHeroDead) {
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                if (sf::FloatRect(300.f, 180.f, 200.f, 30.f).contains(m)) {

                    resetGameSession();

                    isHeroDead = false;
                }
                if (sf::FloatRect(300.f, 240.f, 200.f, 30.f).contains(m)) {
                }
            }
        }

        if (isGamePaused) {
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);

                sf::Vector2f mUI = window.mapPixelToCoords(mousePos, uiView);
                if (isSoundMenuOpen) {
                    if (isButtonClicked(mUI, sf::Vector2f(300.f, 140.f), sf::Vector2f(200.f, 25.f))) {
                        menu.getCurrentUser().musicVolume = handleSliderLogic(mUI, sf::Vector2f(300.f, 165.f), 200.f);
                        menuMusic.setVolume(menu.getCurrentUser().musicVolume);
                        gameMusic.setVolume(menu.getCurrentUser().musicVolume);
                        menu.saveUserToBinary();
                    }
                    else if (isButtonClicked(mUI, sf::Vector2f(300.f, 190.f), sf::Vector2f(200.f, 25.f))) {
                        menu.getCurrentUser().soundVolume = handleSliderLogic(mUI, sf::Vector2f(300.f, 215.f), 200.f);
                        shootSound.setVolume(menu.getCurrentUser().soundVolume);
                        hitSound.setVolume(menu.getCurrentUser().soundVolume);
                        menu.saveUserToBinary();
                    }
                    else if (isButtonClicked(mUI, sf::Vector2f(300.f, 270.f), sf::Vector2f(200.f, 30.f))) {
                        isSoundMenuOpen = false;
                    }
                }

                else {
                    if (isButtonClicked(mUI, sf::Vector2f(300.f, 130.f), sf::Vector2f(200.f, 30.f))) {
                        isSoundMenuOpen = true;
                    }
                    else if (isButtonClicked(mUI, sf::Vector2f(300.f, 180.f), sf::Vector2f(200.f, 30.f))) {
                        saveCurrentProgress(story.currentScene);
                        gameMusic.stop();
                        menuMusic.setVolume(menu.getCurrentUser().musicVolume);
                        menuMusic.play();
                        menu.setState(MenuManager::MAIN_MENU);
                        isGamePaused = false;
                    }
                    else if (isButtonClicked(mUI, sf::Vector2f(300.f, 240.f), sf::Vector2f(200.f, 30.f))) {
                        isGamePaused = false;
                    }
                }
            }
            continue;
        }


        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            if (sf::FloatRect(700.f, 10.f, 90.f, 25.f).contains(m)) {
                isGamePaused = true;
                continue;
            }
        }

        if (event.type == sf::Event::Resized) {
            float windowRatio = (float)event.size.width / (float)event.size.height;
            float viewRatio = 800.f / 400.f;
            sf::FloatRect viewport(0.f, 0.f, 1.f, 1.f);
            if (windowRatio > viewRatio) {
                float width = viewRatio / windowRatio;
                viewport.left = (1.f - width) / 2.f;
                viewport.width = width;
            }
            else {
                float height = windowRatio / viewRatio;
                viewport.top = (1.f - height) / 2.f;
                viewport.height = height;
            }
            gameView.setViewport(viewport);
            uiView.setViewport(viewport);
            window.setView(gameView);
        }

        if (story.currentScene == 4 && techScene.isCodeInputActive) {
            techScene.handleTextInMinigame(event);
        }

        if (showInventory && event.type == sf::Event::MouseWheelScrolled) {
            if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
                hero.inventory.handleScroll(event.mouseWheelScroll.delta);
            }
        }

        if (!dialogue.isOpen && !showInventory && !elevatorScene.isMinigameActive && !techScene.isCodeInputActive) {
            hero.handleInput(event);
        }

        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::I && !dialogue.isOpen && !elevatorScene.isMinigameActive && !techScene.isCodeInputActive) {
                showInventory = !showInventory;
            }

            if (showInventory && event.key.code == sf::Keyboard::Enter) {
                std::string selectedItem = hero.inventory.getSelectedItemName();
                if (hero.inventory.items[selectedItem] > 0) {
                    if (selectedItem == "Medkit") {
                        if (hero.stats.health >= 100.f) {
                            hero.showMessage(L"Çäîðîâüå óæå íà ìàêñèìóìå!", sf::Color::Red);
                        }
                        else {
                            if (menu.getCurrentUser().difficultySetting == 2) {
                                usedMedkitOnHard = true;
                            }
                            hero.inventory.items["Medkit"]--;
                            hero.stats.health = std::min(100.f, hero.stats.health + 30.f);
                            hero.health = hero.stats.health;
                            hero.showMessage(L"Èñïîëüçîâàíà àïòå÷êà", sf::Color::Green);
                        }
                    }
                    else if (selectedItem == "Note") {
                        showInventory = false;
                        dialogue.startDialogue(dialogueDb.getDialogue("apartment_note"));
                    }
                    else if (selectedItem == "Note2") {
                        showInventory = false;
                        dialogue.startDialogue(dialogueDb.getDialogue("hallway_note_uncle_tolya"));
                    }
                    else if (selectedItem == "PDA") {
                        showInventory = false;
                        dialogue.startDialogue(dialogueDb.getDialogue("tech_pda_found"));
                    }
                }
            }

            if (story.currentScene == 6 && serverScene.isEndingSelectionActive && !dialogue.isOpen) {
                if (event.key.code == sf::Keyboard::Num1) {
                    int setting = menu.getCurrentUser().difficultySetting;
                    bool hasAllNotes = (hero.inventory.items["Note"] > 0 && hero.inventory.items["Note2"] > 0);

                    if (hasAllNotes) {
                        if (setting == 0) menu.unlockAchievement(0);
                        else if (setting == 1) {
                            menu.unlockAchievement(1);
                            triggerAchievementNotification(1);
                        }
                        else if (setting == 2) {
                            menu.unlockAchievement(2);
                            triggerAchievementNotification(2);
                        }
                    }

                    if (!failedCloseRange) {
                        menu.unlockAchievement(3);
                        triggerAchievementNotification(3);
                    }

                    if (setting == 2 && !usedMedkitOnHard) {
                        menu.unlockAchievement(5);
                        triggerAchievementNotification(5);
                    }

                    int misses = totalShots - totalHits;
                    if (misses <= 5 && totalShots > 0) {
                        menu.unlockAchievement(6);
                        triggerAchievementNotification(6);
                    }

                    serverScene.isEndingSelectionActive = false;
                    serverScene.selectedEnding = 1;
                    serverScene.bgSprite.setTexture(tunnelTex, true);
                    serverScene.bgSprite.setScale(800.f / tunnelTex.getSize().x, 400.f / tunnelTex.getSize().y);
                    serverScene.bgSprite.setPosition(0.f, 0.f);
                    gameView.setCenter(400.f, 200.f);
                    window.setView(gameView);
                    hero.sprite.setPosition(150.f, 110.f);
                    serverScene.mark.setPosition(70.f, 275.f);
                    hero.faceRight = true;
                    serverScene.mark.staticSprite.setScale(-std::abs(serverScene.mark.staticSprite.getScale().x), serverScene.mark.staticSprite.getScale().y);
                    dialogue.startDialogue(dialogueDb.getDialogue("ending_good_text"));
                }
                if (event.key.code == sf::Keyboard::Num2) {
                    int setting = menu.getCurrentUser().difficultySetting;
                    bool hasAllNotes = (hero.inventory.items["Note"] > 0 && hero.inventory.items["Note2"] > 0);

                    if (hasAllNotes) {
                        if (setting == 0) menu.unlockAchievement(0);
                        else if (setting == 1) {
                            menu.unlockAchievement(1);
                            triggerAchievementNotification(1);
                        }
                        else if (setting == 2) {
                            menu.unlockAchievement(2);
                            triggerAchievementNotification(2);
                        }
                    }

                    if (!failedCloseRange) {
                        menu.unlockAchievement(3);
                        triggerAchievementNotification(3);
                    }

                    if (setting == 2 && !usedMedkitOnHard) {
                        menu.unlockAchievement(5);
                        triggerAchievementNotification(5);
                    }

                    int misses = totalShots - totalHits;
                    if (misses <= 5 && totalShots > 0) {
                        menu.unlockAchievement(6);
                        triggerAchievementNotification(6);
                    }

                    serverScene.isEndingSelectionActive = false;
                    serverScene.selectedEnding = 2;
                    dialogue.startDialogue(dialogueDb.getDialogue("ending_bad_text"));
                }
            }


                if (event.key.code == sf::Keyboard::E) {
                    if (dialogue.isOpen) {
                        if (dialogue.isPrinting()) {
                            dialogue.forceComplete();
                        }
                        else {
                            dialogue.nextLine();
                            if (!dialogue.isOpen) {
                                if (story.currentScene == 6 && serverScene.selectedEnding == 1) {
                                    isHeroDead = true;
                                    isGamePassed = true;
                                    isGamePaused = false;
                                }
                                if (story.currentScene == 6 && serverScene.selectedEnding == 2 && serverScene.mark.getPosition().x == -500.f) {
                                    isHeroDead = true;
                                    isGamePassed = true;
                                    isGamePaused = false;
                                }

                                if (story.currentScene == 1 && story.talkedToMarkStart && pendingMedkitMessage) {
                                    hero.showMessage(L"ÏÎËÓ×ÅÍÎ: ÀÏÒÅ×ÊÀ ÌÀÐÊÀ", sf::Color::Green);
                                    story.markMovingToExit = true;
                                    pendingMedkitMessage = false;
                                }
                                if (story.currentScene == 1 && hero.inventory.items["Ammo"] > 0 && pendingGlockMessage) {
                                    hero.showMessage(L"ÏÎËÓ×ÅÍÎ: ÃËÎÊ-17 (15 ïàòðîíîâ)", sf::Color::Green);
                                    pendingGlockMessage = false;
                                }
                                if (story.currentScene == 2 && !story.hallwayIntroPlayed) {
                                    story.hallwayIntroPlayed = true;
                                }

                            }
                        }
                    }

                    else if (!showInventory) {
                        if (story.currentScene == 1) {
                            if (apartmentScene.nearMark && story.readLaptopEmail && !story.talkedToMarkStart) {
                                pendingMedkitMessage = true;
                            }
                            if (apartmentScene.nearCloset && story.talkedToMarkStart && hero.inventory.items["Ammo"] == 0) {
                                pendingGlockMessage = true;
                            }
                            apartmentScene.handleInteraction(hero, story, dialogue, dialogueDb);
                            saveCurrentProgress(1);
                        }
                        else if (story.currentScene == 2) {
                            if (hallwayScene.nearCloset) {
                                if (hero.inventory.items["Keys"] > 0) {
                                    story.currentScene = 3;
                                    elevatorScene.init();
                                    hero.inventory.loadItemTexture("Laptop", "icons/laptop.png");
                                    hero.inventory.loadItemTexture("Note", "icons/note.png");
                                    hero.inventory.loadItemTexture("Note2", "icons/note_tolya.png");
                                    hero.inventory.loadItemTexture("PDA", "icons/pda.png");
                                    hero.inventory.loadItemTexture("Medkit", "icons/medkit.png");
                                    hero.inventory.loadItemTexture("Ammo", "icons/ammo.png");
                                    hero.inventory.loadItemTexture("Keys", "icons/ammo.png");
                                    hero.sprite.setPosition(1450.f, 210.f);

                                    gameView.setCenter(1200.f, 200.f);
                                    window.setView(gameView);
                                    saveCurrentProgress(2);
                                    return;
                                }
                                else {
                                    hero.showMessage(L"Äâåðü çàïåðòà. Íóæíà ñâÿçêà êëþ÷åé ýëåêòðèêà.", sf::Color::Red);
                                }
                            }
                            hallwayScene.handleInteraction(hero, story, dialogue, dialogueDb);
                        }
                        else if (story.currentScene == 3) {
                            if (elevatorScene.nearElevator && elevatorScene.hackSuccess) {
                                story.currentScene = 4;
                                techScene.init();
                                hero.sprite.setPosition(80.f, 210.f);
                                gameView.setCenter(400.f, 200.f);
                                window.setView(gameView);
                                saveCurrentProgress(3);
                                return;
                            }
                            elevatorScene.handleInteraction(hero, story, dialogue, dialogueDb);
                        }
                        else if (story.currentScene == 4) {
                            if (techScene.nearValve && techScene.gasCleared && !dialogue.isOpen) {
                                story.currentScene = 5;
                                medScene.init();
                                hero.sprite.setPosition(80.f, 210.f);
                                gameView.setCenter(400.f, 200.f);
                                window.setView(gameView);
                                saveCurrentProgress(4);
                                return;
                            }
                            techScene.handleInteraction(hero, story, dialogue, dialogueDb);
                        }
                        else if (story.currentScene == 5) {
                            if (medScene.nearSwitch && medScene.quarantineBypassed && !dialogue.isOpen) {
                                story.currentScene = 6;
                                serverScene.init();
                                hero.sprite.setPosition(80.f, 210.f);
                                gameView.setCenter(400.f, 200.f);
                                window.setView(gameView);
                                saveCurrentProgress(5);
                                return;
                            }
                            medScene.handleInteraction(hero, story, dialogue, dialogueDb);
                        }
                        else if (story.currentScene == 6) {
                            if (serverScene.nearTerminal && serverScene.bossDefeated && !serverScene.dataDownloaded) {
                                serverScene.dataDownloaded = true;
                                dialogue.startDialogue(dialogueDb.getDialogue("lab_documents"));
                                saveCurrentProgress(6);
                            }
                        }
                    }
                }
            }
        }
    }




void GameManager::update(float time) {
    if (isGamePaused && isSoundMenuOpen) {
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {

            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            sf::Vector2f mUI = window.mapPixelToCoords(mousePos, uiView);

            if (isButtonClicked(mUI, sf::Vector2f(300.f, 140.f), sf::Vector2f(200.f, 25.f))) {
                menu.getCurrentUser().musicVolume = handleSliderLogic(mUI, sf::Vector2f(300.f, 165.f), 200.f);
                menuMusic.setVolume(menu.getCurrentUser().musicVolume);
                gameMusic.setVolume(menu.getCurrentUser().musicVolume);
                menu.saveUserToBinary();
            }
            else if (isButtonClicked(mUI, sf::Vector2f(300.f, 190.f), sf::Vector2f(200.f, 25.f))) {
                menu.getCurrentUser().soundVolume = handleSliderLogic(mUI, sf::Vector2f(300.f, 215.f), 200.f);
                shootSound.setVolume(menu.getCurrentUser().soundVolume);
                hitSound.setVolume(menu.getCurrentUser().soundVolume);
                menu.saveUserToBinary();
            }
        }
    }

    if (hero.stats.health <= 0.f) {
        isHeroDead = true;
        isGamePaused = false;
        return;
    }
    if (hero.stats.health < 15.f && hero.stats.health > 0.f) {
        if (!menu.getCurrentUser().achievements[4]) {
            menu.unlockAchievement(4);
            triggerAchievementNotification(4);
        }
    }

    if (isHeroDead) return;

    float diffMod = 1.0f;
    int currentSetting = menu.getCurrentUser().difficultySetting;
    switch (currentSetting) {
    case 0: diffMod = 0.5f; break;
    case 1: diffMod = 1.0f; break;
    case 2: diffMod = 1.6f; break;
    default: diffMod = 1.0f; break;
    }


    if (!isGamePaused && !dialogue.isOpen && !showInventory) {
        hero.update(time);
    }

    if (story.currentScene == 6 && serverScene.selectedEnding == 1) {
        hero.sprite.setPosition(150.f, 110.f);
    }



    dialogue.update(time);


    if (story.currentScene == 1) {
        apartmentScene.update(time, hero, story);
        syncGameCamera(400.f, 1184.f);
        float playerX = hero.sprite.getPosition().x;
        if (playerX < 15.f) {
            if (story.talkedToMarkStart && hero.inventory.items["Ammo"] > 0) {
                hallwayScene.init();
                story.currentScene = 2;
                story.hallwayIntroPlayed = false;

                hero.sprite.setPosition(2300.f, 210.f);
                gameView.setCenter(2000.f, 200.f);
                window.setView(gameView);

                questText.setString(story.getCurrentQuestText());
                sf::FloatRect textBounds = questText.getLocalBounds();
                questText.setOrigin(textBounds.left + textBounds.width / 2.f, textBounds.top + textBounds.height / 2.f);
                questText.setPosition(400.f, 31.f);
                dialogue.startDialogue(dialogueDb.getDialogue("hallway_intro"));
            }
            else {
                hero.sprite.setPosition(25.f, 210.f);
                if (!dialogue.isOpen) {
                    hero.showMessage(L"ß íå âûéäó òóäà áåç îðóæèÿ. Íóæíî îòêðûòü øêàô Ìàðêà.", sf::Color::Red);
                }
            }
        }
    }

    else if (story.currentScene == 2) {
        if (!hallwayScene.isLoaded) {
            hallwayScene.init();
            dialogue.startDialogue(dialogueDb.getDialogue("hallway_intro"));
            gameView.setCenter(2000.f, 200.f);
            window.setView(gameView);
            return;
        }

        if (!dialogue.isOpen && !story.hallwayIntroPlayed) {
            story.hallwayIntroPlayed = true;
        }

        float playerX = hero.sprite.getPosition().x;

        if (!hallwayScene.isAmbushTriggered && playerX <= 1250.f && !dialogue.isOpen && story.hallwayIntroPlayed) {
            hallwayScene.isAmbushTriggered = true;
            hallwayScene.zombieAmbush.setPosition(2380.f, 385.f);
            dialogue.startDialogue(dialogueDb.getDialogue("hallway_ambush_warning"));
        }

        hallwayScene.updateDistances(playerX, dialogue.isOpen, story, hero.inventory.items["Keys"]);
        hallwayScene.update(time, hero, story, dialogue, dialogueDb);

        if (story.hallwayIntroPlayed && !dialogue.isOpen) {
            if (playerX > 1800.f) {
                syncGameCamera(2000.f, 2000.f);
            }
            else {
                syncGameCamera(400.f, 2000.f);
            }
        }
        else {
            window.setView(gameView);
        }

        float cameraX = gameView.getCenter().x;
        if (story.hallwayIntroPlayed && !dialogue.isOpen) {
            hallwayScene.zombie.checkPlayerCollision(hero, 12.f, diffMod, dialogue.isOpen, hitSound);
        }
        if (hallwayScene.isAmbushTriggered) {
            hallwayScene.zombieAmbush.checkPlayerCollision(hero, 15.f, diffMod, dialogue.isOpen, hitSound);
        }
        spawnPlayerBullet();
        checkBulletCollisions(time, hallwayScene.zombie, cameraX, L"ÏÎÏÀÄÀÍÈÅ!", sf::Color::Yellow);
        checkBulletCollisions(time, hallwayScene.zombieAmbush, cameraX, L"ÊÐÈÒ ÊÓÐÜÅÐÓ!", sf::Color::Red);
    }


    else if (story.currentScene == 3) {
        if (!elevatorScene.isLoaded) {
            elevatorScene.init();
        }
        elevatorScene.update(time, hero, story, dialogue, dialogueDb);

        syncGameCamera(400.f, 1200.f);
        float cameraX = gameView.getCenter().x;

        if (elevatorScene.isBossSpawned && elevatorScene.bossHealth > 0 && !dialogue.isOpen) {
            if (hero.sprite.getGlobalBounds().intersects(elevatorScene.bossSprite.getGlobalBounds())) {
                if (hero.invulTimer <= 0.f) {
                    hero.stats.health -= 25.f;
                    hero.health = hero.stats.health;
                    hitSound.setVolume(menu.getCurrentUser().soundVolume);
                    hitSound.play();
                    hero.showMessage(L"ÂAÑ ÓÄAÐÈËÈ!", sf::Color::Red);
                    hero.invulTimer = 50.f;
                }
                float bossX = elevatorScene.bossSprite.getPosition().x;
                float bossY = elevatorScene.bossSprite.getPosition().y;
                float playerX = hero.sprite.getPosition().x;
                if (bossX > playerX) {
                    elevatorScene.bossSprite.setPosition(bossX + 110.f, bossY);
                }
                else {
                    elevatorScene.bossSprite.setPosition(bossX - 110.f, bossY);
                }
            }
        }
        spawnPlayerBullet();
        for (size_t i = 0; i < activeBullets.size();) {
            activeBullets[i].update(time);
            bool hit = false;
            if (elevatorScene.isBossSpawned && elevatorScene.bossHealth > 0) {
                if (activeBullets[i].sprite.getGlobalBounds().intersects(elevatorScene.bossSprite.getGlobalBounds())) {
                    elevatorScene.bossHealth -= 20.f;
                    hero.showMessage(L"ÏÎÏÀÄÀÍÈÅ Â ÌÓÒÀÍÒÀ!", sf::Color::Yellow);
                    hit = true;
                }
            }
            float bX = activeBullets[i].sprite.getPosition().x;
            if (hit || bX < cameraX - 450.f || bX > cameraX + 450.f) {
                activeBullets.erase(activeBullets.begin() + i);
            }
            else {
                i++;
            }
        }
    }


    else if (story.currentScene == 4) {
        if (!techClockStarted) {
            techSceneClock.restart();
            techClockStarted = true;
        }
        if (!techScene.isLoaded) {
            techScene.init();
        }
        techScene.update(time, hero, story, dialogue, dialogueDb);

        syncGameCamera(400.f, 1200.f);

        if (techScene.nearValve && techScene.gasCleared && !dialogue.isOpen) {
            if (techSceneClock.getElapsedTime().asSeconds() <= 15.f) {
                if (!menu.getCurrentUser().achievements[7]) {
                    menu.unlockAchievement(7);
                    triggerAchievementNotification(7);
                }
            }
            story.currentScene = 5;
            medScene.init();
            hero.sprite.setPosition(80.f, 210.f);
            gameView.setCenter(400.f, 200.f);
            window.setView(gameView);
            saveCurrentProgress(4);
            return;
        }
        techScene.handleInteraction(hero, story, dialogue, dialogueDb);
        }


    else if (story.currentScene == 5) {
        if (!medScene.isLoaded) {
            medScene.init();
        }
        medScene.update(time, hero, story, dialogue, dialogueDb);

        syncGameCamera(400.f, 1200.f);
        float cameraX = gameView.getCenter().x;

        medScene.zombie1.checkPlayerCollision(hero, 12.f, diffMod, !medScene.defenseActive || medScene.quarantineBypassed || dialogue.isOpen, hitSound);
        medScene.zombie2.checkPlayerCollision(hero, 12.f, diffMod, !medScene.defenseActive || medScene.quarantineBypassed || dialogue.isOpen, hitSound);
        spawnPlayerBullet();
        checkBulletCollisions(time, medScene.zombie1, cameraX, L"ÓÐÎÍ ÏÎ ÇÀÐÀÆÅÍÍÎÌÓ ÂÐÀ×Ó!", sf::Color::Yellow);
        checkBulletCollisions(time, medScene.zombie2, cameraX, L"ÓÐÎÍ ÏÎ ÇÀÐÀÆÅÍÍÎÌÓ ÂÐÀ×Ó!", sf::Color::Yellow);
        }


    else if (story.currentScene == 6) {
            if (!serverScene.isLoaded) serverScene.init();
            serverScene.update(time, hero, story, dialogue, dialogueDb);

            if (serverScene.selectedEnding == 1) {
                gameView.setCenter(400.f, 200.f);
            }
            else {
                syncGameCamera(400.f, 1200.f);
            }

            serverScene.finalBoss.checkPlayerCollision(hero, 20.f, diffMod, !serverScene.bossSpawned || serverScene.bossDefeated || dialogue.isOpen, hitSound);
            spawnPlayerBullet();
            checkBulletCollisions(time, serverScene.finalBoss, gameView.getCenter().x, L"ÊÐÈÒ ÏÎ ÎÁÚÅÊÒÓ-00!", sf::Color::Red);
            }

    questText.setString(story.getCurrentQuestText());
    sf::FloatRect textBounds = questText.getLocalBounds();
    questText.setOrigin(textBounds.left + textBounds.width / 2.f, textBounds.top + textBounds.height / 2.f);
    questText.setPosition(400.f, 31.f);
}
void GameManager::render() {

        window.clear(sf::Color::Black);


        if (menu.getState() != MenuManager::GAME_ACTIVE) {
            window.setView(uiView);
            menu.draw(window, uiView);
            window.display();
            return;
        }

        window.setView(gameView);
        if (story.currentScene == 1) {
            apartmentScene.draw(window);
        }
    else if (story.currentScene == 2) {
        hallwayScene.draw(window);
        for (size_t i = 0; i < activeBullets.size(); i++) {
            activeBullets[i].draw(window);
        }
        if (hallwayScene.zombie.health > 0 && story.hallwayIntroPlayed) {
            float zX = hallwayScene.zombie.getPosition().x;
            float zY = hallwayScene.zombie.getPosition().y - 195.f;
            zombieHPBar.update(hallwayScene.zombie.health, 60.f, sf::Vector2f(zX, zY));
            zombieHPBar.draw(window);
        }
        if (hallwayScene.isAmbushTriggered && hallwayScene.zombieAmbush.health > 0) {
            float aX = hallwayScene.zombieAmbush.getPosition().x;
            float aY = hallwayScene.zombieAmbush.getPosition().y - 195.f;
            zombieHPBar.update(hallwayScene.zombieAmbush.health, 40.f, sf::Vector2f(aX, aY));
            zombieHPBar.draw(window);
        }
    }
    else if (story.currentScene == 3) {
        elevatorScene.draw(window);
        for (size_t i = 0; i < activeBullets.size(); i++) {
            activeBullets[i].draw(window);
        }
        if (elevatorScene.isBossSpawned && elevatorScene.bossHealth > 0) {
            float bX = elevatorScene.bossSprite.getPosition().x;
            float bY = elevatorScene.bossSprite.getPosition().y - 130.f;
            zombieHPBar.update(elevatorScene.bossHealth, 100.f, sf::Vector2f(bX - 30.f, bY));
            zombieHPBar.draw(window);
        }
    }
    else if (story.currentScene == 4) {
        techScene.draw(window);
    }
    else if (story.currentScene == 5) {
        medScene.draw(window);
        for (size_t i = 0; i < activeBullets.size(); i++) {
            activeBullets[i].draw(window);
        }
        if (medScene.defenseActive) {
            if (medScene.zombie1.health > 0) {
                float zX = medScene.zombie1.getPosition().x;
                float zY = medScene.zombie1.getPosition().y - 195.f;
                zombieHPBar.update(medScene.zombie1.health, 50.f, sf::Vector2f(zX, zY));
                zombieHPBar.draw(window);
            }
            if (medScene.zombie2.health > 0) {
                float zX = medScene.zombie2.getPosition().x;
                float zY = medScene.zombie2.getPosition().y - 195.f;
                zombieHPBar.update(medScene.zombie2.health, 50.f, sf::Vector2f(zX, zY));
                zombieHPBar.draw(window);
            }
        }
    }
    else if (story.currentScene == 6) {
        serverScene.draw(window);
        for (size_t i = 0; i < activeBullets.size(); i++) {
            activeBullets[i].draw(window);
        }
        if (serverScene.bossSpawned && !serverScene.bossDefeated && serverScene.selectedEnding == 0) {
            float bX = serverScene.finalBoss.getPosition().x;
            float bY = serverScene.finalBoss.getPosition().y - 195.f;
            zombieHPBar.update(serverScene.finalBoss.health, 300.f, sf::Vector2f(bX - 30.f, bY));
            zombieHPBar.draw(window);
        }
    }
    hero.draw(window);
    hero.drawMessage(window);
    window.setView(uiView);
    playerHPBar.update(hero.stats.health, 100.f, sf::Vector2f(20.f, 20.f));
    playerHPBar.draw(window);
    window.setView(uiView);
    playerHPBar.update(hero.stats.health, 100.f, sf::Vector2f(20.f, 20.f));
    playerHPBar.draw(window);

    sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
    sf::Vector2f mPos = window.mapPixelToCoords(pixelPos, uiView);


    auto drawInterfaceBtn = [&](sf::Vector2f pos, sf::Vector2f size, const std::wstring& text, bool isH) {
        sf::RectangleShape b(size); b.setPosition(pos);
        b.setFillColor(isH ? sf::Color(100, 30, 180) : sf::Color(45, 15, 75));
        b.setOutlineColor(sf::Color(255, 215, 0)); b.setOutlineThickness(1.5f);
        window.draw(b);
        sf::Text t(text, questFont, 11); t.setPosition(pos.x + 15.f, pos.y + 6.f);
        window.draw(t);
        };

    if (!isHeroDead && !isGamePaused) {
        drawInterfaceBtn(sf::Vector2f(700.f, 10.f), sf::Vector2f(90.f, 25.f), L"Íàñòðîéêè", sf::FloatRect(700.f, 10.f, 90.f, 25.f).contains(mPos));
    }

    if (isGamePaused) {
        window.setView(uiView);
        sf::RectangleShape dim(sf::Vector2f(800.f, 400.f));
        dim.setFillColor(sf::Color(0, 0, 0, 180));
        window.draw(dim);

        if (isSoundMenuOpen) {
            sf::Text pTitle(L"Íàñòðîéêè çâóêà", questFont, 16);
            pTitle.setPosition(310.f, 50.f);
            window.draw(pTitle);

            drawVolumeSlider(L"Ãðîìêîñòü ìóçûêè", menu.getCurrentUser().musicVolume, sf::Vector2f(300.f, 140.f));
            drawVolumeSlider(L"Ãðîìêîñòü çâóêîâ", menu.getCurrentUser().soundVolume, sf::Vector2f(300.f, 190.f));

            drawInterfaceBtn(sf::Vector2f(300.f, 270.f), sf::Vector2f(200.f, 30.f), L"      Íàçàä", isButtonClicked(mPos, sf::Vector2f(300.f, 270.f), sf::Vector2f(200.f, 30.f)));
        }
        else {
            drawInterfaceBtn(sf::Vector2f(300.f, 130.f), sf::Vector2f(200.f, 30.f), L" Íàñòðîéêè çâóêà", isButtonClicked(mPos, sf::Vector2f(300.f, 130.f), sf::Vector2f(200.f, 30.f)));
            drawInterfaceBtn(sf::Vector2f(300.f, 180.f), sf::Vector2f(200.f, 30.f), L"Ñîõðàíèòü è âûéòè", isButtonClicked(mPos, sf::Vector2f(300.f, 180.f), sf::Vector2f(200.f, 30.f)));
            drawInterfaceBtn(sf::Vector2f(300.f, 240.f), sf::Vector2f(200.f, 30.f), L" Âåðíóòüñÿ ê èãðå", isButtonClicked(mPos, sf::Vector2f(300.f, 240.f), sf::Vector2f(200.f, 30.f)));
        }
    }

    if (isHeroDead) {
        sf::RectangleShape deadDim(sf::Vector2f(800.f, 400.f));
        deadDim.setFillColor(isGamePassed ? sf::Color(0, 35, 0, 230) : sf::Color(40, 0, 0, 230));
        window.draw(deadDim);

        sf::Text dTitle;
        dTitle.setFont(questFont);
        dTitle.setCharacterSize(24);

        if (isGamePassed) {
            dTitle.setString(L"ÂÛ ÏÐÎØËÈ ÈÃÐÓ!");
            dTitle.setFillColor(sf::Color::Green);
            dTitle.setPosition(275.f, 60.f);
        }
        else {
            dTitle.setString(L"ÂÛ ÓÌÅÐËÈ");
            dTitle.setFillColor(sf::Color::Red);
            dTitle.setPosition(320.f, 60.f);
        }
        window.draw(dTitle);

        drawInterfaceBtn(sf::Vector2f(300.f, 180.f), sf::Vector2f(200.f, 30.f), L" Íà÷àòü çàíîâî", sf::FloatRect(300.f, 180.f, 200.f, 30.f).contains(mPos));
        drawInterfaceBtn(sf::Vector2f(300.f, 240.f), sf::Vector2f(200.f, 30.f), L"Âûéòè â ãëàâíîå ìåíþ", sf::FloatRect(300.f, 240.f, 200.f, 30.f).contains(mPos));
    }


    if (story.currentScene != 6 || serverScene.selectedEnding == 0) {
        window.draw(questBoxSprite);
        window.draw(questText);
    }
    if (showInventory) {
        hero.inventory.drawUI(window, questFont);
    }
    if (dialogue.isOpen) {
        dialogue.draw(window);
    }
    if (showAchievementPopup && achievementPopupTimer > 0.f) {
        achievementPopupTimer -= 1.0f; 


        sf::RectangleShape popBox(sf::Vector2f(220.f, 24.f));
        popBox.setPosition(560.f, 15.f); 
        popBox.setFillColor(sf::Color(20, 20, 30, 230));
        popBox.setOutlineThickness(1.f);
        popBox.setOutlineColor(sf::Color(255, 215, 0)); 
        window.draw(popBox);

        achievementPopupText.setPosition(570.f, 20.f);
        window.draw(achievementPopupText);

        if (achievementPopupTimer <= 0.f) {
            showAchievementPopup = false;
        }
    }
    if (!isGamePaused && menu.getState() == MenuManager::GAME_ACTIVE) {
        window.setView(gameView);
    }

    window.display(); 

}
void GameManager::spawnPlayerBullet() {
    static bool bulletSpawned = false;

    if (hero.isShooting) {
        if (!bulletSpawned) {
            sf::Vector2f spawnPos = hero.sprite.getPosition();
            spawnPos.y += 65.f;
            if (hero.faceRight) spawnPos.x += 60.f;
            else spawnPos.x -= 60.f;

            Bullet b(bulletTex, spawnPos, hero.faceRight);
            activeBullets.push_back(b);
            shootSound.setVolume(menu.getCurrentUser().soundVolume);
            shootSound.play();
            bulletSpawned = true;
        }
    }
    else {
        bulletSpawned = false;
    }
}
void GameManager::triggerAchievementNotification(int index) {
    std::wstring names[8] = {
        L"ÄÎÑÒÈÆÅÍÈÅ: Êîëëåêöèîíåð",
        L"ÄÎÑÒÈÆÅÍÈÅ: Àðõèâàðèóñ",
        L"ÄÎÑÒÈÆÅÍÈÅ: Èñòîðèê áóíêåðà",
        L"ÄÎÑÒÈÆÅÍÈÅ: Â óïîð!",
        L"ÄÎÑÒÈÆÅÍÈÅ: Íà âîëîñêå",
        L"ÄÎÑÒÈÆÅÍÈÅ: Ìåäèöèíñêèé àñêåòèçì",
        L"ÄÎÑÒÈÆÅÍÈÅ: Ñíàéïåð ñåêòîðà",
        L"ÄÎÑÒÈÆÅÍÈÅ: Ñïðèíòåð"
    };

    if (index >= 0 && index < 8) {
        achievementPopupText.setString(names[index]);
        achievementPopupTimer = 180.f;
        showAchievementPopup = true;
    }
}

bool GameManager::isButtonClicked(sf::Vector2f mousePos, sf::Vector2f btnPos, sf::Vector2f btnSize) {
    return sf::FloatRect(btnPos, btnSize).contains(mousePos);
}

float GameManager::handleSliderLogic(sf::Vector2f mousePos, sf::Vector2f trackPos, float trackWidth) {
    float relativeX = mousePos.x - trackPos.x;
    float percentage = (relativeX / trackWidth) * 100.f;
    return std::max(0.f, std::min(100.f, percentage));
}

void GameManager::drawVolumeSlider(const std::wstring& title, float volume, sf::Vector2f pos) {
    sf::Text text(title + L": " + std::to_wstring(int(volume)) + L"%", questFont, 11);
    text.setPosition(pos.x, pos.y);
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
void GameManager::resetGameSession() {
    story.currentScene = 1;
    hero.stats.health = 100.f;
    hero.health = 100.f;
    hero.inventory.items.clear();
    hero.inventory.addItem("Laptop", 1);
    hero.sprite.setPosition(1150.f, 210.f);
    gameView.setCenter(1150.f, 200.f);
    isGamePassed = false;
    story.talkedToMarkStart = false;
    story.readLaptopEmail = false;
    story.markMovingToExit = false;
    story.hallwayIntroPlayed = false;
    story.noteRead = false;
    apartmentScene.mark.init("npc_sprite.png", "mark_move.png", sf::Vector2f(1050.f, 385.f));
    hallwayScene.isLoaded = false;
    hallwayScene.noteRead = false;
    hallwayScene.isKeyPickedUp = false;
    hallwayScene.isAmbushTriggered = false;
    hallwayScene.init();
    elevatorScene.isLoaded = false;
    elevatorScene.isMinigameActive = false;
    elevatorScene.hackSuccess = false;
    elevatorScene.isBossSpawned = false;
    elevatorScene.bossHealth = 100.f;
    elevatorScene.init();
    techScene.isLoaded = false;
    techScene.isCodeInputActive = false;
    techScene.gasCleared = false;
    techScene.init();
    medScene.isLoaded = false;
    medScene.defenseActive = false;
    medScene.quarantineBypassed = false;
    medScene.init();
    serverScene.isLoaded = false;
    serverScene.bossSpawned = false;
    serverScene.bossDefeated = false;
    serverScene.dataDownloaded = false;
    serverScene.isEndingSelectionActive = false;
    serverScene.selectedEnding = 0;
    serverScene.init();

    UserData& u = menu.getCurrentUser();
    u.lastScene = 0;
    u.playerHealth = 100.f;
    u.ammoCount = 0;
    u.medkitCount = 0;
    u.keysCount = 0;
    u.laptopCount = 1;
    u.noteCount = 0;
    u.note2Count = 0;
    u.pdaCount = 0;
    menu.saveUserToBinary();
}
void GameManager::applyLoadedUserData() {
    UserData& u = menu.getCurrentUser();
    story.currentScene = u.lastScene;
    hero.stats.health = u.playerHealth;
    hero.health = u.playerHealth;
    hero.inventory.items.clear();

    if (u.laptopCount > 0) hero.inventory.addItem("Laptop", u.laptopCount);
    if (u.ammoCount > 0) hero.inventory.addItem("Ammo", u.ammoCount);
    if (u.medkitCount > 0) hero.inventory.addItem("Medkit", u.medkitCount);
    if (u.keysCount > 0) hero.inventory.addItem("Keys", u.keysCount);
    if (u.noteCount > 0) hero.inventory.addItem("Note", u.noteCount);
    if (u.note2Count > 0) hero.inventory.addItem("Note2", u.note2Count);
    if (u.pdaCount > 0) hero.inventory.addItem("PDA", u.pdaCount);

    switch (story.currentScene) {
    case 1:
        hero.sprite.setPosition(1150.f, 210.f);
        break;
    case 2:
        hero.sprite.setPosition(2300.f, 210.f); 
        break;
    case 3:
        hero.sprite.setPosition(1450.f, 210.f);
        break;
    default:
        hero.sprite.setPosition(80.f, 210.f);
        break;
    }

}


void GameManager::syncGameCamera(float minX, float maxX) {
    float playerX = hero.sprite.getPosition().x;
    float cameraX = std::max(minX, std::min(maxX, playerX));
    gameView.setCenter(cameraX, 200.f);
}

void GameManager::initNewGameSession() {
    resetGameSession();
    story.currentScene = 1;
    hero.stats.health = 100.f;
    hero.health = 100.f;
    hero.sprite.setPosition(1150.f, 210.f);
    gameView.setCenter(1150.f, 200.f);
}
