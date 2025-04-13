#include <iostream>
#include "raylib.h"
#include "raymath.h"

// Global settings
constexpr int windowWidth{ 680 };
constexpr int windowHeight{ 320 };
constexpr int gravity{ 25 };
constexpr Vector2 centerScreen{ windowWidth / 2, windowHeight / 2 };
constexpr Color TRANSPARENT{ (0, 0, 0, 0) };
enum GameState { WIN, LOSE, PLAYING };
GameState gameState{ PLAYING };
constexpr int finishLineDistance{ 100 };
float runningTime{ 0 };
float runningIncreaseRate{ 15 };

class Entity {

public:
	// INITIAL
	Vector2 initPosition;
	Rectangle initRect;
	Color initTint;

	// RUNTIME
	Vector2 position;
	Vector2 velocity;
	Rectangle rect;
	Color tint;

	Entity(float initX, float initY, float initWidth, float initHeight, Color initTint) :
		position({ initX, initY }),
		rect({ initX, initY, initWidth, initHeight }),
		tint(initTint),
		velocity({ 0,0 }),
		initPosition({ initX, initY }),
		initRect({ initX, initY, initWidth, initHeight }),
		initTint(initTint){
		Reset();
	}

	virtual ~Entity() = default; // Ensure proper cleanup for derived types

	virtual void Update(const float& deltaTime) {
		position.x += (velocity.x * deltaTime);
		position.y += (velocity.y * deltaTime);
		rect.x = position.x;
		rect.y = position.y;
		DrawRectangle(position.x, position.y, rect.width, rect.height, tint);
	}

	virtual void CheckCollision(Entity* entities[], const int& entitiesCount) {
		for (int e = 0; e < entitiesCount;  e++) {
			Entity* otherEntity{ entities[e] };
			if (otherEntity == this) {
				continue;
			}

			if (CheckCollisionRecs(otherEntity->rect, rect)) {
				// Is colliding with other entity
				OnDeath();
			}
		}
	}

	virtual void Reset() {
		position = initPosition;
		rect = initRect;
		tint = initTint;
		velocity.x = 0;
		velocity.y = 0;
	}

	const float GetGroundPosition() const {
		return windowHeight - rect.height;
	}

private:
	virtual void OnDeath() {
	}
};

class Player : public Entity {
private:
	const float jumpCoyoteTime{ 0.075f };
	const float downForce{ 100 };
	int jumpForce{ 650 };
	float lastJumpClick{ 0 };
	enum class PlayerState { Grounded, OnAir, Dead };
	PlayerState state { PlayerState::Grounded };

public:
	constexpr static float width { 25 };
	constexpr static float height { 75 };
	constexpr static float crouchMultiplier { 0.7f };
	Player() : Entity(0, 0, width, height, BLUE) {
		position = Vector2{ rect.width * 2, GetGroundPosition() };
		initPosition = position;
	}

	void Update(const float& deltaTime) override {
		if (gameState == WIN) {
			// Keep blinking gold
			float t = (sin(GetTime() * 15) + 1) * 0.5f; // Oscillates between 0 and 1
			tint = ColorLerp(initTint, GOLD, t);
			Entity::Update(0);
			return;
		}

		if (state != PlayerState::Dead) {
			// Input check
			lastJumpClick -= deltaTime;
			if (IsKeyDown(KEY_SPACE)) {
				//DrawText("Jumping", windowWidth / 2, (windowHeight / 2) + 50, 20, BLACK);
				lastJumpClick = jumpCoyoteTime;
			}

			// Ground check
			if (position.y >= GetGroundPosition()) {
				state = PlayerState::Grounded;
				position.y = GetGroundPosition(); // Avoid getting inside ground
			}

			rect.height = initRect.height;
			if (IsKeyDown(KEY_DOWN)) {
				velocity.y += downForce;
				rect.height = initRect.height * crouchMultiplier;
			}
		}

		std::string stateName;
		switch (state) {
		case PlayerState::OnAir:
			stateName = "Jumping";

			velocity.y += gravity;

			break;

		case PlayerState::Grounded:
			stateName = "Grounded";
			velocity.y = 0;
			position.y = GetGroundPosition();

			if (lastJumpClick > 0) { // Coyote time
				state = PlayerState::OnAir;
				velocity.y -= jumpForce; // Apply jump to next frame it'll count as OnAir
			}
			break;

		case PlayerState::Dead:
			stateName = "Dead";
			velocity.y += gravity * 2;
			rect.width += (deltaTime * 50);
			rect.height += (deltaTime * 50);
			break;
		}

		//// Debug text
		//DrawText(stateName.c_str(), centerScreen.x, centerScreen.y, 20, BLACK);

		Entity::Update(deltaTime); // Then update position and draw entity
	}

