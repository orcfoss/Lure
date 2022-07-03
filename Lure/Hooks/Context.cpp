#include "pch.h"

#include "Helpers.h"
#include "Hooks/Context.h"

#ifdef _DEBUG

Context__requirePermission_t Context__requirePermission = (Context__requirePermission_t)ADDRESS_CONTEXT__REQUIREPERMISSION;

void __fastcall Context__requirePermission_hook(void* _this, void*, int permission, const char* operation)
{
	int identity = *(int*)_this;

	if (!Context__isInRole(identity, permission))
	{
		if (operation != 0)
		{
			throw std::runtime_error("The current identity (" + std::to_string(identity) + ") cannot " + std::string(operation) + " (requires " + std::to_string(permission) + ")");
		}
		else
		{
			throw std::runtime_error("The current identity (" + std::to_string(identity) + ") cannot perform the requested operation (requires " + std::to_string(permission) + ")");
		}
	}
}

#endif