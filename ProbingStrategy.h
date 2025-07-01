#pragma once

#include <cstddef>

template<typename Key>
class IProbingStrategy
{
public:
	virtual ~IProbingStrategy() = default;

	virtual std::size_t probe(const Key& key, std::size_t hash, std::size_t attempt, std::size_t capacity) const = 0;

	virtual IProbingStrategy* clone() const = 0;
};