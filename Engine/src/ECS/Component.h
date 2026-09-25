#pragma once

#include "ECS.h"
#include <stdexcept>

// Structure to organize the base of all our Components
struct BaseComponent
{
protected:
	// nextID is shared between all Component classes. 
	static int nextID;
	static int AllocateTypeID()
	{
		if (nextID >= static_cast<int>(MAX_COMPONENTS))
			throw std::runtime_error("Component type limit exceeded");
		return nextID++; 
	}
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
		static const int id = AllocateTypeID(); 
		return id; 
	}
};