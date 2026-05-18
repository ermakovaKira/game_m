#include "GameManager.h"
#include "ApartmentScene.h"
#include "HallwayScene.h"
#include "ElevatorScene.h"
#include "TechScene.h"
#include <algorithm>

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

void GameManager::run() {
    while (window.isOpen()) {
        float time = clock.getElapsedTime().asMicroseconds() / 700.0f;
        clock.restart();

        processEvents();
        update(time);
        render();
    }
}

void GameManager::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();

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

        // Ввод текста в мини-игре (Передаем строго 1 аргумент!)
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
                            hero.showMessage(L"Здоровье уже на максимуме!", sf::Color::Red);
                        }
                        else {
                            hero.inventory.items["Medkit"]--;
                            hero.stats.health = std::min(100.f, hero.stats.health + 30.f);
                            hero.health = hero.stats.health;
                            hero.showMessage(L"Использована аптечка", sf::Color::Green);
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

            // ОБРАБОТКА ИНТЕРАКТИВА НА КЛАВИШУ E
            if (event.key.code == sf::Keyboard::E) {
                if (dialogue.isOpen) {
                    if (dialogue.isPrinting()) {
                        dialogue.forceComplete();
                    }
                    else {
                        dialogue.nextLine();

                        if (!dialogue.isOpen) {
                            if (story.currentScene == 1 && story.talkedToMarkStart && pendingMedkitMessage) {
                                hero.showMessage(L"ПОЛУЧЕНО: AПТЕЧКА МАРКА", sf::Color::Green);
                                story.markMovingToExit = true;
                                pendingMedkitMessage = false;
                            }
                            if (story.currentScene == 1 && hero.inventory.items["Ammo"] > 0 && pendingGlockMessage) {
                                hero.showMessage(L"ПОЛУЧЕНО: ГЛОК-17 (15 патронов)", sf::Color::Green);
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
                                return;
                            }
                            else {
                                hero.showMessage(L"Дверь заперта. Нужна связка ключей электрика.", sf::Color::Red);
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
                            return;
                        }
                        elevatorScene.handleInteraction(hero, story, dialogue, dialogueDb);
                    }
                    else if (story.currentScene == 4) {
                        // Если вентиль уже провернут и газ откачан — блокируем повторные клики
                        if (techScene.nearValve && techScene.gasCleared && !dialogue.isOpen) {
                            hero.showMessage(L"АКТ II ЗАВЕРШЕН СЮЖЕТНЫМ ФИНАЛОМ ДЛЯ СЦЕНЫ 4!", sf::Color::Cyan);
                            return;
                        }

                        // Проверяем: если Ева у терминала, код подошел, но рапорт еще не читали — запускаем его!
                        if (techScene.nearTerminal && techScene.terminalBypassed && !dialogue.isOpen) {
                            static bool wasReportPlayed = false;
                            if (!wasReportPlayed) {
                                wasReportPlayed = true;
                                dialogue.startDialogue(dialogueDb.getDialogue("tech_inspect_terminal"));
                                return;
                            }
                        }

                        techScene.handleInteraction(hero, story, dialogue, dialogueDb);
                    }
                }
            }
        }
    }
}

