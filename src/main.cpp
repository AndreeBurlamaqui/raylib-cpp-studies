#include "raylib.h"
#include "raymath.h"
#include "Entities.h"
#include "EnemyManager.h"

int main()
{
	InitWindow(1280, 720, "Classy Clash");
	SetTargetFPS(60);

	Player* player = new Player(150, 17, DARKGREEN);
	std::vector<Entity*> entities;
	entities.push_back(player);

	EnemyManager enemyGroup(*player, entities);

	while (!WindowShouldClose()) {
		/// UPDATE
		const float deltaTime = GetFrameTime();

		enemyGroup.OrganizeEnemies(); // Organize enemies before loop

		int entitiesCount = entities.size();
		for (int e = 0; e < entitiesCount; e++) {
			Entity* thisEntity = entities[e];
			thisEntity->Tick(deltaTime);

			// Check collisions, hits and attack order on other entities
			for (int j = 0; j < entitiesCount; j++) {  // Avoid duplicate checks
				Entity* otherEntity = entities[j];
				if (otherEntity == thisEntity) {
					continue;
				}

				thisEntity->CheckBump(*otherEntity); // Dereference other enemy
				thisEntity->CheckHit(*otherEntity);
			}
		}


		///---

		/// DRAW
		BeginDrawing();
		ClearBackground(LIGHTGRAY);

		// Draw deads first
		for (int e = 0; e < entitiesCount; e++) {
			Entity* thisEntity = entities[e];
			if (!thisEntity->isAlive()) {
				thisEntity->Draw();
			}
		}

		for (int e = 0; e < entitiesCount; e++) {
			Entity* thisEntity = entities[e];
			if (thisEntity->isAlive()) {
				thisEntity->Draw();
			}
		}

		EndDrawing();
		///---
	}

	// Delete enemies
	for (int e = 0; e < entities.size(); e++) {
		delete entities[e];
		entities[e] = nullptr; // Removing any trace
	}

	CloseWindow();
}