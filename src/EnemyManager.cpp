#include "EnemyManager.h"

EnemyManager::EnemyManager(Player& _player, std::vector<Entity*>& entitiesList) : targetPlayer(_player){
    for (int e = 0; e < enemiesCount; e++) {
        Enemy* newEnemy = new Enemy(getRandomOutsidePosition(), e * 75, 100, 15, DARKBLUE);
        enemies[e] = newEnemy;
        entitiesList.push_back(newEnemy);
    }
}

void EnemyManager::OrganizeEnemies() {
    for (int e = 0; e < enemiesCount; e++) {
        Enemy* loopEnemy = enemies[e];
        loopEnemy->setTargetPosition(targetPlayer.getPosition());
    }
}

Vector2 EnemyManager::getRandomOutsidePosition() {
    const int margin = 50;
    int side = GetRandomValue(0, 3); // 0 = left, 1 = right, 2 = top, 3 = bottom
    Vector2 pos{ 0,0 };

    switch (side) {
    case 0: // Left
        pos.x = -margin;
        pos.y = GetRandomValue(0, GetScreenHeight());
        break;
    case 1: // Right
        pos.x = GetScreenWidth() + margin;
        pos.y = GetRandomValue(0, GetScreenHeight());
        break;
    case 2: // Top
        pos.x = GetRandomValue(0, GetScreenWidth());
        pos.y = -margin;
        break;
    case 3: // Bottom
        pos.x = GetRandomValue(0, GetScreenWidth());
        pos.y = GetScreenHeight() + margin;
        break;
    }
    return pos;
}