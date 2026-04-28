#pragma once

namespace LEX
{
	struct Parameter
	{
		static constexpr size_t MAX_SIZE = 10;

		enum Setting
		{
			kRequired,   //Required it's filled out.
			kDefaulted,  //Defaults the type
			kOptional,   //Parameter is optional on the other side
		};

		TypeInfo* type = nullptr;
		Setting settings = kRequired;


		bool IsOptional() const
		{
			return settings != Setting::kRequired;
		}
	};


}