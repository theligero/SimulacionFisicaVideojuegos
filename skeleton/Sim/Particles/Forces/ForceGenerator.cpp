#include "ForceGenerator.h"

void ForceGenerator::SetEnabled(bool e)
{
	_enabled = e;
}

bool ForceGenerator::Enabled() const
{
	return _enabled;
}
