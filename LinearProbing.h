#pragma once

#include "ProbingStrategy.h"
#include <functional>

template<typename Key>
class LinearProbing : public IProbingStrategy<Key>
{
public:
	std::size_t probe(const Key& /*key*/, std::size_t hash, std::size_t attempt, std::size_t capacity) const override 
	{
		return (hash + attempt) % capacity;
	} 

	IProbingStrategy<Key>* clone() const override
	{
		return new LinearProbing<Key>(*this);
	}
};