#include "Entities/Actor.h"
#include <Components/SpriteComponent.h>
#include <Components/AnimationComponent.h>

class Radar : public Actor
{
public:
	using Actor::Actor; 

	virtual void OnCreated() override
	{
		AddComponent<SpriteComponent>("radar-image", 64, 64, 2, 255, true);
		AddComponent<AnimationComponent>(8, 5, true);
	}
};