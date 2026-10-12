#pragma once

enum class PauseActivity
{
	GameplayOnly, Always, PauseOnly
};

struct PausePolicyComponent
{
	PauseActivity activity = PauseActivity::GameplayOnly; 
};