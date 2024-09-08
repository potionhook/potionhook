/*
 * drawmgr.hpp
 *
 *  Created on: May 22, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include <mutex>

extern std::mutex drawing_mutex;

void RenderCheatVisuals();

void BeginCheatVisuals();
void DrawCheatVisuals();
void EndCheatVisuals();
void DrawCache();
