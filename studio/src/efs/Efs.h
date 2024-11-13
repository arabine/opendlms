#ifndef EFS_H_
#define EFS_H_

#include <string>
#include <cstdint>

struct Efs
{
	char *name;
	const std::uint8_t *ptr;
	std::uint32_t size;
};

bool GetFile(const std::string &name, Efs &efs);

#endif // EFS_H_
