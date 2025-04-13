#include "raylib.h"
#include "raymath.h"

#ifndef ENTITIES_H
#define ENTITIES_H

class Entity {
public:
	Entity(Vector2 spawnPos, float startSpeed, float startSize, Color startTint);
	~Entity();

	// GETTERS
	const Vector2& getMotion() const { return curMotion; }
	const Vector2& getPosition() const { return position; }
	const Color& getTint() const { return tint; }
	const float& getSize() const { return size; }
	const float& getAttackSpeed() const { return attackSpeed; }
	const float& getAttackDistance() const { return attackRange * (getSize() / 2); }
	const float& getAttackRadius() const { return attackRadius; }
	float getHealthRatio() { return fmaxf(0, curHealth / maxHealth); }
	const bool& isAttacking() const { return curAttackTime > 0; }
	const bool& isOnGuard() const { return isDefending; }
	const bool& canAttack() const { return curAttackCooldown <= 0 && !isOnGuard(); }
	const bool& isOnKnockback() const { return knockbackTimer > 0; }
	const bool& isAlive() const { return curHealth > 0; }

	// SETTERS
	void setMotion(float x, float y);
	void setPosition(float x, float y);
	void setSpeedMultiplier(float multiplier);
	void setStrength(float value);
	
	// VIRTUAL METHODS
	virtual void Tick(const float& deltaTime);
	virtual void Draw();
	virtual void Reset();
	/// <summary>
	/// Will check if it's bumping into other entity, to not overlap
	/// </summary>
	virtual void CheckBump(Entity& other);
	/// <summary>
	/// Will check if it's overlapping the hitbox of the other entity
	/// </summary>
	virtual void CheckHit(Entity& other);
	virtual bool IsTryingToAttack();
	virtual bool IsTryingToDefend();
	virtual bool TryGetDamagedBy(Entity& attacker, int damage);

private:
	// General
	Vector2 position{};
	Vector2 curMotion{};
	Vector2 targetMotion{};
	float speed{};
	float speedMultiplier{ 1 };
	float size{};
	Color tint{ BLACK };
	float strength{};

	// Health
	float curHealth{ 10 };
	float maxHealth{ 100 };
	bool wasHit;
	int lastDamage{ 0 };

	// Attack
	bool isTryingToAttack{};
	Vector2 attackPos{};
	float attackRadius{ 10 };
	Vector2 leftHandPos{};
	Vector2 rightHandPos{};
	float curAttackTime{ 0 }, attackSpeed{ 0.1f };
	float curAttackCooldown{ 0.15f }, attackCooldown{ 0.15f };
	int curAttackCount{  };
	float attackRange{ 10 };

	// Defending
	bool isDefending{};

	// Knockback
	Vector2 knockbackDir{};
	float knockbackTimer{ 0 };
	float curKnockbackForce { 0 };
	const float knockbackDuration{ 0.15f };
	const float knockbackMaxForce { 2.25f };

protected:
	virtual void OnDeath();
	virtual void UpdatePosition(const float& deltaTime);
	virtual Vector2 GetTargetPosition();
};

class Player : public Entity {
public:
	Player(float startSpeed, float startSize, Color startTint);

	void Tick(const float& deltaTime) override;
	void Draw() override;
	bool IsTryingToAttack() override;
	bool IsTryingToDefend() override;
	bool TryGetDamagedBy(Entity& attacker, int damage) override;

protected:
	Vector2 GetTargetPosition() override;
	const float initialStrength{ 5 };
};

class Enemy : public Entity {
public:
	Enemy(Vector2 spawnPos, float anglePos, float startSpeed, float startSize, Color startTint);

	void setTargetPosition(Vector2 newPos) {
		targetPosition = newPos;
	}

	// Overrides
	void Tick(const float& deltaTime) override;
	bool IsTryingToAttack() override;
	bool IsTryingToDefend() override;
	bool TryGetDamagedBy(Entity& attacker, int damage) override;

private:
	float onDamagedTimer{ 0 };
	float onHitTimer{};
	const float onHitCooldown{ 0.35f };
	int consecutiveHits{};
	float guardTimer{};
	float guardMaxTime{ 3 };
	int hitsToBlock{ 4 };
	bool hasBlocked{};
	float orbitPos{};
	Vector2 targetPosition{}; // Controlled by Enemy Manager
	Vector2 GetTargetPosition() override;
};

#endif