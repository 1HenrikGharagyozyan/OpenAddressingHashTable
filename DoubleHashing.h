#pragma once

#include "ProbingStrategy.h"
#include <functional>

template<typename Key>
class DoubleHashing : public IProbingStrategy<Key>
{
private:
	std::size_t _secondary_prime;

public:
	explicit DoubleHashing(std::size_t secondary_prime = 97)
		: _secondary_prime(secondary_prime)
	{
	}

	std::size_t probe(const Key& key, std::size_t hash, std::size_t attempt, std::size_t capacity) const override
	{
		std::size_t hash2 = _secondary_prime - (std::hash<Key>{}(key) % _secondary_prime);
		return (hash + attempt * hash2) % capacity;
	}

	IProbingStrategy<Key>* clone() const override
	{
		return new DoubleHashing<Key>(*this);
	}
};