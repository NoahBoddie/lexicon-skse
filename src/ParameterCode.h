#pragma once

namespace LEX
{
	struct ParameterCode
	{
		union
		{
			struct
			{
				uint32_t code;
				int32_t data;
			};

			size_t raw;
		};
		
		constexpr ParameterCode(uint32_t c, int32_t d = 0) noexcept : code{ c }, data{ d } {}
		constexpr ParameterCode(void* data) noexcept : raw{ std::bit_cast<size_t>(data) } {}

		constexpr ParameterCode InsertData(int32_t d) const noexcept
		{
			ParameterCode result{ *this };
			result.data = d;
			return result;
		}

		operator void* () const
		{
			return reinterpret_cast<void*>(raw);
		}
	};
}