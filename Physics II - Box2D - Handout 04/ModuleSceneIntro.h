#pragma once
#include "Module.h"
#include "p2List.h"
#include "p2Point.h"
#include "Globals.h"

class PhysBody;
class b2RevoluteJoint;
class b2Body;

class ModuleSceneIntro : public Module
{
public:
	ModuleSceneIntro(Application* app, bool start_enabled = true);
	~ModuleSceneIntro();

	bool Start();
	update_status Update();
	bool CleanUp();
	void OnCollision(PhysBody* bodyA, PhysBody* bodyB);

public:
	p2List<PhysBody*> circles;

	PhysBody* sensor;
	bool sensed;

	SDL_Texture* circle;
	SDL_Texture* fondo;
	SDL_Texture* goku;
	SDL_Texture* nube;
	SDL_Texture* puñoIzq;
	SDL_Texture* puñoDer;
	SDL_Texture* baston;
	uint bonus_fx;
	uint shenlong;
	uint resp;
	uint dbrap;
	uint punch_fx;
	p2Point<int> ray;
	bool ray_on;

	//BG
	PhysBody* tableroColliders[11];

	//Palas
	PhysBody* flipperLeft;
	PhysBody* flipperRight;
	PhysBody* flipperLeftPoint;
	PhysBody* flipperRightPoint;
	int flipperforce = -250;

	//Goku
	b2RevoluteJoint* revolute_jointgoku;
	b2Body* pivot_goku;
	PhysBody* gokup;

	//Spring
	PhysBody* springTop;
	PhysBody* springBot;
	int springForce;

	// Bumpers
	PhysBody* bumperTop;
	PhysBody* bumperMid;

	// Sensors
	PhysBody* leftPad;
	PhysBody* rightPad;

	PhysBody* leftPlat;
	PhysBody* rightPlat;

	PhysBody* loseSensor;

	float bumperTopX;
	float bumperTopY;
	float bumperMidX;
	float bumperMidY;

	int scoreFont;
	char scoreText[10] = { "\0" };
	int score;
	bool reset;
	int lives;
	char scoreLives[4] = { "\0" };

	int count;
	bool dir;

	float bumperVel = 0.4f;

	void createPalas();
	void createBG();
	void createSpring();
	void createMotor();
	void CreateSensors();
	void resetBall();
};
