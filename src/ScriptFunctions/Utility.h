#pragma once

namespace LEX::REG
{
	struct KeyedCache
	{
		KeyedCache(const KeyedCache&) = delete;
		KeyedCache(KeyedCache&&) = delete;
		KeyedCache& operator=(const KeyedCache&) = delete;
		KeyedCache& operator=(KeyedCache&&) = delete;

		KeyedCache()
		{
			cacheList.push_back(this);
		}

		~KeyedCache()
		{
			//This should NOT be removed, but this is being set up just in case
			std::erase(cacheList, this);

		}

		struct KeyResult
		{
			KeyedCache& cache;

			template <class T>
			inline void AddHash(const T& v) {
				std::hash<T> hasher;
				size_t seed = cache.delayKey;
				seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			}

			template <class T>
			KeyResult& operator<<(const T& key)
			{
				AddHash(key);

				return *this;
			}

			operator bool()//Should update
			{
				return cache.cacheKey != cache.delayKey;
			}
			//Only exists
		};



		static void ClearCache()
		{
			for (auto cache : cacheList)
			{
				cache->cacheKey = cache->delayKey = 0;
			}
		}
		



		template <class T>
		KeyResult operator<<(const T& key)
		{
			KeyResult result{ *this };

			result.AddHash(key);

			return result;
		}


		inline static std::vector<KeyedCache*> cacheList;

		size_t cacheKey = 0;
		size_t delayKey = 0;//If a value is set, this overrides the other

		
	};


	template <typename T>
	struct ValueCache : public KeyedCache
	{
		//Ideally this should use trival kinds of things.
		T value{};

		//bool GetCache()
		

		ValueCache& operator=(const T& new_val)
		{
			cacheKey = delayKey;
			value = new_val;
			return *this;
		}

		ValueCache& operator=(T&& new_val)
		{
			cacheKey = delayKey;
			value = std::move(new_val);
			return *this;
		}

		operator T()
		{
			return value;
		}
	};



	inline int ExampleTest(int* ptr)
	{
		std::pair<size_t, size_t> test;

		static ValueCache<int> cache;

		if (cache << ptr << true)
		{
			cache = ptr ? *ptr : -1;
		}

		return cache;
	}
}