	void Reset() override {
		state = PlayerState::Grounded;
		lastJumpClick = 0;
		Entity::Reset();
	}

protected:
	void OnDeath() override {
		if (state == PlayerState::Dead) {
			return; // Already dead
		}

		// Die towards outside screen like mario
		state = PlayerState::Dead;
		gameState = LOSE;
		velocity.y = -(jumpForce * 1.75f);
		rect.height = initRect.height;
	}
};

class Hazard : public Entity {

	struct HazardShape {
	public:
		Rectangle rect;

		HazardShape(float _w, float _h) : rect({ 0, 0, _w, _h }) {}

		virtual void ApplyShape(Hazard& haz) {
			haz.rect = rect;
		}
	};

	struct GroundShape : public HazardShape {
	public:
		GroundShape() : HazardShape(75, 50) {}

		virtual void ApplyShape(Hazard& haz) override {
			HazardShape::ApplyShape(haz);
			haz.position.y = haz.GetGroundPosition();
		}
	};

	struct AirShape : public HazardShape {
	public:
		AirShape() : HazardShape(100, 50) {}

		virtual void ApplyShape(Hazard& haz) override {
			HazardShape::ApplyShape(haz);
			haz.position.y = static_cast<float>(windowHeight) / 2;
		}
	};

	struct StickShape : public HazardShape {
	public:
		StickShape() : HazardShape(25, 100) {}

		virtual void ApplyShape(Hazard& haz) override {
			HazardShape::ApplyShape(haz);
			haz.position.y = haz.GetGroundPosition();
		}
	};

	struct CrouchShape : public HazardShape {
	public:
		CrouchShape() : HazardShape(100, 150) {}

		virtual void ApplyShape(Hazard& haz) override {
			HazardShape::ApplyShape(haz);
			haz.position.y = haz.GetGroundPosition() - (Player::height * Player::crouchMultiplier);
		}
	};

public:
	const int hazardSpeed{ 250 };

	Hazard() : Entity(windowWidth, 0, 0, 0, RED) { }
	~Hazard() override {
		// Free hazard shapes
		for (int i = 0; i < shapeCount; i++) {
			delete possibleShapes[i]; // Free memory for each shape
			possibleShapes[i] = nullptr; // Avoid dangling pointers
		}
	}

	void Update(const float& deltaTime) override {
		if (gameState != PLAYING) {
			Entity::Update(0);
			return;
		}

		if (position.x < -rect.width) {
			Reset();
		}

		velocity.x = -(hazardSpeed + (runningTime * runningIncreaseRate));

		Entity::Update(deltaTime);
	}

	void Reset() override{
		Entity::Reset();
		ChangeShape();
	}

protected:
	const static int shapeCount{ 4 };
	HazardShape* possibleShapes[shapeCount]{ new GroundShape(), new AirShape(), new StickShape(), new CrouchShape()};

	void ChangeShape() {
		HazardShape* randomShape{ possibleShapes[GetRandomValue(0, shapeCount - 1)] };
		randomShape->ApplyShape(*this);
	}
};

class Environment {
public:
	Vector2 position;
	float size;
	Color tint;
	Environment() : position({ 0,0 }), size (0), tint(WHITE) {} // Empty ctor
	Environment(Color _t, float _s, int _vel) : tint(_t), size(_s), parallaxVel(_vel) {
		position = { (float)GetRandomValue(0, windowWidth + size), (float)(windowHeight - size) };
	}

	void Draw(const float& deltaTime) {
		if (position.x < -size) {
			Reset();
		}

		if (gameState == PLAYING) {
			position.x -= (parallaxVel + (runningTime * runningIncreaseRate)) * deltaTime;
		}
		// Triangle points
		float doubleSize = size * 2;
		Vector2 leftPoint = { position.x, position.y - size }; // Root
		Vector2 rightPoint = { position.x - size, position.y + size}; // root + size
		Vector2 upperPoint = { position.x + size, position.y + size }; // Size is height
		DrawTriangle(upperPoint, leftPoint, rightPoint, tint);
	}

private:
	int parallaxVel = 50;

	void Reset() {
		position.x = windowWidth + (size * GetRandomValue(1, 3));
	}
};


