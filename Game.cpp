#include "game.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "game_object.h"
#include "ball_object.h"

// Game-related State data
SpriteRenderer  *Renderer;
GameObject      *Player;

// 初始化球的速度
const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);
// 球的半径
const GLfloat BALL_RADIUS = 12.5f;

BallObject     *Ball;


Game::Game(GLuint width, GLuint height) 
	: State(GAME_ACTIVE), Keys(), Width(width), Height(height) 
{ 

}

Game::~Game()
{
    delete Renderer;
    delete Player;
}


void Game::Init()
{


	// Load shaders
	ResourceManager::LoadShader(R"(shaders\sprite.vs)", R"(shaders\sprite.frag)", nullptr, "sprite");
	// Configure shaders
	glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(this->Width), static_cast<GLfloat>(this->Height), 0.0f, -1.0f, 1.0f);
	ResourceManager::GetShader("sprite").Use().SetInteger("sprite", 0);
	ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
	// Load textures
	ResourceManager::LoadTexture(R"(textures/bg.jpg)", GL_FALSE, "background");
	ResourceManager::LoadTexture(R"(textures/awesomeface.png)", GL_TRUE, "face");
	ResourceManager::LoadTexture(R"(textures/box.png)", GL_FALSE, "block");
	ResourceManager::LoadTexture(R"(textures/box.png)", GL_FALSE, "block_solid");
	ResourceManager::LoadTexture(R"(textures/box.png)", true, "paddle");
	// Set render-specific controls
	Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
	// Load levels
	GameLevel one; one.Load(R"(levels/one.lvl)", this->Width, this->Height * 0.5);
	GameLevel two; two.Load(R"(levels/two.lvl)", this->Width, this->Height * 0.5);
	GameLevel three; three.Load(R"(levels/three.lvl)", this->Width, this->Height * 0.5);
	GameLevel four; four.Load(R"(levels/four.lvl)", this->Width, this->Height * 0.5);
	this->Levels.push_back(one);
	this->Levels.push_back(two);
	this->Levels.push_back(three);
	this->Levels.push_back(four);
	this->Level = 0;


	// Configure game objects
	glm::vec2 playerPos = glm::vec2(this->Width / 2 - PLAYER_SIZE.x / 2, this->Height - PLAYER_SIZE.y);
	Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));

	glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2 - BALL_RADIUS, -BALL_RADIUS * 2);
	Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY,
		ResourceManager::GetTexture("face"));
}

void Game::Update(GLfloat dt)
{
	Ball->Move(dt, this->Width);
	this->DoCollisions();
}


void Game::ProcessInput(GLfloat dt)
{
	if (this->State == GAME_ACTIVE)
	{
		GLfloat velocity = PLAYER_VELOCITY * dt;
		// 移动玩家挡板
		if (this->Keys[GLFW_KEY_A])
		{
			if (Player->Position.x >= 0)
			{
				Player->Position.x -= velocity;
				if (Ball->Stuck)
					Ball->Position.x -= velocity;
			}
		}
		if (this->Keys[GLFW_KEY_D])
		{
			if (Player->Position.x <= this->Width - Player->Size.x)
			{
				Player->Position.x += velocity;
				if (Ball->Stuck)
					Ball->Position.x += velocity;
			}
		}
		if (this->Keys[GLFW_KEY_SPACE])
			Ball->Stuck = false;
	}
}

void Game::Render()
{
	//Renderer->DrawSprite((Texture2D&)(ResourceManager::GetTexture("face")), glm::vec2(200, 200), glm::vec2(300, 400), 45.0f, glm::vec3(0.0f, 1.0f, 0.0f));

	if (this->State == GAME_ACTIVE)
	{
		// Draw background
		Renderer->DrawSprite(ResourceManager::GetTexture("background"), glm::vec2(0, 0), glm::vec2(this->Width, this->Height), 0.0f);
		// Draw level
		this->Levels[this->Level].Draw(*Renderer);
		// Draw player
		Player->Draw(*Renderer);
		Ball->Draw(*Renderer);
	}
}

void Game::DoCollisions()
{
	for (GameObject &box : this->Levels[this->Level].Bricks)
	{
		if (!box.Destroyed)
		{
			if (CheckCollision(*Ball, box))
			{
				if (!box.IsSolid)
					box.Destroyed = GL_TRUE;
			}
		}
	}
}

GLboolean Game::CheckCollision(GameObject &one, GameObject &two) // AABB - AABB collision
{
	// x轴方向碰撞？
	bool collisionX = one.Position.x + one.Size.x >= two.Position.x &&
		two.Position.x + two.Size.x >= one.Position.x;
	// y轴方向碰撞？
	bool collisionY = one.Position.y + one.Size.y >= two.Position.y &&
		two.Position.y + two.Size.y >= one.Position.y;
	// 只有两个轴向都有碰撞时才碰撞
	return collisionX && collisionY;
}