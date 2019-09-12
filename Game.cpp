#include "game.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "game_object.h"
#include "ball_object.h"
//������Ϸ�ľ�����Ⱦ
SpriteRenderer  *Renderer;
//��Ҳ����ĳ�����ľ��
GameObject      *Player;
//��Ϸ����ײ����
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
//��ʼ��һЩ��Դ��������ͼ����ɫ�����ؿ���Ϣ
void Game::Init()
{	
	//��ɫ��
	ResourceManager::LoadShader(R"(shaders\sprite.vs)", R"(shaders\sprite.frag)", nullptr, "sprite");
	glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(this->Width), static_cast<GLfloat>(this->Height), 0.0f, -1.0f, 1.0f);
	ResourceManager::GetShader("sprite").Use().SetInteger("sprite", 0);
	ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
	//��ͼ��Դ
	ResourceManager::LoadTexture(R"(textures/bg.jpg)", GL_FALSE, "background");
	ResourceManager::LoadTexture(R"(textures/awesomeface.png)", GL_TRUE, "face");
	ResourceManager::LoadTexture(R"(textures/box.png)", GL_FALSE, "block");
	ResourceManager::LoadTexture(R"(textures/box.png)", GL_FALSE, "block_solid");
	ResourceManager::LoadTexture(R"(textures/box.png)", true, "paddle");
	//��ɫ������SpriteRenderer������
	Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
	//�ؿ���Ϣ
	GameLevel one; one.Load(R"(levels/one.lvl)", this->Width, this->Height * 0.5);
	GameLevel two; two.Load(R"(levels/two.lvl)", this->Width, this->Height * 0.5);
	GameLevel three; three.Load(R"(levels/three.lvl)", this->Width, this->Height * 0.5);
	GameLevel four; four.Load(R"(levels/four.lvl)", this->Width, this->Height * 0.5);
	this->Levels.push_back(one);
	this->Levels.push_back(two);
	this->Levels.push_back(three);
	this->Levels.push_back(four);
	//��ʼ�ؿ�
	this->Level = 0;
	//��ҵĳ�ʼλ��
	glm::vec2 playerPos = glm::vec2(this->Width / 2 - PLAYER_SIZE.x / 2, this->Height - PLAYER_SIZE.y);
	//�������λ�úͳߴ�
	Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));
	//���λ��
	glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2 - BALL_RADIUS, -BALL_RADIUS * 2);
	//�������λ�úͳߴ�
	Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("face"));
}
//ÿ֡���ã����µ�����
void Game::Update(GLfloat dt)
{
	//���ƶ�
	Ball->Move(dt, this->Width);
	//��������ײ
	this->DoCollisions();
	//����磬������Ϸ
	if (Ball->Position.y >= this->Height)
	{
		this->ResetLevel();
		this->ResetPlayer();
	}
}
//����ļ���
void Game::ProcessInput(GLfloat dt)
{
	if (this->State == GAME_ACTIVE)
	{
		GLfloat velocity = PLAYER_VELOCITY * dt;
		//�����ƶ�
		if (this->Keys[GLFW_KEY_A])
		{
			if (Player->Position.x >= 0)
			{
				Player->Position.x -= velocity;
				if (Ball->Stuck)
					Ball->Position.x -= velocity;
			}
		}
		//�����ƶ�
		if (this->Keys[GLFW_KEY_D])
		{
			if (Player->Position.x <= this->Width - Player->Size.x)
			{
				Player->Position.x += velocity;
				if (Ball->Stuck)
					Ball->Position.x += velocity;
			}
		}
		//��ʼ��Ϸ��������
		if (this->Keys[GLFW_KEY_SPACE])
			Ball->Stuck = false;
	}
}

