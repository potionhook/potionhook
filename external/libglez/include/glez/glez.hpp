/*
  Created by Jenny White on 30.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#pragma once

namespace glez
{

void preInit();
void init(int width, int height);
void shutdown();

void resize(int width, int height);

void begin();
void end();
}; // namespace glez