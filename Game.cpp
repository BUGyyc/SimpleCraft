#include "game.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "game_object.h"


// Game-related State data
SpriteRenderer  *Renderer;
GameObject      *Player;


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
	ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
	ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);

	// Load textures
	ResourceManager::LoadTexture(R"(textures\awesomeface.png)", GL_TRUE, "face");

	// Set render-specific controls
	Renderer = new SpriteRenderer((Shader&)ResourceManager::GetShader("sprite"));
}

void Game::Update(GLfloat dt)
{

}


void Game::ProcessInput(GLfloat dt)
{

}

void Game::Render()
{
	Renderer->DrawSprite((Texture2D&)(ResourceManager::GetTexture("face")), glm::vec2(200, 200), glm::vec2(300, 400), 45.0f, glm::vec3(0.0f, 1.0f, 0.0f));
}