void GameManager::update(float time) {
    if (!dialogue.isOpen && !showInventory) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && !hero.isShooting && !elevatorScene.isMinigameActive && !techScene.isCodeInputActive) {
            if (hero.inventory.items["Ammo"] > 0) {
                hero.inventory.items["Ammo"]--;
                hero.isShooting = true;
                hero.currentFrame = 0.f;
                hero.sprite.setTexture(hero.textureShoot);
                hero.animationClock.restart();
                std::cout << "Выстрел! Патронов осталось: " << hero.inventory.items["Ammo"] << std::endl;
            }
            else if (!wasShootingLastFrame) {
                hero.showMessage(L"НЕТ ПАТРОНОВ!", sf::Color::Red);
            }
            wasShootingLastFrame = true;
        }
        else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
            wasShootingLastFrame = false;
        }

        hero.update(time);
    }
    dialogue.update(time);

    if (story.currentScene == 1) {
        apartmentScene.update(time, hero, story);
        float playerX = hero.sprite.getPosition().x;
        float cameraX = std::max(400.f, std::min(1184.f, playerX));
        gameView.setCenter(cameraX, 200.f);

        if (playerX < 15.f) {
            if (story.talkedToMarkStart && hero.inventory.items["Ammo"] > 0) {
                hallwayScene.init();
                story.currentScene = 2;
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
                    hero.showMessage(L"Я не выйду туда без оружия. Нужно открыть шкаф Марка.", sf::Color::Red);
                }
            }
        }
    }
    else if (story.currentScene == 2) {
        if (!hallwayScene.isLoaded) {
            hallwayScene.init();
        }

        float playerX = hero.sprite.getPosition().x;

        if (!hallwayScene.isAmbushTriggered && playerX <= 1250.f && !dialogue.isOpen) {
            hallwayScene.isAmbushTriggered = true;
            hallwayScene.zombieAmbush.setPosition(2380.f, 385.f);
            dialogue.startDialogue(dialogueDb.getDialogue("hallway_ambush_warning"));
        }

        hallwayScene.update(time, hero, story, dialogue, dialogueDb);

        float cameraX = std::max(400.f, std::min(2000.f, playerX));
        gameView.setCenter(cameraX, 200.f);

        if (hero.isShooting && int(hero.currentFrame) == 0 && activeBullets.empty()) {
            sf::Vector2f spawnPos = hero.sprite.getPosition();
            spawnPos.y += 65.f;
            if (hero.faceRight) spawnPos.x += 60.f;
            else spawnPos.x -= 60.f;

            Bullet b(bulletTex, spawnPos, hero.faceRight);
            activeBullets.push_back(b);
        }

        for (size_t i = 0; i < activeBullets.size();) {
            activeBullets[i].update(time);

            bool hit = false;

            if (hallwayScene.zombie.health > 0) {
                if (activeBullets[i].sprite.getGlobalBounds().intersects(hallwayScene.zombie.getGlobalBounds())) {
                    hallwayScene.zombie.health -= 20.f;
                    hit = true;
                }
            }

            if (!hit && hallwayScene.isAmbushTriggered && hallwayScene.zombieAmbush.health > 0) {
                if (activeBullets[i].sprite.getGlobalBounds().intersects(hallwayScene.zombieAmbush.getGlobalBounds())) {
                    hallwayScene.zombieAmbush.health -= 20.f; // <-- ДОБАВЛЕНО: Теперь урон наносится!
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
    else if (story.currentScene == 3) {
        if (!elevatorScene.isLoaded) {
            elevatorScene.init();
        }
        elevatorScene.update(time, hero, story, dialogue, dialogueDb);

        float playerX = hero.sprite.getPosition().x;
        float cameraX = std::max(400.f, std::min(1200.f, playerX));
        gameView.setCenter(cameraX, 200.f);

        // Спавн новой пули при выстреле на Сцене 3
        if (hero.isShooting && int(hero.currentFrame) == 0 && activeBullets.empty()) {
            sf::Vector2f spawnPos = hero.sprite.getPosition();
            spawnPos.y += 65.f;
            if (hero.faceRight) spawnPos.x += 60.f;
            else spawnPos.x -= 60.f;

            Bullet b(bulletTex, spawnPos, hero.faceRight);
            activeBullets.push_back(b);
        }

        // ИСПРАВЛЕНО: Полный цикл полета пуль и проверки урона по Мутанту-Боссу!
        for (size_t i = 0; i < activeBullets.size();) {
            activeBullets[i].update(time);
            bool hit = false;

            if (elevatorScene.isBossSpawned && elevatorScene.bossHealth > 0) {
                // Сверяем хитбокс пули с хитбоксом спрайта Босса в холле
                if (activeBullets[i].sprite.getGlobalBounds().intersects(elevatorScene.bossSprite.getGlobalBounds())) {
                    elevatorScene.bossHealth -= 20.f; // Наносим честные 20 урона
                    hero.showMessage(L"ПОПАДАНИЕ В МУТАНТА!", sf::Color::Yellow);
                    hit = true;
                }
            }

            float bX = activeBullets[i].sprite.getPosition().x;
            // Удаляем пулю, если она попала в цель или вылетела далеко за края экрана
            if (hit || bX < cameraX - 450.f || bX > cameraX + 450.f) {
                activeBullets.erase(activeBullets.begin() + i);
            }
            else {
                i++;
            }
        }
    }
    else if (story.currentScene == 4) {
        if (!techScene.isLoaded) {
            techScene.init();
        }
        techScene.update(time, hero, story, dialogue, dialogueDb);

        float playerX = hero.sprite.getPosition().x;
        float cameraX = std::max(400.f, std::min(1200.f, playerX));
        gameView.setCenter(cameraX, 200.f);
    }

    if (!dialogue.isOpen) {
        questText.setString(story.getCurrentQuestText());
        sf::FloatRect textBounds = questText.getLocalBounds();
        questText.setOrigin(textBounds.left + textBounds.width / 2.f, textBounds.top + textBounds.height / 2.f);
        questText.setPosition(400.f, 31.f);
    }
}

void GameManager::render() {
    window.clear(sf::Color::Black);

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

    hero.draw(window);
    hero.drawMessage(window);

    window.setView(uiView);

    playerHPBar.update(hero.stats.health, 100.f, sf::Vector2f(20.f, 20.f));
    playerHPBar.draw(window);

    window.draw(questBoxSprite);
    window.draw(questText);

    if (showInventory) {
        hero.inventory.drawUI(window, questFont);
    }

    if (dialogue.isOpen) {
        dialogue.draw(window);
    }

    window.display();
}
