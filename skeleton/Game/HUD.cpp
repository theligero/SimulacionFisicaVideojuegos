#include "HUD.h"

#include "../Render/Render.h"

void HUD::Draw(int score, double time, const std::string& msg)
{
	// Texto esquina (puntuación)
	Snippets::drawText("Score: " + std::to_string(score), 0, 500);
	Snippets::drawText("Time: " + std::to_string((int)time), 0, 485);

	// Texto central (mensajes importantes)
	if (!msg.empty()) {
		Snippets::drawText(msg, 175, 250);
	}
}
