#include "Globals.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModuleSceneIntro.h"
#include "ModuleInput.h"
#include "ModuleTextures.h"
#include "ModuleAudio.h"
#include "ModulePhysics.h"
#include "ModuleFonts.h"

ModuleSceneIntro::ModuleSceneIntro(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	circle = NULL;
	ray_on = false;
	sensed = false;
}

ModuleSceneIntro::~ModuleSceneIntro()
{}

// Load assets
bool ModuleSceneIntro::Start()
{
	LOG("Loading Intro assets");
	bool ret = true;

	App->renderer->camera.x = App->renderer->camera.y = 0;

	bonus_fx = App->audio->LoadFx("pinball/bonus.wav");
	fondo = App->textures->Load("pinball/FondoPinballDragonBall.png");
	circle = App->textures->Load("pinball/Pelota.png");
	goku = App->textures->Load("pinball/GokuenlaNube.png");
	nube = App->textures->Load("pinball/NubeMuelle.png");
	puñoIzq = App->textures->Load("pinball/PunoBumperIzq.png");
	puñoDer = App->textures->Load("pinball/PunoBumperDer.png");
	baston = App->textures->Load("pinball/PaletaIzqHorizontal.png");
	punch_fx = App->audio->LoadFx("pinball/Punetazo.wav");
	shenlong = App->audio->LoadFx("pinball/SonidoShenlong.wav");
	resp = App->audio->LoadFx("pinball/Buu.wav");
	createBG();
	createPalas();
	createSpring();
	createMotor();
	CreateSensors();
	reset = false;
	lives = 3;

	char lookupTable[] = { "! @,_./0123456789$;< ?abcdefghijklmnopqrstuvwxyz" };
	scoreFont = App->fonts->Load("pinball/rtype_font3.png", lookupTable, 2);
	score = 0;

	App->audio->PlayMusic("pinball/Music.ogg");
	return ret;
}

// Load assets
bool ModuleSceneIntro::CleanUp()
{
	LOG("Unloading Intro scene");

	return true;
}

