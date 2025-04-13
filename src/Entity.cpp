#include "Entities.h"
#include <cstdio>
//#include <typeinfo>

constexpr float minMotion = -1;
constexpr float maxMotion = 1;

Entity::Entity(Vector2 spawnPos, float startSpeed, float startSize, Color startTint) : position(spawnPos), curMotion({ 0,0 }), targetMotion({ 0,0 }), speed(startSpeed), size(startSize), tint(startTint), attackPos(spawnPos), rightHandPos(spawnPos), leftHandPos(spawnPos),
curAttackTime(0), attackSpeed(0.15f), wasHit{false}, maxHealth{100}
{
	curHealth = maxHealth;
}

Entity::~Entity() = default;

// METHODS
void Entity::setMotion(float x, float y) {
	// Clamp motion -1 ~ 1 to ensure that speed will be the actual velocity scale
	targetMotion.x = Clamp(x, -1, 1);
	targetMotion.y = Clamp(y, -1, 1);
}

void Entity::setPosition(float x, float y) {
	position.x = x;
	position.y = y;
}

void Entity::setSpeedMultiplier(float multiplier) {
	speedMultiplier = multiplier;
}

void Entity::setStrength(float value) {
	strength = value;
	printf("New Strength: %f\n", strength);
}

// VIRTUAL METHODS
void Entity::Tick(const float& deltaTime) {
	if (!isAlive()) {
		return;
	}

	// Check actions
	Vector2 attackDirection = Vector2Normalize(Vector2Subtract(GetTargetPosition(), position));
	attackPos = Vector2Add(getPosition(), Vector2Scale(attackDirection, getAttackDistance()));
	if (knockbackTimer < 0) {
		wasHit = false;
		if (isAttacking()) {
			curAttackCooldown = attackCooldown;
			curAttackTime -= deltaTime;
			if (curAttackTime > 0) {
				// Dash frontwards
				setMotion(attackDirection.x, attackDirection.y);
			}
			else
			{
				// Stay still until attack finishes
				setMotion(0, 0);
			}
		}
		else
		{
			if (canAttack() && IsTryingToAttack()) {
				curAttackTime = attackSpeed;
				curAttackCount++;
			}

			if (curAttackCooldown > 0) {
				curAttackCooldown -= deltaTime;
			}

			if (IsTryingToDefend()) {
				// Hands forward body
				isDefending = true;
				setSpeedMultiplier(0.5f);
			}
			else {
				// Resting hands
				isDefending = false;
				setSpeedMultiplier(1);
			}
		}
	}
	else {
		// On knockback, can't do anything besides slide
		knockbackTimer -= deltaTime;
		setMotion(knockbackDir.x, knockbackDir.y);
		setSpeedMultiplier(curKnockbackForce);
	}

	UpdatePosition(deltaTime);
}

void Entity::UpdatePosition(const float& deltaTime) {
	// Track hand position
	Vector2 targetRightHandPos{ position };
	Vector2 targetLeftHandPos{ position };
	float handSpeed = deltaTime * speed * 0.125f; // Resting speed


	if (isAttacking()) {
		handSpeed = 1.f - curAttackTime / attackSpeed;

		// Based on cur attack count, use one of the hands to attack
		//DrawPolyLines(attackPos, 3, 5, 0, RED);
		if (curAttackCount % 2 == 0) {
			// Use right hand
			targetRightHandPos = attackPos;
		}
		else {
			// Use left hand
			targetLeftHandPos = attackPos;
		}
	}
	else {
		Vector2 forward = Vector2Normalize(Vector2Subtract(attackPos, getPosition()));
		Vector2 forwardHandPos = Vector2Add(getPosition(), Vector2Scale(forward, getSize() * 1.5f));
		Vector2 perpendicular =  {-forward.y, forward.x };
		if (IsTryingToDefend()) {
			// Hands forward body
			handSpeed = deltaTime * getSize();
			float defendingPos = getSize() * 0.25f;
			targetRightHandPos = Vector2Add(forwardHandPos, Vector2Scale(perpendicular, defendingPos));
			targetLeftHandPos = Vector2Add(forwardHandPos, Vector2Scale(perpendicular, -defendingPos));
		}
		else {
			// Hands sideways
			float restingPos = getSize() * 0.75f;
			targetRightHandPos = Vector2Add(forwardHandPos, Vector2Scale(perpendicular, restingPos));
			targetLeftHandPos = Vector2Add(forwardHandPos, Vector2Scale(perpendicular, -restingPos));
		}
	}

	// Update body
	if (isOnKnockback()) {
		// No smoothing
		curMotion = targetMotion;
	}
	else
	{
		curMotion = Vector2Lerp(curMotion, targetMotion, 8.5f * deltaTime);
	}
	position = Vector2Add(position, Vector2Scale(curMotion, deltaTime * speed * speedMultiplier));

	// Update hands
	//DrawPolyLines(targetRightHandPos, 4, 8, 0, GREEN);
	//DrawPolyLines(targetLeftHandPos, 4, 8, 0, GREEN);
	rightHandPos = Vector2Lerp(rightHandPos, targetRightHandPos, handSpeed);
	leftHandPos = Vector2Lerp(leftHandPos, targetLeftHandPos, handSpeed);
}

