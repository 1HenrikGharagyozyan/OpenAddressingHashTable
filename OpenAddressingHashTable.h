#pragma once

#include <vector>
#include <optional>
#include <utility>
#include <stdexcept>
#include <functional>
#include <unordered_set>

#include "Bucket.h"
#include "ProbingStrategy.h"
#include "LinearProbing.h"

template<
	typename Key,
	typename T = Key,
	typename Hash = std::hash<Key>,
	typename KeyEqual = std::equal_to<Key>,
	typename ProbingStrategy = LinearProbing<Key>,
	bool AllowDuplicates = false
>
class OpenAddressingHashTable
{
public:
	using key_type = Key;
	using mapped_type = T;
	using hasher = Hash;
	using key_equal = KeyEqual;
	using size_type = std::size_t;
	using value_type = std::conditional_t<std::is_same_v<Key, T>, Key, std::pair<const Key, T>>;
	using bucket_type = Bucket<Key, T>;
	using probing_strategy_type = ProbingStrategy;
	using base_probing_strategy_type = IProbingStrategy<Key>;

private:
	std::vector<bucket_type*> _buckets;
	size_type _size = 0;
	float _max_load_factor = 0.75f;

	hasher _hash;
	key_equal _equal;
	base_probing_strategy_type* _probing = nullptr;

public:
	template<bool IsConst>
	class HashIterator
	{
		using bucket_ptr = bucket_type*;
		using bucket_ptr_ptr = std::conditional_t<IsConst, bucket_type* const*, bucket_type**>;
		using value_ref = std::conditional_t<IsConst, const value_type&, value_type&>;
		using value_ptr = std::conditional_t<IsConst, const value_type*, value_type*>;

		bucket_ptr_ptr _current;
		bucket_ptr_ptr _end;

		void skip_to_valid();

	public:
		using iterator_category = std::forward_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = OpenAddressingHashTable::value_type;
		using reference = value_ref;
		using pointer = value_ptr;

		HashIterator();
		HashIterator(bucket_ptr_ptr current, bucket_ptr_ptr end);

		reference operator*() const;
		pointer operator->() const;

		HashIterator& operator++();
		HashIterator operator++(int);

		bool operator==(const HashIterator& rhs) const;
		bool operator!=(const HashIterator& rhs) const;
	};

	using iterator = HashIterator<false>;
	using const_iterator = HashIterator<true>;


	OpenAddressingHashTable(size_type capacity = 16);
	OpenAddressingHashTable(std::initializer_list<value_type> init);
	OpenAddressingHashTable(size_type capacity, const hasher& hash, const key_equal& equal, const ProbingStrategy& strategy);
	OpenAddressingHashTable(const OpenAddressingHashTable& other);
	OpenAddressingHashTable(OpenAddressingHashTable&& other) noexcept;
	~OpenAddressingHashTable();

	OpenAddressingHashTable& operator=(const OpenAddressingHashTable& other);
	OpenAddressingHashTable& operator=(OpenAddressingHashTable&& other) noexcept;

	std::pair<iterator, bool> insert(const value_type& kv);
	std::pair<iterator, bool> insert(value_type&& kv);
	std::pair<iterator, bool> insert(const key_type& key, const mapped_type& value);

	template<typename... Args>
	std::pair<iterator, bool> emplace(Args&&... args);

	template<typename... Args>
	std::pair<iterator, bool> try_emplace(const key_type& key, Args&&... args);

	template<typename M>
	std::pair<iterator, bool> insert_or_assign(const key_type& key, M&& obj);

	size_type erase(const key_type& key);

	void clear();

	mapped_type& operator[](const key_type& key);
	mapped_type& operator[](key_type&& key);

	mapped_type& at(const key_type& key);
	const mapped_type& at(const key_type& key) const;

	iterator find(const key_type& key);
	const_iterator find(const key_type& key) const;

	bool contains(const key_type& key) const;

	std::pair<iterator, iterator> equal_range(const key_type& key);
	std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const;

	size_type count(const key_type& key) const;

	size_type size() const noexcept;
	bool empty() const noexcept;

