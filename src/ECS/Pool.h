#pragma once

#include <optional>
#include <vector>

// Abstract class used to point to a Pool without specifying a type. 
class BasePool 
{
public:
	virtual ~BasePool() {}

	// Removes an iten from the Pool
	virtual void Remove(int entityID) = 0; 
};


template <typename T>
class Pool : public BasePool
{
public:
	// Creates a new Pool. The default size of a new Pool is 100. 
	Pool(int size = 0) { Resize(size); }
	~Pool() = default;

	// Returns true if the Pool is empty, otherwise returns false.
	bool IsEmpty() const { return data.empty(); }
	// Returns the size of the Pool as an integer.
	int GetSize() const { return static_cast<int>(data.size()); }
	// Sets the size of the Pool to the given size. 
	void Resize(int size) { data.resize(size); }
	// Clears the contents of the Pool. 
	void Clear() { data.clear(); }
	// Adds a new object to the Pool of type T. 
	void Add(T object) { data.push_back(object); }
	// Removes an iten from the Pool
	void Remove(int entityID) override { data.at(entityID).reset(); }
	// Sets a certain index of the Pool to the given object of type T. 
	void Set(int index, T object) { data[index] = object; }
	// Gets the object of type T at the given index. 
	// T& Get(int index) { return static_cast<T&>(data[index]); }
	T* Get(int entityID) 
	{ 
		if (entityID < 0 or static_cast<size_t>(entityID) >= data.size() or
			not data[entityID].has_value())
				return nullptr; 
		return &data[entityID].value();
	}
	const T* Get(int entityID) const
	{
		if (entityID < 0 or static_cast<size_t>(entityID) >= data.size() or
			not data[entityID].has_value())
			return nullptr;
		return &data[entityID].value();
	}
	// Returns the contents of the given index from the Pool. 
	T& operator [](unsigned int index) { return data[index]; }


private:
	// The core of a Pool is a simple vector, but we use Pool as a wrapper to 
	// make accessing the information within smarter
	std::vector<std::optional<T>> data; 
};