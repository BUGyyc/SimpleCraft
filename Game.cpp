#include "game.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "game_object.h"
#include "ball_object.h"
//整个游戏的精灵渲染
SpriteRenderer  *Renderer;
//玩家操作的长方形木板
GameObject      *Player;
//游戏中碰撞的球
BallObject      *Ball;
Game::Game(GLuint width, GLuint height)
	: State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{
}
Game::~Game()
{
	delete Renderer;
	delete Player;
	delete Ball;
}
//初始化一些资源，包括贴图、着色器、关卡信息
void Game::Init()
{	
	//着色器
	ResourceManager::LoadShader(R"(shaders\sprite.vs)", R"(shaders\sprite.frag)", nullptr, "sprite");
	glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(this->Width), static_cast<GLfloat>(this->Height), 0.0f, -1.0f, 1.0f);
	ResourceManager::GetShader("sprite").Use().SetInteger("sprite", 0);
	ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
	//贴图资源
	ResourceManager::LoadTexture(R"(textures/bg.jpg)", GL_FALSE, "background");
	ResourceManager::LoadTexture(R"(textures/awesomeface.png)", GL_TRUE, "face");
	ResourceManager::LoadTexture(R"(textures/box.png)", GL_FALSE, "block");
	ResourceManager::LoadTexture(R"(textures/box.png)", GL_FALSE, "block_solid");
	ResourceManager::LoadTexture(R"(textures/box.png)", true, "paddle");
	//着色器传入SpriteRenderer对象内
	Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
	//关卡信息
	GameLevel one; one.Load(R"(levels/one.lvl)", this->Width, this->Height * 0.5);
	GameLevel two; two.Load(R"(levels/two.lvl)", this->Width, this->Height * 0.5);
	GameLevel three; three.Load(R"(levels/three.lvl)", this->Width, this->Height * 0.5);
	GameLevel four; four.Load(R"(levels/four.lvl)", this->Width, this->Height * 0.5);
	this->Levels.push_back(one);
	this->Levels.push_back(two);
	this->Levels.push_back(three);
	this->Levels.push_back(four);
	//起始关卡
	this->Level = 0;
	//玩家的初始位置
	glm::vec2 playerPos = glm::vec2(this->Width / 2 - PLAYER_SIZE.x / 2, this->Height - PLAYER_SIZE.y);
	//设置玩家位置和尺寸
	Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));
	//球的位置
	glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2 - BALL_RADIUS, -BALL_RADIUS * 2);
	//设置球的位置和尺寸
	Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("face"));
}
//每帧调用，更新的内容
void Game::Update(GLfloat dt)
{
	//球移动
	Ball->Move(dt, this->Width);
	//检测球的碰撞
	this->DoCollisions();
	//球出界，重置游戏
	if (Ball->Position.y >= this->Height)
	{
		this->ResetLevel();
		this->ResetPlayer();
	}
}
//输入的监听
void Game::ProcessInput(GLfloat dt)
{
	if (this->State == GAME_ACTIVE)
	{
		GLfloat velocity = PLAYER_VELOCITY * dt;
		//往左移动
		if (this->Keys[GLFW_KEY_A])
		{
			if (Player->Position.x >= 0)
			{
				Player->Position.x -= velocity;
				if (Ball->Stuck)
					Ball->Position.x -= velocity;
			}
		}
		//往右移动
		if (this->Keys[GLFW_KEY_D])
		{
			if (Player->Position.x <= this->Width - Player->Size.x)
			{
				Player->Position.x += velocity;
				if (Ball->Stuck)
					Ball->Position.x += velocity;
			}
		}
		//开始游戏，发射球
		if (this->Keys[GLFW_KEY_SPACE])
			Ball->Stuck = false;
	}
}

//渲染
void Game::Render()
{
	if (this->State == GAME_ACTIVE)
	{
		//画游戏底图背景
		Renderer->DrawSprite(ResourceManager::GetTexture("background"), glm::vec2(0, 0), glm::vec2(this->Width, this->Height), 0.0f);
		//根据关卡信息画图
		this->Levels[this->Level].Draw(*Renderer);
		//玩家木板绘制
		Player->Draw(*Renderer);
		//绘制球
		Ball->Draw(*Renderer);
	}
}
//重置关卡
void Game::ResetLevel()
{
	if (this->Level == 0)this->Levels[0].Load("levels/one.lvl", this->Width, this->Height * 0.5f);
	else if (this->Level == 1)
		this->Levels[1].Load("levels/two.lvl", this->Width, this->Height * 0.5f);
	else if (this->Level == 2)
		this->Levels[2].Load("levels/three.lvl", this->Width, this->Height * 0.5f);
	else if (this->Level == 3)
		this->Levels[3].Load("levels/four.lvl", this->Width, this->Height * 0.5f);
}
//重置玩家
void Game::ResetPlayer()
{
	Player->Size = PLAYER_SIZE;
	Player->Position = glm::vec2(this->Width / 2 - PLAYER_SIZE.x / 2, this->Height - PLAYER_SIZE.y);
	Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2 - BALL_RADIUS, -(BALL_RADIUS * 2)), INITIAL_BALL_VELOCITY);
}

