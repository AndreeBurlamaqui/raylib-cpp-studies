#include "Entities.h"

Player::Player(float startSpeed, float startSize, Color startTint) : Entity({ GetScreenWidth() / 2.f, GetScreenHeight() / 2.f }, startSpeed, startSize, startTint), initialStrength{8}
{
	setStrength(initialStrength);
}

void Player::Tick(const float& deltaTime) {
	if (!isAlive()) {
		return;
	}

	// Update velocity by input
	Vector2 tickMotion{};
	if (IsKeyDown(KEY_W)) {
		tickMotion.y -= 1;
	}

	if (IsKeyDown(KEY_S)) {
		tickMotion.y += 1;
	}

	if (IsKeyDown(KEY_A)) {
		tickMotion.x -= 1;
	}

	if (IsKeyDown(KEY_D)) {
		tickMotion.x += 1;
	}

	if (!isAttacking()) {
		setMotion(tickMotion.x, tickMotion.y);
	}

	// Update player position
	Entity::Tick(deltaTime);
}

void Player::Draw() {
	// Draw entity
	Entity::Draw();

	//// Draw debug values
	//const int debugFontSize = 20;
	//int cornerY = GetScreenHeight();
	//int cornerX = 5;
	//const char* motionDebug = TextFormat("%s: x:%.f/y:%.f", "Motion", getMotion().x, getMotion().y);
	//cornerY -= (debugFontSize + cornerX);
	//DrawText(motionDebug, cornerX, cornerY, debugFontSize, BLACK);
}

bool Player::IsTryingToAttack() {
	return IsMouseButtonDown(MOUSE_BUTTON_LEFT);
}

bool Player::IsTryingToDefend() {
	return IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
}

Vector2 Player::GetTargetPosition() {
	if (isAttacking()) {
		return Entity::GetTargetPosition(); // It'll return the current attack pos, so that the player attack one "line" per punch
	}

	return GetMousePosition();
}

bool Player::TryGetDamagedBy(Entity& attacker, int damage) {
	if (Entity::TryGetDamagedBy(attacker, damage)) {
		// Update strength based on health left
		float scaledStrength = initialStrength * (1.f / getHealthRatio());
		setStrength(fmaxf(initialStrength, scaledStrength));
		return true;
	}
	return false;
}