int main()
{
	InitWindow(windowWidth, windowHeight, "Dapper Dasher");
	SetTargetFPS(60);

	const int entitiesCount = 2;
	Entity* entities[entitiesCount]{
		new Hazard(),
		new Player(),
	};

	const int mountainsCount = 30;
	Environment mountains[mountainsCount]; // Background
	for (int i = 0; i < mountainsCount; i++) {
		if (i < 18) { // First 18 elements (Second layer)
			mountains[i] = Environment(DARKGRAY, GetRandomValue(50, 125), 25);
		}
		else { // Remaining 12 elements (First layer)
			mountains[i] = Environment(GRAY, GetRandomValue(25, 75), 50);
		}
	}

	const int grassCount = 100;
	Environment grass[grassCount]; // Foreground
	for (int i = 0; i < grassCount; i++) {
		grass[i] = Environment(DARKGREEN, GetRandomValue(2, 5), 175);
	}

	const char* resetText{ "Press R to restart" };
	const int fontSize = 20;
	const float resetTextSize = MeasureText(resetText, fontSize);
	const char* recordText{ "Record: " };

	while (!WindowShouldClose())
	{
		BeginDrawing();
		ClearBackground(RAYWHITE);

		const float& deltaTime = GetFrameTime();

		// BACKGROUND
		for (int m = 0; m < mountainsCount; m++) {
			mountains[m].Draw(deltaTime);
		}
		DrawRectangleGradientV(0, 0, windowWidth, windowHeight, TRANSPARENT, LIGHTGRAY);

		// ENTITIES
		for (int e = 0; e < entitiesCount; e++) {
			Entity* entityLoop = entities[e];
			entityLoop->Update(deltaTime);
			entityLoop->CheckCollision(entities, entitiesCount);
		}

		// FOREGROUND
		for (int m = 0; m < grassCount; m++) {
			grass[m].Draw(deltaTime);
		}

		// UI
		// Show how much to finish
		const int UI_finishLineSize = windowWidth / 2;
		const int UI_startFinishLineSize = UI_finishLineSize / 2;
		const int UI_endFinishLineSize = windowWidth - UI_startFinishLineSize;
		const int UI_finishLineHeight = 16;
		const int UI_finishLineThickness = 2;
		const int UI_midFinishLine = (UI_finishLineHeight / 2) + (UI_finishLineThickness / 2);
		const int UI_blockSize = 6;
		DrawRectangle(UI_startFinishLineSize, UI_finishLineHeight, UI_finishLineSize, UI_finishLineThickness, GRAY); // Line
		DrawRectangle(UI_startFinishLineSize - UI_blockSize, UI_midFinishLine, UI_blockSize, UI_finishLineHeight, GRAY); // Start square
		DrawRectangle(UI_endFinishLineSize - UI_blockSize, UI_midFinishLine, UI_blockSize, UI_finishLineHeight, GRAY); // End square
		const int UI_progressionHeight = UI_finishLineHeight * 0.5f;
		//float ui_progression = Lerp(UI_startFinishLineSize, UI_endFinishLineSize, runningTime / finishLineDistance);
		//DrawCircle(ui_progression, UI_midFinishLine + UI_progressionHeight, UI_progressionHeight, GREEN); // Progression
		float ui_progression = Lerp(0, UI_finishLineSize, runningTime / finishLineDistance);
		DrawRectangle(UI_startFinishLineSize, UI_midFinishLine + (UI_progressionHeight / 2), ui_progression, UI_progressionHeight, DARKGREEN); // Progression


		// END FRAME CHECK
		switch (gameState) {
		case PLAYING:
			DrawText(TextFormat("%.2f", runningTime), UI_finishLineSize - (MeasureText("00.00", fontSize) / 2), UI_finishLineHeight + (fontSize / 2), fontSize, DARKGRAY);
			if (runningTime >= finishLineDistance) {
				gameState = WIN;
			}
			runningTime += deltaTime;
			break;
		case LOSE:
		case WIN:
			DrawText(resetText, centerScreen.x - (resetTextSize / 2), centerScreen.y, fontSize, BLACK);
			const char* recordRunning = TextFormat("%s%.2f", recordText, runningTime);
			float recordTextSize = MeasureText(recordRunning, fontSize);
			DrawText(recordRunning, centerScreen.x - (recordTextSize / 2), centerScreen.y + fontSize, fontSize, BLACK);
			if (IsKeyDown(KEY_R)) {
				for (int e = 0; e < entitiesCount; e++) {
					entities[e]->Reset();
				}

				runningTime = 0;
				gameState = PLAYING;
			}
			break;
		}

		EndDrawing();
	}

	// Cleanup memory
	for (int i = 0; i < entitiesCount; i++) {
		delete entities[i]; // Free memory for each entity
		entities[i] = nullptr; // Avoid dangling pointers
	}

	CloseWindow();
}
