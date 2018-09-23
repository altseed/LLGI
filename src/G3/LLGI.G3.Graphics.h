
#pragma once

#include "../LLGI.Base.h"

namespace LLGI
{
namespace G3
{

class Graphics
	: public ReferenceObject
{
private:
public:
	Graphics() = default;
	virtual ~Graphics() = default;

	virtual void Execute(CommandList* commandList);

	virtual CommandList* CreateCommandList();
};

}
}