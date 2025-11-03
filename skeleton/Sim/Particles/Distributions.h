#ifndef DISTRIBUTIONS_H_
#define DISTRIBUTIONS_H_

#pragma once

#include <random>
#include <algorithm>

struct RNG {
	std::mt19937 rng;
	RNG(unsigned seed = std::random_device{}()) : rng(seed) {}
};

struct Uniform01 {
	std::uniform_real_distribution<float> d{ 0.f, 1.f };
	float operator()(RNG& R) { return d(R.rng); }
};

struct UniformRange {
	float a, b; std::uniform_real_distribution<float> d;
	UniformRange(float A, float B) : a(A), b(B), d(A, B) {}
	float operator()(RNG& R) { return d(R.rng); }
};

struct Normal {
	std::normal_distribution<float> d;
	Normal(float mean, float stddev) : d(mean, stddev) {}
	float operator()(RNG& R) { return d(R.rng); }
};

#endif // DISTRIBUTIONS_H_