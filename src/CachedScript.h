#pragma once

//*src
#include "Lexicon/Engine/Project.h"
#include "Lexicon/Interfaces/ProjectManager.h"
namespace LEX
{
	class Script;

	struct cached_script
	{
		static Script* condition()
		{
			static Script* script = nullptr;
			
			if (!script)
			{
				auto shared = ProjectManager::instance->GetShared();

				script = shared->FindScript("Condition");
			}

			return script;
		}
	};
}