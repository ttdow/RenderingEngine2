#include "pch.h"

#include "RenderingEngine.h"

int main(int argc, char* argv[])
{
	uint32_t c = 256;
	std::cout << (c >> 1) << std::endl;
	std::cout << (c >> 2) << std::endl;
	std::cout << (c << 1) << std::endl;

	uint32_t red = (c >> 16) & 255;
	//std::cout << red << std::endl;

	try
	{
		std::unique_ptr<Engine::RenderingEngine> engine = std::make_unique<Engine::RenderingEngine>();

		engine->Run();
	}
	catch (const std::exception& e)
	{
		std::cerr << "ERROR::" << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}