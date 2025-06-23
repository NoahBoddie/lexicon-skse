#pragma once

namespace LEX
{

	
	Voidable previousResult{};

	double noCall = std::nan("1BADCA11");
	double noReturn = std::nan("1BADDEED");

	

	//These are probably going to be free at all times.
	thread_local double consoleReturn = noReturn;
	//thread_local uint32_t timestamp = 0;

	void ClearRetValue()
	{
		consoleReturn = noCall;
	}

	bool IsNoReturnValue()
	{
		return std::memcmp(&consoleReturn, &noReturn, 8) == 0;
		return consoleReturn == noReturn;
	}
	bool ConsoleWasCalled()
	{
		return std::memcmp(&consoleReturn, &noCall, 8) != 0;
		return consoleReturn != noCall;
	}

}