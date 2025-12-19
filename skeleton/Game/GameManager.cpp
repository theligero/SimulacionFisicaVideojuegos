#include "GameManager.h"

#include <iostream>

GameManager::GameManager() :
	_score(0), _ammo(10), _timeLeft(60.0), _currentState(GameState::MENU)
{
}

void GameManager::Update(double dt)
{
	if (_currentState == GameState::PLAYING) {
		_timeLeft -= dt;

		if (_timeLeft <= 0.0) {
			_timeLeft = 0.0;
			_currentState = GameState::GAME_OVER;
		}
	}
}

void GameManager::OnShotFired()
{
	if (_ammo > 0) _ammo--;
}

void GameManager::OnDuckHit()
{
	_score += 100;
}

int GameManager::GetScore() const
{
	return _score;
}

int GameManager::GetAmmo() const
{
	return _ammo;
}

double GameManager::GetTimeLeft() const
{
	return _timeLeft;
}

GameState GameManager::GetState() const
{
	return _currentState;
}

std::string GameManager::GetStateMessage() const
{
	switch (_currentState) {
	case GameState::MENU:
		return "TIRO AL PATO - PULSA ESPACIO PARA EMPEZAR";
		break;
	case GameState::GAME_OVER:
		return "FIN DEL JUEGO - SCORE: " + std::to_string(_score) + " (R para reiniciar)";
		break;
	case GameState::PLAYING:
	default:
		return ""; // sin mensaje central mientras se juega
	}
}

void GameManager::StartGame()
{
	_score = 0;
	_ammo = 999;
	_timeLeft = 60.0;
	_currentState = GameState::PLAYING;
}
