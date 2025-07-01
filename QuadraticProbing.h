#pragma once

#include "ProbingStrategy.h"

template<typename Key>
class QuadraticProbing : public IProbingStrategy<Key>
{
private:
	std::size_t _c1;
	std::size_t _c2;
public:
	QuadraticProbing(std::size_t c1 = 1, std::size_t c2 = 3)
		: _c1(c1)
		, _c2(c2)
	{
	}

	std::size_t probe(const Key& /*key*/, std::size_t hash, std::size_t attempt, std::size_t capacity) const override
	{
		return (hash + _c1 * attempt + _c2 * attempt * attempt) % capacity;
	}

	IProbingStrategy<Key>* clone() const override
	{
		return new QuadraticProbing<Key>(_c1, _c2);
	}
};