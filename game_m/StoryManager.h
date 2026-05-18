#ifndef STORY_MANAGER_H
#define STORY_MANAGER_H

#pragma once
#include <string>
#include "Config.h"

class StoryManager {
public:
    int currentScene;
    Difficulty gameDifficulty;

    bool noteRead;
    bool readLaptopEmail;
    bool talkedToMarkStart;
    bool markMovingToExit;

    bool hallwayIntroPlayed;

    bool techPdaFound;
    bool techTerminalBypassed;
    bool techGasCleared;

    bool medCardFound;
    bool medQuarantineBypassed;

    bool serverDownloadStarted;
    bool serverDataSecured;

    StoryManager(Difficulty diff) {
        gameDifficulty = diff;
        currentScene = 1;

        noteRead = false;
        readLaptopEmail = false;
        talkedToMarkStart = false;
        markMovingToExit = false;

        hallwayIntroPlayed = false;

        techPdaFound = false;
        techTerminalBypassed = false;
        techGasCleared = false;

        medCardFound = false;
        medQuarantineBypassed = false;

        serverDownloadStarted = false;
        serverDataSecured = false;
    }

    std::wstring getCurrentQuestText() {
        if (currentScene == 1) {
            if (!noteRead) {
                return L"Задание: Осмотреть кухонный стол и найти записку Марка";
            }
            if (!readLaptopEmail) {
                return L"Задание: Взломать брандмауэр и проверить сеть на ПК";
            }
            if (!talkedToMarkStart) {
                return L"Задание: Обсудить план побега с Марком у дивана";
            }
            return L"Задание: Забрать Глок-17 из шкафа в прихожей и выйти наружу";
        }

        if (currentScene == 2) {
            if (!hallwayIntroPlayed) {
                return L"Задание: Осмотреться в темном коридоре подъезда";
            }
            return L"Задание: Найти способ открыть техническую дверь шлюза";
        }

        if (currentScene == 3) {
            return L"Задание: Подключить ноутбук к терминалу и взломать гермозатвор";
        }

        if (currentScene == 4) {
            if (!techPdaFound) {
                return L"Задание: Сектор заблокирован! Срочно обыщите тело техника и найдите КПК";
            }
            if (!techTerminalBypassed) {
                return L"Задание: Откройте инвентарь (I), прочтите лог КПК и введите шифр в настенный терминал";
            }
            if (!techGasCleared) {
                return L"Задание: Электроника перезапущена! Подбегите к резервному клапану трубы и проверните вентиль (E)";
            }
            return L"Задание: Давление сброшено. Газ полностью откачан. Проследуйте к лифту глубокого заложения";
        }

        if (currentScene == 5) {
            if (!medCardFound) {
                return L"Задание: Путь в архив перекрыт. Найдите ключ-карту вирусолога в операционной";
            }
            if (!medQuarantineBypassed) {
                return L"Задание: Активируйте терминал дезинфекции шлюза и удерживайте оборону";
            }
            return L"Задание: Карантин снят. Пройдите через гермодверь в серверный отсек";
        }

        if (currentScene == 6) {
            if (!serverDownloadStarted) {
                return L"Задание: Подключите дешифратор к главному серверному ядру корпорации";
            }
            if (!serverDataSecured) {
                return L"Задание: Защищайте Марка! Удерживайте натиск штурмового отряда до конца загрузки";
            }
            return L"Задание: Файлы заговора скачаны. Бегите к аварийной шахте вентиляции!";
        }

        return L"Задание: Продолжайте прохождение";
    }
};

#endif
