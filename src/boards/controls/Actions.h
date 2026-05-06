#pragma once

#include <functional>

typedef enum
{
  NONE,
  UP,
  DOWN,
  SELECT,
  BACK,
  LAST_INTERACTION
} UIAction;

typedef std::function<void(UIAction)> ActionCallback_t;
