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
    bool hallwayKeysFound;
    bool hallwayNote2Read;

    bool elevatorInspected;
    bool elevatorBossDead;
    bool elevatorHackSuccess;

    bool techPdaFound;
    bool techTerminalBypassed;
    bool techGasCleared;

    bool medCardFound;
    bool medQuarantineStarted;
    bool medQuarantineBypassed;

    bool serverBossSpawned;
    bool serverBossDefeated;
    bool serverDataDownloaded;

    StoryManager(Difficulty diff) {
        gameDifficulty = diff;
        currentScene = 1;

        noteRead = false;
        readLaptopEmail = false;
        talkedToMarkStart = false;
        markMovingToExit = false;
        hallwayIntroPlayed = false;
        hallwayKeysFound = false;
        hallwayNote2Read = false;
        elevatorInspected = false;
        elevatorBossDead = false;
        elevatorHackSuccess = false;
        techPdaFound = false;
        techTerminalBypassed = false;
        techGasCleared = false;
        medCardFound = false;
        medQuarantineStarted = false;
        medQuarantineBypassed = false;

        serverBossSpawned = false;
        serverBossDefeated = false;
        serverDataDownloaded = false;
    }

    std::wstring getCurrentQuestText() {
        if (currentScene == 1) {
            if (!noteRead) return L"Задание: Подойдите к кухонный столу и прочтите записку Марка (E)";
            if (!readLaptopEmail) return L"Задание: Сядьте за рабочий ПК и взломайте брандмауэр сети (E)";
            if (!talkedToMarkStart) return L"Задание: Подойдите к дивану и обсудите план побега с Марком (E)";
            if (!markMovingToExit) return L"Задание: Введите дату знакомства на шкафу прихожей и заберите Глок (E)";
            return L"Задание: Оружие в руках. Сделайте шаг за входную дверь в коридор подъезда";
        }
        if (currentScene == 2) {
            if (!hallwayIntroPlayed) return L"Задание: Сделайте несколько шагов вперёд и осмотритесь в темноте";
            if (!hallwayKeysFound) return L"Задание: Расстреляйте заражённого Толика (Пробел) и обыщите его тело (E)";
            if (!hallwayNote2Read) return L"Задание: Пройдите в квартиру N40 и заберите рапорт с тумбочки (E)";
            return L"Задание: Подойдите к технической двери шлюза слева и откройте её ключами (E)";
        }
        if (currentScene == 3) {
            if (!elevatorInspected) return L"Задание: Обследуйте сломанные створки шахты лифта в центре зала (E)";
            if (!elevatorBossDead) return L"Задание: Срочно подключите ноутбук к панели слева и запустите дешифратор (E)";
            if (!elevatorHackSuccess) return L"Задание: Отбивайтесь от Мутанта и удерживайте Пробел в тайминги волны брандмауэра";
            return L"Задание: Частота синхронизирована! Подбегите к открывшимся дверям лифта (E)";
        }
        if (currentScene == 4) {
            if (!techPdaFound) return L"Задание: Сектор заблокирован ядом! Срочно обыщите тело инженера на X=450 (E)";
            if (!techTerminalBypassed) return L"Задание: Откройте инвентарь (I), прочтите КПК через Enter и введите шифр в терминал на X=1100 (E)";
            if (!techGasCleared) return L"Задание: Дождитесь завершения рапорта. Бегите к клапану трубы на X=750 и проверните вентиль (E)";
            return L"Задание: Газ полностью откачан. Проследуйте к правому краю локации в открывшийся шлюз";
        }
        if (currentScene == 5) {
            if (!medCardFound) return L"Задание: Путь заблокирован. Обыщите хирургические столы на X=350 и найдите ключ-карту (E)";
            if (!medQuarantineStarted) return L"Задание: Карта у вас. Бегите к настенной панели дезинфекции на X=1150 и приложите её (E)";
            if (!medQuarantineBypassed) return L"Задание: Карантин! Защищайте Марка и удерживайте оборону от наступающих зомби-врачей";
            return L"Задание: Дезинфекция завершена, блокировка снята. Сделайте шаг в открывшийся Surgery Block";
        }

        if (currentScene == 6) {
            if (!serverBossSpawned) return L"Задание: Исследуйте глубины Секретной Лаборатории застройщика";
            if (!serverBossDefeated) return L"Задание: ФИНАЛЬНАЯ БИТВА! Уничтожьте фиолетового Мутанта Объект-00";
            if (!serverDataDownloaded) return L"Задание: Опасность миновала. Бегите к Главному Серверу на X=800 и скачайте архивы (E)";
            return L"Задание: БЕСКОМПРОМИССНЫЙ ВЫБОР! Сделайте свой выбор финала на клавиатуре (1 или 2)";
        }

        return L"Задание: Сюжетная ветка обрабатывается...";
    }
};

#endif