	size_type capacity() const noexcept;
	size_type bucket_index() const noexcept;

	float load_factor() const noexcept;
	float max_load_factor() const noexcept;
	void max_load_factor(float ml);
	void reserve(size_type n);
	void rehash(size_type new_capacity);

	iterator begin();
	iterator end();
	const_iterator begin() const;
	const_iterator end() const;
	const_iterator cbegin() const;
	const_iterator cend() const;

	void swap(OpenAddressingHashTable& other) noexcept;

	bool operator==(const OpenAddressingHashTable& other) const;
	bool operator!=(const OpenAddressingHashTable& other) const;

	template<typename K, typename M, typename H, typename E, typename P, bool D>
	friend void swap(OpenAddressingHashTable<K, M, H, E, P, D>& lhs, OpenAddressingHashTable<K, M, H, E, P, D>& rhs) noexcept;

private:
	size_type find_index(const key_type& key) const;
	std::pair<size_type, bool> probe_insert_slot(const key_type& key, const size_type& hash_value);
	void check_load_and_rehash();
	const key_type& get_key(const value_type& val) const;
	void allocate_buckets(size_type n);
	void destroy_buckets();
};

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
template<bool IsConst>
inline void OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::HashIterator<IsConst>::skip_to_valid()
{
	while (_current != _end && (!(*_current) || !(*_current)->is_occupied()))
		++_current;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
template<bool IsConst>
inline OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::HashIterator<IsConst>::HashIterator()
	: _current(nullptr)
	, _end(nullptr)
{
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
template<bool IsConst>
inline OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::HashIterator<IsConst>
		::HashIterator(bucket_ptr_ptr current, bucket_ptr_ptr end)
	: _current(current)
	, _end(end)
{
	skip_to_valid();
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
template<bool IsConst>
typename OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::HashIterator<IsConst>::reference
		OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::HashIterator<IsConst>::operator*() const
{
	return (*_current)->value_ref();
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
template<bool IsConst>
typename OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::HashIterator<IsConst>::pointer 
		OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::HashIterator<IsConst>::operator->() const
{
	return &(*_current)->value_ref();
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
template<bool IsConst>
typename OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::HashIterator<IsConst>& 
		OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::HashIterator<IsConst>::operator++()
{
	++_current;
	skip_to_valid();
	return *this;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
template<bool IsConst>
typename OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::HashIterator<IsConst> 
		OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::HashIterator<IsConst>::operator++(int)
{
	HashIterator temp = *this;
	++(*this);
	return temp;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
template<bool IsConst>
inline bool OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::HashIterator<IsConst>
		::operator==(const HashIterator& rhs) const
{
	return _current == rhs._current;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
template<bool IsConst>
inline bool OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::HashIterator<IsConst>
		::operator!=(const HashIterator& rhs) const
{
	return _current != rhs._current;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
inline typename OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::size_type 
		OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::find_index(const key_type& key) const
{
	if (_buckets.empty())
		return static_cast<size_type>(-1);

	const size_type hash = _hash(key);
	const size_type capacity = _buckets.size();
	for (size_type i = 0; i < capacity; ++i)
	{
		size_type index = _probing->probe(key, hash, i, capacity);
		bucket_type* bucket = _buckets[index];

		if (bucket->is_empty())
			return capacity;
		if (bucket->is_occupied() && _equal(bucket->key(), key))
			return index;
	}
	return capacity;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
inline std::pair<typename OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::size_type, bool> 
		OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>
		::probe_insert_slot(const key_type& key, const size_type& hash_value)
{
	size_type first_deleted_index = _buckets.size();
	size_type capacity = _buckets.size();

	for (size_type i = 0; i < capacity; ++i)
	{
		size_type index = _probing->probe(key, hash_value, i, capacity);
		bucket_type* bucket = _buckets[index];

		if (bucket->is_empty())
			return { (first_deleted_index != capacity ? first_deleted_index : index), true };
		else if (bucket->is_deleted())
		{
			if (first_deleted_index == capacity)
				first_deleted_index = index;
		}
		else if (bucket->is_occupied() && _equal(bucket->key(), key))
		{
			if constexpr (AllowDuplicates)
				continue;
			else
				return { index, false }; 
		}
	}

	if (first_deleted_index != capacity)
		return { first_deleted_index, true };

	return { capacity, false };
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
inline void OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::check_load_and_rehash()
{
	if (load_factor() > max_load_factor())
	{
		size_type new_capacity = _buckets.size() * 2;
		rehash(new_capacity);
	}
} 

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
inline const typename OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::key_type&
		OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::get_key(const value_type& val) const
{
	if constexpr (std::is_same_v<key_type, mapped_type>)
		return val;
	else
		return val.first;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
inline void OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::allocate_buckets(size_type n)
{
	_buckets.resize(n);
	for (auto& bucket : _buckets)
		bucket = new bucket_type();
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
inline void OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::destroy_buckets()
{
	for (auto& bucket : _buckets)
	{
		delete bucket;
		bucket = nullptr;
	}
	_buckets.clear();
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
inline OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::OpenAddressingHashTable(size_type capacity)
	: _hash(Hash())
	, _equal(KeyEqual())
	, _size(0)
	, _max_load_factor(0.75f)
	, _probing(new ProbingStrategy())
{
	allocate_buckets(capacity);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
inline OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>
		::OpenAddressingHashTable(std::initializer_list<value_type> init)
	: OpenAddressingHashTable(init.size())
{
	for (const auto& elem : init)
		insert(elem);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
inline OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>
		::OpenAddressingHashTable(size_type capacity, const hasher& hash, const key_equal& equal, const ProbingStrategy& strategy)
	: _hash(hash)
	, _equal(equal)
	, _size(0)
	, _max_load_factor(0.75f)
	, _probing(strategy.clone())
{
	allocate_buckets(capacity);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
inline OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>
		::OpenAddressingHashTable(const OpenAddressingHashTable& other)
	: _hash(other._hash)
	, _equal(other._equal)
	, _size(0)
	, _max_load_factor(other._max_load_factor)
	, _probing(nullptr)
{
	_probing = other._probing ? other._probing->clone() : nullptr;
	allocate_buckets(other._buckets.size());

	for (size_type i = 0; i < other._buckets.size(); ++i)
	{
		if (other._buckets[i] && other._buckets[i]->is_occupied())
		{
			_buckets[i]->set(*other._buckets[i]);
			++_size;
		}
	}
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
inline OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>
		::OpenAddressingHashTable(OpenAddressingHashTable&& other) noexcept
	: _buckets(std::move(other._buckets))
	, _size(other._size)
	, _hash(std::move(other._hash))
	, _equal(std::move(other._equal))
	, _max_load_factor(other._max_load_factor)
	, _probing(other._probing)
{
	other._size = 0;
	other._probing = nullptr;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
inline OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::~OpenAddressingHashTable()
{
	destroy_buckets();
	if (_probing)
	{
		delete _probing;
		_probing = nullptr;
	}
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
inline OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>& 
		OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>
		::operator=(const OpenAddressingHashTable& other)
{
	if (this != &other)
	{
		destroy_buckets();
		if (_probing)
		{
			delete _probing;
			_probing = nullptr;
		}

		_hash = other._hash;
		_equal = other._equal;
		_max_load_factor = other._max_load_factor;
		_size = 0;

		_probing = other._probing ? other._probing->clone() : nullptr;
		allocate_buckets(other._buckets.size());

		for (size_type i = 0; i < other._buckets.size(); ++i)
		{
			if (other._buckets[i] && other._buckets[i]->is_occupied())
			{
				_buckets[i]->set(*other._buckets[i]);
				++_size;
			}
		}
	}
	return *this;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
inline OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>& 
		OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>
		::operator=(OpenAddressingHashTable&& other) noexcept
{
	if (this != &other)
	{
		destroy_buckets();
		if (_probing)
		{
			delete _probing;
			_probing = nullptr;
		}

		_buckets = std::move(other._buckets);
		_hash = std::move(other._hash);
		_equal = std::move(other._equal);
		_max_load_factor = other._max_load_factor;
		_size = other._size;
		_probing = other._probing;

		other._probing = nullptr;
		other._size = 0;
	}
	return *this;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
std::pair<typename OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::iterator, bool> 
		OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::insert(const value_type& kv)
{
	check_load_and_rehash();

	const key_type& key = get_key(kv);
	size_type hash_value = _hash(key);
	auto [index, inserted] = probe_insert_slot(key, hash_value);

	if (index == _buckets.size())
		return { end(), false };

	if (inserted)
	{
		_buckets[index]->set(kv);
		++_size;
	}

	return { iterator(_buckets.data() + index, _buckets.data() + _buckets.size()), inserted };
}  

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
std::pair<typename OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::iterator, bool> 
		OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::insert(value_type&& kv)
{
	check_load_and_rehash();

	const key_type& key = get_key(kv);
	size_type hash_value = _hash(key);
	auto [index, inserted] = probe_insert_slot(key, hash_value);

	if (index == _buckets.size())
		return { end(), false };

	if (inserted)
	{
		_buckets[index]->set(std::move(kv));
		++_size;
	}

	return { iterator(_buckets.data() + index, _buckets.data() + _buckets.size()), inserted };
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
std::pair<typename OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::iterator, bool> 
		OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>
		::insert(const key_type& key, const mapped_type& value)
{
	check_load_and_rehash();
	size_type hash_value = _hash(key);
	auto [index, inserted] = probe_insert_slot(key, hash_value);

	if (index == _buckets.size())
		return { end(), false };

	if (inserted)
	{
		if constexpr (std::is_same_v<Key, T>)
			_buckets[index]->set(key);
		else
			_buckets[index]->set(std::make_pair(key, value));
		++_size;
	}
	return { iterator(_buckets.data() + index, _buckets.data() + _buckets.size()), inserted };
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
template<typename ...Args>
inline std::pair<typename OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::iterator, bool> 
		OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::emplace(Args&&... args)
{
	check_load_and_rehash();

	value_type val(std::forward<Args>(args)...);
	const key_type& key = get_key(val);

	size_type hash_value = _hash(key);
	auto [index, inserted] = probe_insert_slot(key, hash_value);

	if (index == _buckets.size())
		return { end(), false };

	if (inserted)
	{
		_buckets[index]->set(std::move(val));
		++_size;
	}

	return{ iterator(_buckets.data() + index, _buckets.data() + _buckets.size()), inserted };
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
template<typename ...Args>
inline std::pair<typename OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::iterator, bool>
		OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>
		::try_emplace(const key_type& key, Args&&... args)
{
	check_load_and_rehash();

	size_type hash_value = _hash(key);
	auto [index, inserted] = probe_insert_slot(key, hash_value);

	if (index == _buckets.size())
		return { end(), false };

	if (inserted)
	{
		if constexpr (std::is_same_v<Key, T>)
			_buckets[index]->set(key);
		else
			_buckets[index]->emplace(key, std::forward<Args>(args)...);
		++_size;
	}

	return { iterator(_buckets.data() + index, _buckets.data() + _buckets.size()), inserted };
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
template<typename M>
inline std::pair<typename OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::iterator, bool>
		OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>
		::insert_or_assign(const key_type& key, M&& obj)
{
	check_load_and_rehash();

	size_type hash_value = _hash(key);
	auto [index, inserted] = probe_insert_slot(key, hash_value);

	if (index == _buckets.size())
		return { end(), false };

	if (inserted)
	{
		if constexpr (std::is_same_v<Key, T>)
			_buckets[index]->set(key);
		else
			_buckets[index]->set(std::make_pair(key, std::forward<M>(obj)));
		++_size;
	}
	else
	{
		if constexpr (!std::is_same_v<Key, T>)
			_buckets[index]->get_mapped() = std::forward<M>(obj);
	}
	
	return { iterator(_buckets.data() + index, _buckets.data() + _buckets.size()), inserted };
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
typename OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::size_type 
		OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::erase(const key_type& key)
{
	size_type index = find_index(key);
	if (index == _buckets.size() || !_buckets[index] || !_buckets[index]->is_occupied())
		return 0;

	_buckets[index]->make_deleted();
	--_size;
	return 1;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
void OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::clear()
{
	for (auto* bucket : _buckets)
	{
		if (bucket)
			bucket->clear();
	}
	_size = 0;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
typename OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::mapped_type& 
		OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::operator[](const key_type& key)
{
	check_load_and_rehash();

	size_type hash_value = _hash(key);
	auto [index, inserted] = probe_insert_slot(key, hash_value);
	bucket_type* bucket = _buckets[index];

	if (inserted)
	{
		bucket->make_occupied(std::pair<const key_type, mapped_type>(key, mapped_type()));
		++_size;
	}
	return bucket->get_mapped();
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
typename OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::mapped_type& 
		OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::operator[](key_type&& key)
{
	check_load_and_rehash();

	size_type hash_value = _hash(key);
	auto [index, inserted] = probe_insert_slot(key, hash_value);
	bucket_type* bucket = _buckets[index];

	if (inserted)
	{
		bucket->make_occupied(std::pair<const key_type, mapped_type>(std::move(key), mapped_type()));
		++_size;
	}
	return bucket->get_mapped();
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
typename OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::mapped_type& 
		OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::at(const key_type& key)
{
	size_type index = find_index(key);
	if (index == _buckets.size() || !_buckets[index]->is_occupied())
		throw std::out_of_range("Key not found");
	return _buckets[index]->get_mapped();
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
const typename OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::mapped_type& 
		OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::at(const key_type& key) const
{
	size_type index = find_index(key);
	if (index == _buckets.size() || !_buckets[index]->is_occupied())
		throw std::out_of_range("Key not found");
	return _buckets[index]->get_mapped();
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
typename OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::iterator 
		OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::find(const key_type& key)
{
	size_type index = find_index(key);
	if (index == _buckets.size() || !_buckets[index] || !_buckets[index]->is_occupied())
		return end();
	return iterator(_buckets.data() + index, _buckets.data() + _buckets.size());
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
typename OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::const_iterator 
		OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::find(const key_type& key) const
{
	size_type index = find_index(key);
	if (index == _buckets.size() || !_buckets[index]->is_occupied())
		return cend();
	return const_iterator(_buckets.data() + index, _buckets.data() + _buckets.size());
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
bool OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::contains(const key_type& key) const
{
	size_type index = find_index(key);
	return index != _buckets.size() && _buckets[index]->is_occupied();
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
std::pair<typename OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::iterator, 
		typename OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::iterator> 
		OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::equal_range(const key_type& key)
{
	if constexpr (!AllowDuplicates)
	{
		auto it = find(key);
		return it == end() ? std::make_pair(it, it) : std::make_pair(it, std::next(it));
	}
	else
	{
		auto begin_it = find(key);
		if (begin_it == end())
			return { end(), end() };

		auto it = begin_it;
		while (it != end() && _equal(get_key(*it), key))
			++it;

		return { begin_it, it };
	}
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
std::pair<typename OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::const_iterator, 
		typename OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::const_iterator> 
		OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::equal_range(const key_type& key) const
{
	if constexpr (!AllowDuplicates)
	{
		auto it = find(key);
		return it == end() ? std::make_pair(it, it) : std::make_pair(it, std::next(it));
	}
	else
	{
		auto begin_it = find(key);
		if (begin_it == end())
			return { end(), end() };

		auto it = begin_it;
		while (it != end() && _equal(get_key(*it), key))
			++it;

		return { begin_it, it };
	}
} 

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
typename OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::size_type 
		OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::count(const key_type& key) const
{
	if constexpr (!AllowDuplicates)
		return contains(key) ? 1 : 0;
	else
	{
		size_type result = 0;
		for (const auto& bucket : _buckets)
		{
			if (bucket && bucket->is_occupied() && _equal(bucket->key(), key))
				++result;
		}
		return result;
	}
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
typename OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::size_type 
		OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::size() const noexcept
{
	return _size;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
bool OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::empty() const noexcept
{
	return _size == 0;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
typename OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::size_type 
		OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::capacity() const noexcept
{
	return _buckets.size();
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
inline typename OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::size_type 
		OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::bucket_index() const noexcept
{
	
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
float OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::load_factor() const noexcept
{
	return _buckets.empty() ? 0.0f : static_cast<float>(_size) / static_cast<float>(_buckets.size());
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
float OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::max_load_factor() const noexcept
{
	return _max_load_factor;
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
void OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::max_load_factor(float ml)
{
	if (ml <= 0.0f || ml > 1.0f)
		throw std::invalid_argument("max_load_factor must be in (0, 1]");
	_max_load_factor = ml;
	check_load_and_rehash();
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
void OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::reserve(size_type n)
{
	if (n > _buckets.size())
		rehash(n);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
void OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::rehash(size_type new_capacity)
{
	std::vector<bucket_type*> old_buckets = std::move(_buckets);
	size_type old_size = _size;

	destroy_buckets();
	allocate_buckets(new_capacity);

	_size = 0;

	for (auto bucket : old_buckets)
	{
		if (bucket && bucket->is_occupied())
		{
			const auto& val = bucket->value();
			const key_type& key = get_key(val);
			size_type hash_value = _hash(key);

			auto [index, inserted] = probe_insert_slot(key, hash_value);
			if (inserted)
			{
				_buckets[index]->set(val);
				++_size;
			}
		}
		delete bucket; // очищаем старый бакет
	}
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
typename OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::iterator 
		OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::begin()
{
	return iterator(_buckets.data(), _buckets.data() + _buckets.size());
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
typename OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::iterator 
		OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::end()
{
	auto end_ptr = _buckets.data() + _buckets.size();
	return iterator(end_ptr, end_ptr);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
typename OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::const_iterator 
		OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::begin() const
{
	return const_iterator(_buckets.data(), _buckets.data() + _buckets.size());
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
typename OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::const_iterator 
		OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::end() const
{
	auto end_ptr = _buckets.data() + _buckets.size();
	return const_iterator(end_ptr, end_ptr);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
typename OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::const_iterator 
		OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::cbegin() const
{
	return const_iterator(_buckets.data(), _buckets.data() + _buckets.size());
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
typename OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::const_iterator 
		OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>::cend() const
{
	auto end_ptr = _buckets.data() + _buckets.size();
	return const_iterator(end_ptr, end_ptr);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
inline void OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>
		::swap(OpenAddressingHashTable& other) noexcept
{
	std::swap(_buckets, other._buckets);
	std::swap(_size, other._size);
	std::swap(_max_load_factor, other._max_load_factor);
	std::swap(_hash, other._hash);
	std::swap(_equal, other._equal);
	std::swap(_probing, other._probing);
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
inline bool OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>
		::operator==(const OpenAddressingHashTable& other) const
{
	if (_size != other._size)
		return false;

	if constexpr (!AllowDuplicates)
	{
		for (const auto& kv : *this)
		{
			const key_type& key = get_key(kv);
			auto it = other.find(key);
			if (it == other.end() || *it != kv)
				return false;
		}
		return true;
	}
	else
	{
		std::unordered_multiset<value_type> this_set, other_set;
		for (const auto& x : *this) 
			this_set.insert(x);
		for (const auto& x : other) 
			other_set.insert(x);
		return this_set == other_set;
	}
}

template<typename Key, typename T, typename Hash, typename KeyEqual, typename ProbingStrategy, bool AllowDuplicates>
inline bool OpenAddressingHashTable<Key, T, Hash, KeyEqual, ProbingStrategy, AllowDuplicates>
		::operator!=(const OpenAddressingHashTable& other) const
{
	return !(*this == other);
}

template<typename K, typename M, typename H, typename E, typename P, bool D>
inline void swap(OpenAddressingHashTable<K, M, H, E, P, D>& lhs, OpenAddressingHashTable<K, M, H, E, P, D>& rhs) noexcept
{
	lhs.swap(rhs);
}
