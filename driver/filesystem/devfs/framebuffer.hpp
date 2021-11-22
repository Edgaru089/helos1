#pragma once

#include "devfs.hpp"

namespace helos {
namespace filesystem {
namespace device {


// Framebuffer device: fb0, fb1 ~ fbN
class Framebuffer {
public:
	// Init mounts the proper devices: fb0, and N other framebuffer devices.
	static void Init(int fbN);
};


} // namespace device
} // namespace filesystem
} // namespace helos
