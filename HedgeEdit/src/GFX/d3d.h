#pragma once
#define SAFE_RELEASE(ptr) if (ptr) { ptr->Release(); ptr = nullptr; }