//��Ⱦ
void Game::Render()
{
	if (this->State == GAME_ACTIVE)
	{
		//����Ϸ��ͼ����
		Renderer->DrawSprite(ResourceManager::GetTexture("background"), glm::vec2(0, 0), glm::vec2(this->Width, this->Height), 0.0f);
		//���ݹؿ���Ϣ��ͼ
		this->Levels[this->Level].Draw(*Renderer);
		//���ľ�����
		Player->Draw(*Renderer);
		//������
		Ball->Draw(*Renderer);
	}
}
//���ùؿ�
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
//�������
void Game::ResetPlayer()
{
	Player->Size = PLAYER_SIZE;
	Player->Position = glm::vec2(this->Width / 2 - PLAYER_SIZE.x / 2, this->Height - PLAYER_SIZE.y);
	Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2 - BALL_RADIUS, -(BALL_RADIUS * 2)), INITIAL_BALL_VELOCITY);
}

Collision CheckCollision(BallObject &one, GameObject &two);
Direction VectorDirection(glm::vec2 closest);
//ִ����ײ
void Game::DoCollisions()
{	
	//�������еķ����������ײ���
	for (GameObject &box : this->Levels[this->Level].Bricks)
	{
		//ֻ�з�����ڲż��
		if (!box.Destroyed)
		{
			//����Ƿ�����ײ
			Collision collision = CheckCollision(*Ball, box);
			//����ײ
			if (std::get<0>(collision))
			{
				//������ͨ�ķ���
				if (!box.IsSolid)
					box.Destroyed = GL_TRUE;
				//����
				Direction dir = std::get<1>(collision);
				glm::vec2 diff_vector = std::get<2>(collision);
				//���ҷ���ļ���
				if (dir == LEFT || dir == RIGHT)
				{
					Ball->Velocity.x = -Ball->Velocity.x; 
					GLfloat penetration = Ball->Radius - std::abs(diff_vector.x);
					if (dir == LEFT)
						Ball->Position.x += penetration;
					else
						Ball->Position.x -= penetration;
				}
				else //���·���
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
	//������������ײ���
	Collision result = CheckCollision(*Ball, *Player);
	if (!Ball->Stuck && std::get<0>(result))
	{
		GLfloat centerBoard = Player->Position.x + Player->Size.x / 2;
		GLfloat distance = (Ball->Position.x + Ball->Radius) - centerBoard;
		GLfloat percentage = distance / (Player->Size.x / 2);
		GLfloat strength = 2.0f;
		glm::vec2 oldVelocity = Ball->Velocity;
		//��������ٶȷ���
		Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength;
		Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity); 																			
		Ball->Velocity.y = -1 * abs(Ball->Velocity.y);
	}
}

//��ײ���
Collision CheckCollision(BallObject &one, GameObject &two) // AABB - Circle collision
{
	//Բ��
	glm::vec2 center(one.Position + one.Radius);
	//��������
	glm::vec2 aabb_half_extents(two.Size.x / 2, two.Size.y / 2);
	glm::vec2 aabb_center(two.Position.x + aabb_half_extents.x, two.Position.y + aabb_half_extents.y);
	//��ֵ����
	glm::vec2 difference = center - aabb_center;
	//��������ϵı߽�
	glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
	// �õ�����Բ�ĵ������
	glm::vec2 closest = aabb_center + clamped;
	//����㵽Բ�ĵĲ�ֵ����
	difference = closest - center;
	//ͨ���뾶����Ƚ��ж��Ƿ���ײ
	if (glm::length(difference) < one.Radius) 
		return std::make_tuple(GL_TRUE, VectorDirection(difference), difference);
	else
		return std::make_tuple(GL_FALSE, UP, glm::vec2(0, 0));
}


//���㷽��
Direction VectorDirection(glm::vec2 target)
{
	//�ĸ�����
	glm::vec2 compass[] = {
		glm::vec2(0.0f, 1.0f),	// up
		glm::vec2(1.0f, 0.0f),	// right
		glm::vec2(0.0f, -1.0f),	// down
		glm::vec2(-1.0f, 0.0f)	// left
	};
	GLfloat max = 0.0f;
	GLuint best_match = -1;
	//�����ĸ�����
	for (GLuint i = 0; i < 4; i++)
	{	
		//����ֵ
		GLfloat dot_product = glm::dot(glm::normalize(target), compass[i]);
		//ȡ���Cos ��С���
		if (dot_product > max)
		{
			max = dot_product;
			best_match = i;
		}
	}
	//����һ������ʵķ���
	return (Direction)best_match;
}