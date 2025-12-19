#ifndef GAME_MANAGER_H_
#define GAME_MANAGER_H_

#pragma once

#include <string>

enum class GameState { MENU, PLAYING, GAME_OVER };

class GameManager {
public:
	GameManager();

	// Control de flujo
	void Update(double dt);
	void OnShotFired();
	void OnDuckHit();

	// Getters para pintar textos
	int GetScore() const;
	int GetAmmo() const;
	double GetTimeLeft() const;
	GameState GetState() const;
	std::string GetStateMessage() const;

	void StartGame();

private:
	int _score;
	int _ammo;
	double _timeLeft;
	GameState _currentState;
};

#endif // GAME_MANAGER_H_