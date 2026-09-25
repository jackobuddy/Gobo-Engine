#pragma once

#include <bitset>

// This file stores key information needed by all ECS structures

// This is the maximum number of globally registered component types. 
inline constexpr unsigned int MAX_COMPONENTS = 64;

// We use a bitset to keep track of which components an Entity has and 
// it also helps keep track of which entities a given System is interested in. 
typedef std::bitset<MAX_COMPONENTS> Signature;