void Entity::Draw() {
	Color bodyTint = tint;
	Color hpTint = MAROON;
	if (isAlive()) {
		if (wasHit && (knockbackTimer * 0.5f) > 0) {
			bodyTint = RAYWHITE;
			hpTint = RAYWHITE;
		}
	}
	else {
		bodyTint = DARKBROWN;
	}

	DrawCircleV(position, size, bodyTint);

	// Draw fists
	DrawCircleV(rightHandPos, size * 0.35f, bodyTint);
	DrawCircleV(leftHandPos, size * 0.35f, bodyTint);

	// Arms?
	//DrawLineEx(getPosition(), rightHandPos, getSize() * 0.25f, tint);
	//DrawLineEx(getPosition(), leftHandPos, getSize() * 0.25f, tint);

	// Health ring
	DrawRing(position, size * 0.5f, size * 0.75f, 0, getHealthRatio() * 360, 12, hpTint);

	// Text feedback
	if (knockbackTimer > 0 && isAlive()) {
		// Received an attack
		Vector2 dmgLabelPos = Vector2SubtractValue((Vector2Add(getPosition(), Vector2{ (float)GetRandomValue(-2, 2), (float)GetRandomValue(-2, 2) })), size * 0.5f);

		if (wasHit) {
			// Display the damage
			DrawText(TextFormat("-%i", lastDamage), dmgLabelPos.x, dmgLabelPos.y, 16, MAROON);
		}
		else {
			// Display it was blocked
			
			DrawText("BLOCKED", dmgLabelPos.x, dmgLabelPos.y, 12, MAROON);
		}
	}
}

void Entity::Reset() {
	
}

void Entity::OnDeath() {

}

void Entity::CheckBump(Entity& other) {
	if (!other.isAlive() || !isAlive()) {
		return;
	}

	if (!CheckCollisionCircles(getPosition(), getSize(), other.getPosition(), other.getSize())) {
		return;
	}

	// Get direction of collision
	Vector2 direction = Vector2Subtract(other.getPosition(), getPosition());

	// Calculate penetration depth
	float overlap = (getSize() + other.getSize()) - Vector2Length(direction);

	direction = Vector2Normalize(direction);
	Vector2 correction = Vector2Scale(direction, overlap * 0.5f);
	position = Vector2Subtract(position, correction);

	// Instead of inverting motion, project motion onto the normal
	Vector2 normal = { -direction.x, -direction.y };
	float velocityAlongNormal = Vector2DotProduct(curMotion, normal);

	if (velocityAlongNormal < 0) { // Prevent increasing speed
		Vector2 pushbackMotion = Vector2Normalize(Vector2Subtract(curMotion, Vector2Scale(normal, velocityAlongNormal)));
		setMotion(pushbackMotion.x, pushbackMotion.y);
	}
}

void Entity::CheckHit(Entity& other) {
	if (!other.isAlive() || !isAlive()) {
		return;
	}

	if (!isAttacking()) {
		return;
	}

	// The attack will be a line from the player to the max range
	for (float h = 0; h < getAttackDistance(); h += attackRadius) {
		Vector2 hitPos = Vector2Lerp(attackPos, getPosition(), h / getAttackDistance());
		//DrawCircleLinesV(hitPos, attackRadius, RED);
		if (CheckCollisionCircles(hitPos, attackRadius, other.getPosition(), other.getSize())) {
			other.TryGetDamagedBy(*this, strength);
			//printf("%s attacked %s by %i damage\n", typeid(*this).name(), typeid(other).name(), 5);
			break;
		}
	}

}

bool Entity::IsTryingToAttack() {
	return false;
}

bool Entity::IsTryingToDefend() {
	return false;
}

Vector2 Entity::GetTargetPosition() {
	return attackPos;
}

bool Entity::TryGetDamagedBy(Entity& attacker, int damage) {
	if (knockbackTimer > 0) {
		return false; // Wait for knockback to end. Don't stack hits
	}

	knockbackDir = Vector2Normalize(Vector2Subtract(getPosition(), attacker.getPosition()));
	if (isDefending) {
		// Don't take full knockback and neither damage
		knockbackTimer = knockbackDuration * 0.35f;
		curKnockbackForce = knockbackMaxForce * 0.1f;
		wasHit = false;
		return false;
	}

	knockbackTimer = knockbackDuration;
	curKnockbackForce = knockbackMaxForce;
	curHealth -= damage;
	lastDamage = damage;
	wasHit = true;
	return true;
}