// Update: draw background
update_status ModuleSceneIntro::Update()
{
	App->renderer->Blit(fondo, 0, 0);
	int x, y;
	leftPlat->GetPosition(x, y);
	App->renderer->Blit(puñoIzq, x+23, y - 10);
	rightPlat->GetPosition(x, y);
	App->renderer->Blit(puñoDer, x + 23, y - 10);

	float32 flipperLeftAngle = flipperLeft->body->GetAngle();
	float32 flipperRightAngle = flipperRight->body->GetAngle();
	b2Vec2 pos;
	pos = flipperLeft->body->GetPosition();
	App->renderer->Blit(baston, METERS_TO_PIXELS(pos.x) - 50, METERS_TO_PIXELS(pos.y), NULL, 0, RADTODEG * (flipperLeftAngle), 50, 14);
	
	pos = flipperRight->body->GetPosition();
	App->renderer->Blit(baston, METERS_TO_PIXELS(pos.x)- 50, METERS_TO_PIXELS(pos.y), NULL, 0, RADTODEG * (flipperRightAngle), 50, 14);

	pos = springTop->body->GetPosition();
	App->renderer->Blit(nube, METERS_TO_PIXELS(pos.x)-31, METERS_TO_PIXELS(pos.y) -20, NULL);

	float32 flippgoku = pivot_goku->GetAngularDamping();

	pos = gokup->body->GetPosition();
	App->renderer->Blit(goku, METERS_TO_PIXELS(pos.x) - 90, METERS_TO_PIXELS(pos.y) - 60, NULL, 0, RADTODEG * (flippgoku), 90, 60);

	if (App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT) {
		flipperLeft->body->ApplyForceToCenter(b2Vec2(0, flipperforce), 1);
	}
	if (App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT) {
		flipperRight->body->ApplyForceToCenter(b2Vec2(0, flipperforce), 1);
	}

	b2Vec2 posKick = springTop->body->GetPosition();

	if (App->input->GetKey(SDL_SCANCODE_DOWN) == KEY_REPEAT)
	{
		if (springForce < 300) {
			springForce += 10;
		}
		springTop->body->ApplyForceToCenter(b2Vec2(0, springForce), 1);
	}
	if (App->input->GetKey(SDL_SCANCODE_DOWN) == KEY_UP) {
		springForce = 0;
		// Fx
	}


	// lo de abajo de puede borrar 
	if(App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
	{
		ray_on = !ray_on;
		ray.x = App->input->GetMouseX();
		ray.y = App->input->GetMouseY();
	}

	if(App->input->GetKey(SDL_SCANCODE_1) == KEY_DOWN)
	{
		PhysBody* circle = App->physics->CreateCircle(App->input->GetMouseX(), App->input->GetMouseY(), 10);
		circle->body->SetGravityScale(0.1f);
		circles.add(circle);
		circles.getLast()->data->listener = this;
	}

	// Prepare for raycast ------------------------------------------------------
	
	iPoint mouse;
	mouse.x = App->input->GetMouseX();
	mouse.y = App->input->GetMouseY();
	int ray_hit = ray.DistanceTo(mouse);

	fVector normal(0.0f, 0.0f);

	// All draw functions ------------------------------------------------------
	p2List_item<PhysBody*>* c = circles.getLast();

	if (c != NULL) {
		int x, y;
		c->data->GetPosition(x, y);

		App->renderer->Blit(circle, x, y, NULL, 1.0f, c->data->GetRotation());

		c = c->next;
	}

	// ray -----------------
	if(ray_on == true)
	{
		fVector destination(mouse.x-ray.x, mouse.y-ray.y);
		destination.Normalize();
		destination *= ray_hit;

		App->renderer->DrawLine(ray.x, ray.y, ray.x + destination.x, ray.y + destination.y, 255, 255, 255);

		if(normal.x != 0.0f)
			App->renderer->DrawLine(ray.x + destination.x, ray.y + destination.y, ray.x + destination.x + normal.x * 25.0f, ray.y + destination.y + normal.y * 25.0f, 100, 255, 100);
	}

	if (reset) {
		b2Vec2 pos = b2Vec2(PIXEL_TO_METERS(645), PIXEL_TO_METERS(700));
		circles.getLast()->data->body->SetTransform(pos, 0);
		reset = false;
		App->audio->PlayFx(resp);
	}

	if (App->input->GetKey(SDL_SCANCODE_R) == KEY_DOWN)
	{
		b2Vec2 pos = b2Vec2(PIXEL_TO_METERS(645), PIXEL_TO_METERS(700));
		circles.getLast()->data->body->SetTransform(pos,0);
		App->audio->PlayFx(resp);
	}

	App->fonts->BlitText(25, 760, scoreFont, "score");
	sprintf_s(scoreText, 10, "%7d", score);
	App->fonts->BlitText(-5, 780, scoreFont, scoreText);

	App->fonts->BlitText(530, 760, scoreFont, "lives");
	sprintf_s(scoreLives, 10, "%d", lives);
	App->fonts->BlitText(595, 780, scoreFont, scoreLives);

	if (lives <= 0) {
		lives = 3;
		score = 0;
	}

	return UPDATE_CONTINUE;
}

void ModuleSceneIntro::OnCollision(PhysBody* bodyA, PhysBody* bodyB)
{
	int x, y;

	/*
	if(bodyA)
	{
		bodyA->GetPosition(x, y);
		App->renderer->DrawCircle(x, y, 50, 100, 100, 100);
	}

	if(bodyB)
	{
		bodyB->GetPosition(x, y);
		App->renderer->DrawCircle(x, y, 50, 100, 100, 100);
	}*/
}

void ModuleSceneIntro::createPalas()
{
	//-------Crear las palas-------------------------------------------------------------------------------
	int x1 = 245;
	int y1 = 715;

	int x2 = 415;
	int y2 = 715;

	int w = 90;
	int h = 10;

	// --- Left flipper ---
	flipperLeft = App->physics->CreateRectangle(x1, y1, w, h);
	flipperLeftPoint = App->physics->CreateCircle(x1, y2, 2);
	flipperLeftPoint->body->SetType(b2_staticBody);

	// Flipper Joint (flipper rectangle x flipper circle to give it some movement)
	b2RevoluteJointDef flipperLeftJoint;

	flipperLeftJoint.bodyA = flipperLeft->body;
	flipperLeftJoint.bodyB = flipperLeftPoint->body;
	flipperLeftJoint.referenceAngle = 0 * DEGTORAD;
	flipperLeftJoint.enableLimit = true;
	flipperLeftJoint.lowerAngle = -30 * DEGTORAD;
	flipperLeftJoint.upperAngle = 30 * DEGTORAD;
	flipperLeftJoint.localAnchorA.Set(PIXEL_TO_METERS(-33), 0);
	flipperLeftJoint.localAnchorB.Set(0, 0);
	b2RevoluteJoint* joint_leftFlipper = (b2RevoluteJoint*)App->physics->world->CreateJoint(&flipperLeftJoint);

	// --- Right flipper ---
	flipperRight = App->physics->CreateRectangle(x2, y2, w, h);
	flipperRightPoint = App->physics->CreateCircle(x2, y2, 2);
	flipperRightPoint->body->SetType(b2_staticBody);

	// Flipper Joint
	b2RevoluteJointDef flipperRightJoint;

	flipperRightJoint.bodyA = flipperRight->body;
	flipperRightJoint.bodyB = flipperRightPoint->body;
	flipperRightJoint.referenceAngle = 0 * DEGTORAD;
	flipperRightJoint.enableLimit = true;
	flipperRightJoint.lowerAngle = -30 * DEGTORAD;
	flipperRightJoint.upperAngle = 30 * DEGTORAD;
	flipperRightJoint.localAnchorA.Set(PIXEL_TO_METERS(33), 0);
	flipperRightJoint.localAnchorB.Set(0, 0);
	b2RevoluteJoint* joint_rightFlipper = (b2RevoluteJoint*)App->physics->world->CreateJoint(&flipperRightJoint);
	//------------------------------------------------------------------------------------------------------------------
}

void ModuleSceneIntro::createBG()
{
	int contorno[48] = {
		661, 817,
		664, 156,
		556, 20,
		130, 20,
		35, 143,
		28, 318,
		82, 378,
		24, 378,
		74, 467,
		28, 467,
		41, 701,
		240, 827,
		241, 993,
		413, 993,
		413, 822,
		595, 704,
		606, 450,
		551, 450,
		605, 381,
		550, 381,
		607, 325,
		607, 177,
		628, 185,
		628, 818
	};
	
	//Triangulo medio
	int Triangulomedio[8] = {
		328, 298,
		276, 434,
		375, 434,
		328, 298
	};
	
	//Churro arriba
	int Churro[38] = {
		265, 86,
		286, 96,
		270, 105,
		211, 113,
		178, 127,
		165, 153,
		162, 183,
		160, 212,
		158, 229,
		154, 247,
		141, 229,
		135, 201,
		134, 176,
		133, 146,
		138, 121,
		154, 100,
		180, 89,
		203, 85,
		249, 83
	};

	int palos_der[12] =
	{
		549, 499,
		546, 619,
		461, 690,
		457, 686,
		541, 615,
		544, 500
	};

	int palos_izq[12] =
	{
		102, 505,
		105, 626,
		190, 697,
		193, 693,
		110, 622,
		107, 505
	};

	//BrazoDer
	int BrazoDer[8] = {
		475, 480,
		418, 585,
		451, 603,
		494, 479
	};
	//BrazosIzq
	int BrazosIzq[8] = {
		175, 480,
		232, 588,
		196, 600,
		156, 477
	};

	tableroColliders[0] = App->physics->CreateChain(0, 0, contorno, 48);
	tableroColliders[1] = App->physics->CreateChain(0, 0, Triangulomedio, 6);
	tableroColliders[2] = App->physics->CreateChain(0, 0, palos_izq, 12);
	tableroColliders[3] = App->physics->CreateChain(0, 0, palos_der, 12);
	tableroColliders[4] = App->physics->CreateChain(0, 0, Churro, 38);
	tableroColliders[5] = App->physics->CreateChain(0, 0, BrazoDer, 8);
	tableroColliders[6] = App->physics->CreateChain(0, 0, BrazosIzq, 8);

}

void ModuleSceneIntro::createSpring()
{
	// --- Spring Physics ---
	springTop = App->physics->CreateRectangle(645, 745, 30, 10);
	springBot = App->physics->CreateRectangle(645, 800, 30, 10);
	springBot->body->SetType(b2_staticBody);	
	springTop->body->SetFixedRotation(true);
	// Create a Joint to Join the top and the bot from the spring
	b2DistanceJointDef SpringJointDef;

	SpringJointDef.bodyA = springTop->body;
	SpringJointDef.bodyB = springBot->body;

	SpringJointDef.localAnchorA.Set(0, 0);
	SpringJointDef.localAnchorB.Set(0, 0);

	SpringJointDef.length = 1.5f;

	SpringJointDef.collideConnected = true;

	SpringJointDef.frequencyHz = 7.0f;
	SpringJointDef.dampingRatio = 0.05f;

	b2PrismaticJoint* SpringJoint = (b2PrismaticJoint*)App->physics->world->CreateJoint(&SpringJointDef);

	resetBall();
}

void ModuleSceneIntro::createMotor()
{
	pivot_goku = App->physics->CreateCircle(450, 230, 5, false)->body;

	b2RevoluteJointDef revoluteDef;
	revoluteDef.bodyA = pivot_goku;
	gokup = App->physics->CreateRectangle(450, 230, 100, 20);
	revoluteDef.bodyB = gokup->body;
	revoluteDef.collideConnected = false;
	revoluteDef.localAnchorA.Set(0, 0);
	revoluteDef.localAnchorB.Set(0.0, 0);
	revoluteDef.enableMotor = true;
	revoluteDef.motorSpeed = -2.2;
	revoluteDef.maxMotorTorque = 500;

	revolute_jointgoku = (b2RevoluteJoint*)App->physics->world->CreateJoint(&revoluteDef);
}

void ModuleSceneIntro::CreateSensors()
{
	// Pads
	leftPad = App->physics->CreateRectangleSensor(0, 0, 140, 5);
	rightPad = App->physics->CreateRectangleSensor(0, 0, 140, 5);

	b2Vec2 posLeftPad(6, 7.3);	// X, Y, and Angle to aply a rotation.... Idk what i'm doing but it works  || Ahh bueno que son metros y no pixeles y me da palo cambiarlo :D
	rightPad->body->SetTransform(posLeftPad, -1.2);

	b2Vec2 posRightPad(7.1, 7.3);
	leftPad->body->SetTransform(posRightPad, 1.23);

	leftPlat = App->physics->CreateRectangleSensor(0, 0, 40, 5);
	rightPlat = App->physics->CreateRectangleSensor(0, 0, 40, 5);
	b2Vec2 posLPlat(4.3,14.1);
	b2Vec2 posRPlat(8.8, 14);

	leftPlat->body->SetTransform(posLPlat, 0.55f); // who uses radiants having degrees... 90 degrees all the world knows what does it means... but 90 radiants??? wtf are 90 radiants?
	rightPlat->body->SetTransform(posRPlat, -0.6f);

	// Losing a ball sensor
	loseSensor = App->physics->CreateRectangleSensor(326, 1000, 200, 200);
}

void ModuleSceneIntro::resetBall()
{
	PhysBody* circle = App->physics->CreateCircle(645, 700, 15);
	circle->body->SetGravityScale(0.1f);
	circles.add(circle);
	circles.getLast()->data->listener = this;
	App->audio->PlayFx(resp);
}
