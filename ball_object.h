/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#ifndef BALLOBJECT_H
#define BALLOBJECT_H

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "texture.h"
#include "sprite_renderer.h"
#include "game_object.h"

// BallObject holds the state of the Ball object inheriting
// relevant state data from GameObject. Contains some extra
// functionality specific to Breakout's ball object that
// were too specific for within GameObject alone.
class BallObject : public GameObject
{
public:
	// Ball state	
	GLfloat   Radius;
	GLboolean Stuck;
	// Constructor(s)
	BallObject();
	BallObject(glm::vec2 pos, GLfloat radius, glm::vec2 velocity, Texture2D sprite);
	// Moves the ball, keeping it constrained within the window bounds (except bottom edge); returns new position
	glm::vec2 Move(GLfloat dt, GLuint window_width);
	// Resets the ball to original state with given position and velocity
	void      Reset(glm::vec2 position, glm::vec2 velocity);
};

#endif