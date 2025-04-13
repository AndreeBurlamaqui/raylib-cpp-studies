#include <vector>
#include "Entities.h"

#ifndef ENEMYMANAGER_H
#define ENEMYMANAGER_H
/// <summary>
/// Class to control a group of enemies.
/// Each enemy still has their some of their own "brain", but the group controlling will be made here
/// </summary>
class EnemyManager {
public:
	EnemyManager(Player& _player, std::vector<Entity*>& entitiesList);
	void OrganizeEnemies();

private:  
	static constexpr int enemiesCount = 5;
	Enemy* enemies[enemiesCount];
	Player& targetPlayer;

	Vector2 getRandomOutsidePosition();
};
#endif