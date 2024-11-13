
#include <list>
#include <string>
#include <cstdint>
#include <Windows.h>

class Serial
{
public:


	bool ListComPorts();

	std::list<std::uint8_t> GetList() { return mList; }

private:

	void CheckNamePort(LPTSTR lpstrPortName);

	std::list<std::uint8_t> mList;
};

