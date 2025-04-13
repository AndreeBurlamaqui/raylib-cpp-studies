#include "raylib.h"
#include "raymath.h"
#include "Entities.h"

int main()
{
	InitWindow(1280, 720, "Classy Clash");
	SetTargetFPS(60);

	const int enemiesCount = 1;
	const int entitiesCount = enemiesCount + 1;
	Player* player = new Player(150, 17, DARKGREEN);
	Entity* entities[entitiesCount]{
		player // First index will be player
	};
	for (int e = 1; e < entitiesCount; e++) {
		entities[e] = new Enemy(player, e * 75, 100, 15, DARKBLUE);
	}

	while (!WindowShouldClose()) {
		/// UPDATE
		const float deltaTime = GetFrameTime();

		for (int e = 0; e < entitiesCount; e++) {
			Entity* thisEntity = entities[e];
			thisEntity->Tick(deltaTime);

			// Check collisions, hits and attack order on other entities
			bool anyEnemyAttacking = false;
			for (int j = 0; j < entitiesCount; j++) {  // Avoid duplicate checks
				Entity* otherEntity = entities[j];
				if (otherEntity == thisEntity) {
					continue;
				}

				thisEntity->CheckBump(*otherEntity); // Dereference other enemy
				thisEntity->CheckHit(*otherEntity);

				if (otherEntity != player && otherEntity->isAttacking()) {
					anyEnemyAttacking = true;
				}
			}

			if (thisEntity != player) {
				// Tell this entity that he's attacking
				thisEntity->setCanAttack(!anyEnemyAttacking);
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
	for (int e = 0; e < entitiesCount; e++) {
		delete entities[e];
		entities[e] = nullptr; // Removing any trace
	}

	CloseWindow();
}