Collision CheckCollision(BallObject &one, GameObject &two);
Direction VectorDirection(glm::vec2 closest);
//执行碰撞
void Game::DoCollisions()
{	
	//遍历所有的方块与球的碰撞情况
	for (GameObject &box : this->Levels[this->Level].Bricks)
	{
		//只有方块存在才检测
		if (!box.Destroyed)
		{
			//检测是否有碰撞
			Collision collision = CheckCollision(*Ball, box);
			//有碰撞
			if (std::get<0>(collision))
			{
				//销毁普通的方块
				if (!box.IsSolid)
					box.Destroyed = GL_TRUE;
				//方向
				Direction dir = std::get<1>(collision);
				glm::vec2 diff_vector = std::get<2>(collision);
				//左右方向的计算
				if (dir == LEFT || dir == RIGHT)
				{
					Ball->Velocity.x = -Ball->Velocity.x; 
					GLfloat penetration = Ball->Radius - std::abs(diff_vector.x);
					if (dir == LEFT)
						Ball->Position.x += penetration;
					else
						Ball->Position.x -= penetration;
				}
				else //上下方向
				{
					Ball->Velocity.y = -Ball->Velocity.y;						  
					GLfloat penetration = Ball->Radius - std::abs(diff_vector.y);
					if (dir == UP)
						Ball->Position.y -= penetration;
					else
						Ball->Position.y += penetration;
				}
			}
		}
	}
	//计算球和玩家碰撞结果
	Collision result = CheckCollision(*Ball, *Player);
	if (!Ball->Stuck && std::get<0>(result))
	{
		GLfloat centerBoard = Player->Position.x + Player->Size.x / 2;
		GLfloat distance = (Ball->Position.x + Ball->Radius) - centerBoard;
		GLfloat percentage = distance / (Player->Size.x / 2);
		GLfloat strength = 2.0f;
		glm::vec2 oldVelocity = Ball->Velocity;
		//更改球的速度方向
		Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength;
		Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity); 																			
		Ball->Velocity.y = -1 * abs(Ball->Velocity.y);
	}
}

//碰撞检测
Collision CheckCollision(BallObject &one, GameObject &two) // AABB - Circle collision
{
	//圆心
	glm::vec2 center(one.Position + one.Radius);
	//矩形中心
	glm::vec2 aabb_half_extents(two.Size.x / 2, two.Size.y / 2);
	glm::vec2 aabb_center(two.Position.x + aabb_half_extents.x, two.Position.y + aabb_half_extents.y);
	//差值向量
	glm::vec2 difference = center - aabb_center;
	//求出矩形上的边界
	glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
	// 得到距离圆心的最近点
	glm::vec2 closest = aabb_center + clamped;
	//最近点到圆心的差值向量
	difference = closest - center;
	//通过半径与间距比较判断是否碰撞
	if (glm::length(difference) < one.Radius) 
		return std::make_tuple(GL_TRUE, VectorDirection(difference), difference);
	else
		return std::make_tuple(GL_FALSE, UP, glm::vec2(0, 0));
}


//计算方向
Direction VectorDirection(glm::vec2 target)
{
	//四个方向
	glm::vec2 compass[] = {
		glm::vec2(0.0f, 1.0f),	// up
		glm::vec2(1.0f, 0.0f),	// right
		glm::vec2(0.0f, -1.0f),	// down
		glm::vec2(-1.0f, 0.0f)	// left
	};
	GLfloat max = 0.0f;
	GLuint best_match = -1;
	//遍历四个方向
	for (GLuint i = 0; i < 4; i++)
	{	
		//求点乘值
		GLfloat dot_product = glm::dot(glm::normalize(target), compass[i]);
		//取最大Cos 最小锐角
		if (dot_product > max)
		{
			max = dot_product;
			best_match = i;
		}
	}
	//返回一个最合适的方向
	return (Direction)best_match;
}