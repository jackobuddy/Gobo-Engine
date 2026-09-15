#pragma once
#include "ECS.h"

// Structure to organize the base of all our Components
struct BaseComponent
{
protected:
	// nextID is shared between all Component classes. 
	static int nextID; 
};


// A Component must always be associated with a type of Component. 
template <typename T>
class Component: public BaseComponent
{
public: 
	// Sets and/or returns the unique ID of Component<T>
	static int GetID() 
	{ 
		// Because this value is static, all Components of the same class
		// will share the same ID. For example, all TransformComponents will
		// share the same ID value. 
		static auto id = nextID++; // We increment the nextID value by 1 per new type
		return id; 
	}
};