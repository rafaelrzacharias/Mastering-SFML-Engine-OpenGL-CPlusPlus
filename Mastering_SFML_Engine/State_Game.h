#pragma once
#include <memory>
#include <SFML/Graphics.hpp>
#include "BaseState.h"
#include "Map.h"
#include "EventManager.h"

class State_Game : public BaseState{
public:
	State_Game(StateManager* l_stateManager);
	~State_Game();

	void OnCreate();
	void OnDestroy();

	void Activate();
	void Deactivate();

	void Update(const sf::Time& l_time);
	void Draw();

	void MainMenu(EventDetails* l_details);
	void PlayerMove(EventDetails* l_details);

	// Debug:
	void ApplyForce(EventDetails* l_details);
	void Explosion(EventDetails* l_details);
private:
	void UpdateCamera();
	int m_player;
};