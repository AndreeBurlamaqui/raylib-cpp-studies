#pragma once
#include "raylib.h"
#include "DapperDasher.cpp"

class Entity {

public:
	Vector2 position;
	Vector2 velocity;
	Rectangle rect;
	Color tint;

	Entity(float initX, float initY, float initWidth, float initHeight, Color initTint) :
		position({ initX, initY }), rect({ 0,0, initWidth, initHeight }), tint(initTint){}
	virtual ~Entity() = default; // Ensure proper cleanup for derived types

	virtual void Update(const float& deltaTime);

protected:
	float GetGroundPosition();
};

class Player : public Entity {
public:
	Player() :Entity(Game::windowWidth, GetGroundPosition(), 25, 75, BLUE){}
	int jumpForce{ 550 };
	const float jumpCoyoteTime{ 0.075f };
	float lastJumpClick;
	enum class PlayerState { Grounded, OnAir };
	PlayerState state;
};

class Hazard : Entity {

};