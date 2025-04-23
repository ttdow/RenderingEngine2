#include "pch.h"

#define VMA_IMPLEMENTATION

//#define _WIN32_WINNT 0x0A00
//#define ASIO_STANDALONE
//#include <asio.hpp>

#include "VulkanBackend.h"
	
namespace Engine
{
	void VulkanBackend::TrainDQN()
	{
		asio::io_context io;
		asio::ip::tcp::socket socket(io);

		// Create a new socket connection endpoint and connect to it.
		asio::ip::tcp::endpoint endpoint(asio::ip::make_address("127.0.0.1"), 8801);
		socket.connect(endpoint);

		MSELoss mse;
		DQNAgent agent(16, 4, &mse);

		const int episodes = 2000;
		const int updateInterval = episodes / 10;
		const float gamma = 0.99f;
		const float lr = 0.001f;
		float epsilon = 1.0f;
		const float epsilonTarget = 0.02f;
		const float epsilonMin = 0.01f;
		const float k_value = -std::log((epsilonTarget - epsilonMin) / (1.0f - epsilonMin)) / episodes;
		const int updateSize = 3;
		const int targetUpdateFrequency = 100;
		const int batchSize = 32;

		const int32_t RESET_SIGNAL = htonl(-1);

		const int32_t TEST = htonl(1);

		std::vector<int> actionCounter = { 0, 0, 0, 0 };
		std::vector<int> actionTotaler = { 0, 0, 0, 0 };

		float averageReward = 0.0f;

		size_t stepCounter = 0;

		try
		{
			for (int ep = 1; ep < episodes; ep++)
			{
				// Receive initial state data from Python server.
				std::vector<char> buffer(updateSize * sizeof(float));
				asio::read(socket, asio::buffer(buffer));

				// Convert the data to a flloat[].
				std::vector<float> update(updateSize);
				std::memcpy(update.data(), buffer.data(), updateSize * sizeof(float));

				// Unpack state data.
				std::vector<float> state(16, 0.0f);
				state[update[0]] = 1.0f;

				// Track reward over episode.
				float totalReward = 0.0f;

				while (true)
				{
					stepCounter++;

					// Use the DQN agent to select an action.
					int action = agent.Act(state, epsilon);

					// Track action selection metrics.
					actionCounter[action]++;
					actionTotaler[action]++;

					// Send action to Python server.
					int32_t netAction = htonl(action);
					asio::write(socket, asio::buffer(&netAction, sizeof(netAction)));

					// Receive next state data from Python server.
					asio::read(socket, asio::buffer(buffer));
					std::memcpy(update.data(), buffer.data(), updateSize * sizeof(float));

					// Unpack update data.
					std::vector<float> nextState(16, 0.0f);
					nextState[update[0]] = 1.0f;

					float reward = update[1];
					bool done = (update[2] > 0.5f) ? true : false;

					// Add transition to replay memory.
					agent.Step(state, action, reward, nextState, done);

					// Train the DQN agent.
					//agent.Train(state, action, reward, nextState, done, gamma, lr);

					// Batch training.
					agent.TrainBatch(gamma, lr, batchSize);

					// Update target network periodically.
					if (stepCounter % targetUpdateFrequency == 0)
					{
						agent.UpdateTargetNetwork();
					}

					// Update state for next step.
					state = nextState;
					totalReward += reward;

					// Move to next episode if the environment was terminated or truncated.
					if (done)
					{
						break;
					}
				}

				averageReward += totalReward;

				// Update epsilon for explore/exploit.
				epsilon = epsilonMin + (1.0f - epsilonMin) * std::exp(-k_value * ep);

				// Print episode metrics.
				if (ep % updateInterval == 0)
				{
					std::cout << "Epsiode " << ep << ", Average reward: " << (averageReward / updateInterval) << ", Epsilon: " << epsilon << '\n';
					//std::cout << "Action 0: " << actionCounter[0] << ", Action 1: " << actionCounter[1] << ", Action 2: " << actionCounter[2] << ", Action 3: " << actionCounter[3] << '\n';

					averageReward = 0.0f;
					actionCounter = { 0, 0, 0, 0 };
				}
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << '\n';
		}

		//std::cout << "Action 0: " << actionTotaler[0] << ", Action 1: " << actionTotaler[1] << ", Action 2: " << actionTotaler[2] << ", Action 3: " << actionTotaler[3] << '\n';

		// Close the connection and client socket.
		socket.shutdown(asio::ip::tcp::socket::shutdown_both);
		socket.close();
		std::cout << "Connection closed.\n";
	}

	void VulkanBackend::AccumulateWaveToHeightField(const double x, const double waveLength, const double maxHeight, std::array<double, kBufferSize>& heightField)
	{
		const double quarterWaveLength = 0.25 * waveLength;
		const int start = static_cast<int>((x - quarterWaveLength) * kBufferSize);
		const int end = static_cast<int>((x + quarterWaveLength) * kBufferSize);

		for (int i = start; i < end; i++)
		{
			int iNew = i;
			if (i < 0)
			{
				iNew = -i - 1;
			}
			else if (i >= static_cast<int>(kBufferSize))
			{
				iNew = 2 * kBufferSize - i - 1;
			}

			double distance = std::fabs((i + 0.5) / kBufferSize - x);
			double height = maxHeight * 0.5 * (std::cos(std::min(distance * PI / quarterWaveLength, PI)) + 1.0);

			heightField[iNew] += height;
		}
	}

	void VulkanBackend::UpdateWave(const double timeInterval, double& x, double& speed)
	{
		x += (timeInterval * speed);

		// Boundary reflection.
		if (x > 1.0)
		{
			speed *= -1.0;
			x = 1.0 + (timeInterval * speed);
		}
		else if (x < 0.0)
		{
			speed *= -1.0;
			x = timeInterval * speed;
		}
	}

	void VulkanBackend::DrawFluid(const std::array<double, kBufferSize>& heightField) const
	{
		std::string buffer(kBufferSize, ' ');

		// Convert height field to grayscale.
		for (size_t i = 0; i < kBufferSize; i++)
		{
			double height = heightField[i];
			size_t tableIndex = std::min(static_cast<size_t>(std::floor(kGrayScaleTableSize * height)), kGrayScaleTableSize - 1);
			buffer[i] = kGrayScaleTable[tableIndex];
		}

		// Draw new buffer.
		std::cout << buffer.c_str() << std::endl;
	}

	void VulkanBackend::Fluid()
	{
		const double waveLengthX = 0.8;
		const double waveLengthY = 1.2;

		const double maxHeightX = 0.5;
		const double maxHeightY = 0.4;

		double x = 0.0;
		double y = 1.0;
		double speedX = 1.0;
		double speedY = -0.5;

		const int fps = 100;
		const double timeInterval = 1.0 / fps;

		std::array<double, kBufferSize> heightField;

		for (int i = 0; i < 1000; i++)
		{
			// March through time.
			UpdateWave(timeInterval, x, speedX);
			UpdateWave(timeInterval, y, speedY);
		
			// Clear height field.
			for (double& height : heightField)
			{
				height = 0.0;
			}

			// Accumulate waves for each center point.
			AccumulateWaveToHeightField(x, waveLengthX, maxHeightX, heightField);
			AccumulateWaveToHeightField(y, waveLengthY, maxHeightY, heightField);

			// Draw height field.
			DrawFluid(heightField);

			// Wait.
			std::this_thread::sleep_for(std::chrono::milliseconds(1000 / fps));
		}

		return;
	}

	VulkanBackend::VulkanBackend(HINSTANCE hInstance, HWND hwnd)
	{
		/*
		RecurrentNeuralNetwork rnn(3, 4, 2); // 3 inputs, 4 hidden, 2 outputs

		std::vector<std::vector<float>> inputSeq = 
		{ 
			{ 1.0f, 0.0f, -1.0f }, 
			{ 0.5f, 0.2f, -0.3f },
			{ -0.2f, 0.1f, 0.0f }
		};

		std::vector<std::vector<float>> targetSeq =
		{
			{ 0.0f, 1.0f },
			{ 0.5f, 0.5f },
			{ 1.0f, 0.0f }
		};

		for (int epoch = 0; epoch < 10; epoch++)
		{
			rnn.ResetState();
			
			std::cout << "Epoch " << epoch << ":\n";
			
			for (const auto& x : inputSeq)
			{
				std::vector<float> y = rnn.Step(x);
				RecurrentNeuralNetwork::PrintVec(y);
				std::cout << '\n';
			}

			rnn.Backward(targetSeq, 0.1f);
		}
		*/

		/*
		using namespace std::chrono;

		NeuralNetwork nn({ 3, 16, 16, 1 });

		// 3-bit Parity dataset.
		std::vector<std::vector<float>> inputs = 
		{ 
			{ 0.0f, 0.0f, 0.0f }, 
			{ 1.0f, 0.0f, 0.0f }, 
			{ 0.0f, 1.0f, 0.0f }, 
			{ 0.0f, 0.0f, 1.0f }, 
			{ 1.0f, 1.0f, 0.0f },
			{ 1.0f, 0.0f, 1.0f },
			{ 0.0f, 1.0f, 1.0f },
			{ 1.0f, 1.0f, 1.0f }
		};

		std::vector<std::vector<float>> targets = 
		{ 
			{ 0.0f },
			{ 1.0f },
			{ 1.0f },
			{ 1.0f },
			{ 0.0f },
			{ 0.0f },
			{ 0.0f },
			{ 1.0f }
		};

		auto start1 = high_resolution_clock::now();

		const int epochs = 10000;
		const int interval = epochs / 10;
		for (int epoch = 0; epoch < epochs; epoch++)
		{
			for (int i = 0; i < inputs.size(); i++)
			{
				nn.Train(inputs[i], targets[i], 0.05f);
			}

			if (epoch % interval == 0)
			{
				float loss = 0;
				for (int i = 0; i < inputs.size(); i++)
				{
					auto pred = nn.Predict(inputs[i]);
					float err = targets[i][0] - pred[0];
					loss += err * err;
				}

				std::cout << "Epoch " << epoch << " Loss: " << loss << '\n';
			}
		}

		auto end1 = high_resolution_clock::now();
		auto duration1 = duration_cast<microseconds>(end1 - start1);

		std::cout << "Training time: " << duration1.count() / 1000.0 << " ms.\n";

		auto start2 = high_resolution_clock::now();
		 
		for (int i = 0; i < inputs.size(); i++)
		{
			auto output = nn.Predict(inputs[i]);

			std::cout << "Neural network prediction: " << output[0] << '\n';
		}

		auto end2 = high_resolution_clock::now();
		auto duration2 = duration_cast<microseconds>(end2 - start2);

		std::cout << "Predictions time: " << duration2.count() / 1000.0 << " ms.\n";

		/*
		// XOR problem dataset.
		std::vector<TrainingExample> dataset =
		{
			{ { 0.0f, 0.0f }, 0.0f },
			{ { 0.0f, 1.0f }, 1.0f },
			{ { 1.0f, 0.0f }, 1.0f },
			{ { 1.0f, 1.0f }, 0.0f }
		};

		std::vector<std::vector<float>> inputs = { {0, 0}, {0, 1}, {1, 0}, {1, 1} };
		std::vector<float> targets = { 0, 1, 1, 0 };

		const int n = 1;
		const int epochs = 1000000; // One million epochs.
		const int updatePeriod = epochs / 10;
		std::vector<float> results(4);

		std::random_device rd;
		std::mt19937 g(rd());

		auto start1 = high_resolution_clock::now();

		for (int j = 0; j < n; j++)
		{
			SimpleNeuralNetwork nn;

			for (int epoch = 0; epoch < epochs; epoch++)
			{
				std::shuffle(dataset.begin(), dataset.end(), g);

				for (int i = 0; i < 4; i++)
				{
					nn.Train(inputs[i], targets[i], 0.1f);
				}

				if (epoch % updatePeriod == 0)
				{
					float loss = 0;
					for (int i = 0; i < inputs.size(); i++)
					{
						float pred = nn.Predict(inputs[i]);
						float err = targets[i] - pred;
						loss += err * err;
					}

					std::cout << "Epoch " << epoch << " Loss: " << loss << '\n';
				}
			}

			auto end1 = high_resolution_clock::now();
			auto duration1 = duration_cast<microseconds>(end1 - start1);

			std::cout << "Neural network train time: " << (duration1.count() / 1000.0) << " ms.\n";

			auto start2 = high_resolution_clock::now();

			// Predict.
			for (int i = 0; i < 4; i++)
			{
				float pred = nn.Predict(inputs[i]);

				results[i] += pred;
			}

			auto end2 = high_resolution_clock::now();
			auto duration2 = duration_cast<nanoseconds>(end2 - start2);

			std::cout << "Neural network predict time: " << (duration2.count() / 1000.0) << " us.\n";
		}
		
		for (int i = 0; i < 4; i++)
		{
			results[i] /= static_cast<double>(n);

			std::cout << inputs[i][0] << " XOR " << inputs[i][1] << " = " << results[i] << '\n';
		}
		*/

		/*
		const size_t reps = 10000000000;

		Vector2 v(1.0, 2.0);
		Vector2 u(4.0, 3.0);

		// This is being optimized to SIMD by the compiler.
		auto start1 = high_resolution_clock::now();
		Vector2 sum;
		for (size_t i = 0; i < reps; i++)
		{
			Vector2 w = v + u;

			sum = sum + w;
		}
		auto end1 = high_resolution_clock::now();
		auto duration1 = duration_cast<microseconds>(end1 - start1);

		std::cout << "Vector2 test: " << sum << " (Time: " << duration1.count() / 1000.0 << " ms)" << std::endl;

		Vector2_SIMD x(1.0, 2.0);
		Vector2_SIMD y(4.0, 3.0);

		auto start2 = high_resolution_clock::now();
		Vector2_SIMD sum_SIMD;
		for (size_t i = 0; i < reps; i++)
		{
			Vector2_SIMD w = x + y;

			sum_SIMD = sum_SIMD + w;
		}
		auto end2 = high_resolution_clock::now();
		auto duration2 = duration_cast<microseconds>(end2 - start2);

		std::cout << "Vector2_SIMD test: " << sum_SIMD << " (Time: " << duration2.count() / 1000.0 << " ms)" << std::endl;
		*/

		//Fluid();

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pNext = nullptr;
		appInfo.pApplicationName = "RenderingEngine";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_4;

		// Get list of all available instance extensions.
		uint32_t availableExtensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions.data());

		// Print list of available instance extensions.
		std::cout << "AVAILABLE INSTANCE EXTENSIONS: \n";
		for (const auto& extension : availableExtensions)
		{
			std::cout << " " << extension.extensionName << '\n';
		}

		std::vector<const char*> requiredExtensions;
		requiredExtensions.push_back("VK_KHR_surface");
		requiredExtensions.push_back("VK_KHR_win32_surface");
		requiredExtensions.push_back("VK_KHR_get_physical_device_properties2");
		requiredExtensions.push_back("VK_KHR_get_surface_capabilities2");

		// Verify all required instance extensions are available.
		for (const char* required : requiredExtensions)
		{
			bool extensionFound = false;

			// Compare the required extension to all available extensions.
			for (const VkExtensionProperties& available : availableExtensions)
			{
				if (strcmp(required, available.extensionName) == 0)
				{
					// We found a match, break out of the inner loop.
					extensionFound = true;
					break;
				}
			}

			// Ensure a match was found for the current required extension.
			if (!extensionFound)
			{
				throw std::runtime_error("All required Vulkan extensions are not available!");
			}
		}

		// Create the instance.
		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = requiredExtensions.size();
		createInfo.ppEnabledExtensionNames = requiredExtensions.data();
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Vulkan instance!");
		}

		// Create surface.
		VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
		surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceInfo.pNext = nullptr;
		surfaceInfo.hinstance = hInstance;
		surfaceInfo.hwnd = hwnd;

		if (vkCreateWin32SurfaceKHR(instance, &surfaceInfo, nullptr, &surface) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Win32 surface!");
		}

		// Count available physical devices.
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		// If no physical device exists, throw an error.
		if (deviceCount == 0)
		{
			throw std::runtime_error("Failed to find any GPUs with Vulkan support!");
		}

		// Get a list of the available physical devices.
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		// Iterate through physical devices and rank each one.
		std::multimap<int, VkPhysicalDevice> candidates;
		for (const VkPhysicalDevice& device : devices)
		{
			int score = RateDeviceSuitability(device);
			candidates.insert(std::make_pair(score, device));
		}

		// Ensure the best candidate is suitable.
		if (candidates.rbegin()->first > 0)
		{
			physicalDevice = candidates.rbegin()->second;
		}
		else
		{
			throw std::runtime_error("Failed to find a suitable GPU!");
		}

		// Save the details of the selected physical device.
		queueFamilyIndices = FindQueueFamilies(physicalDevice);
		swapchainSupportDetails = QuerySwapchainSupport(physicalDevice);
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
		vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);

		// Setup ray tracing properties struct for pNext chain.
		physicalDeviceRTProperties = {};
		physicalDeviceRTProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
		physicalDeviceRTProperties.pNext = nullptr;

		// Query ray tracing properties of the physical device.
		physicalDeviceProperties2 = {};
		physicalDeviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		physicalDeviceProperties2.pNext = &physicalDeviceRTProperties;
		physicalDeviceProperties2.properties = physicalDeviceProperties;
		vkGetPhysicalDeviceProperties2(physicalDevice, &physicalDeviceProperties2);

		// Print the name of the selected device.
		std::cout << "Selected GPU: " << physicalDeviceProperties.deviceName << std::endl;

		// Get the selected physical device's available extensions.
		uint32_t deviceExtensionCount = 0;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtensionCount, nullptr);
		std::vector<VkExtensionProperties> availableDeviceExtensions(deviceExtensionCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtensionCount, availableDeviceExtensions.data());

		// Print the selected physical device's available extensions.
		std::cout << "AVAILABLE DEVICE EXTENSIONS: " << std::endl;
		for (const VkExtensionProperties& extension : availableDeviceExtensions)
		{
			std::cout << "  " << extension.extensionName << std::endl;
		}

		// List the required extensions.
		std::vector<const char*> deviceRequiredExtensions =
		{
			"VK_KHR_swapchain",
			"VK_KHR_maintenance4",
			"VK_KHR_dynamic_rendering",
			"VK_EXT_descriptor_indexing",
			"VK_KHR_buffer_device_address",
			"VK_KHR_synchronization2"
		};

		// Ensure the selected device supports the required extensions.
		for (const char* extension : deviceRequiredExtensions)
		{
			bool extensionFound = false;

			for (const VkExtensionProperties& available : availableDeviceExtensions)
			{
				if (strcmp(extension, available.extensionName) == 0)
				{
					extensionFound = true;
					break;
				}
			}

			if (!extensionFound)
			{
				throw std::runtime_error("All required physical device extensions are not available!");
			}
		}

		// List the hardware-accelerated ray tracing extensions.
		const std::vector<const char*> raytracingExtensionList =
		{
			"VK_KHR_ray_tracing_pipeline",
			"VK_KHR_acceleration_structure",
			"VK_KHR_maintenance3",
			"VK_KHR_deferred_host_operations"
		};

		// Check if the selected physical device supports hardware-accelerated ray tracing.
		hardwareRaytracingSupported = false;
		for (const char* extension : raytracingExtensionList)
		{
			bool extensionFound = false;

			for (const VkExtensionProperties& available : availableDeviceExtensions)
			{
				if (strcmp(extension, available.extensionName) == 0)
				{
					extensionFound = true;
					break;
				}
			}

			// If any one extension is not present, flag hardware accelerated ray tracing as unsupported.
			if (!extensionFound)
			{
				hardwareRaytracingSupported = false;
				break;
			}
		}

		// Enable ray tracing features if hardware ray tracing is supported.
		VkPhysicalDeviceAccelerationStructureFeaturesKHR asFeatures = {};
		VkPhysicalDeviceRayTracingPipelineFeaturesKHR rtFeatures = {};
		if (hardwareRaytracingSupported)
		{
			// TODO.
		}
		else
		{
			std::cout << "Hardware does not support RTX. Skipping RTX features.\n";
		}

		// Queue families.
		float queuePriority = 1.0f;
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.pNext = nullptr;
		queueCreateInfo.flags = 0;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		queueCreateInfo.pQueuePriorities = &queuePriority;
		
		// Vulkan 1.3 features.
		VkPhysicalDeviceVulkan13Features features13 = {};
		features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
		features13.pNext = (hardwareRaytracingSupported) ? &rtFeatures : nullptr;
		features13.synchronization2 = VK_TRUE;
		features13.dynamicRendering = VK_TRUE;
		features13.maintenance4 = VK_TRUE;

		// Vulkan 1.2 features.
		VkPhysicalDeviceVulkan12Features features12 = {};
		features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		features12.pNext = &features13;
		features12.bufferDeviceAddress = VK_TRUE;
		features12.descriptorIndexing = VK_TRUE;

		// Extends features for Vulkan 1.2 and 1.3.
		VkPhysicalDeviceFeatures2 features2 = {};
		features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		features2.pNext = &features12;

		VkDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pNext = &features2;
		deviceCreateInfo.queueCreateInfoCount = 1;
		deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
		deviceCreateInfo.pEnabledFeatures = nullptr;
		deviceCreateInfo.enabledExtensionCount = deviceRequiredExtensions.size();
		deviceCreateInfo.ppEnabledExtensionNames = deviceRequiredExtensions.data();
		deviceCreateInfo.enabledLayerCount = 0;
		deviceCreateInfo.ppEnabledLayerNames = nullptr;

		if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Vulkan logical device!");
		}

		std::cout << "Logical device created successfully.\n";

		// Query device queues.
		vkGetDeviceQueue(device, queueFamilyIndices.graphicsFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(device, queueFamilyIndices.computeFamily.value(), 0, &computeQueue);
		vkGetDeviceQueue(device, queueFamilyIndices.transferFamily.value(), 0, &transferQueue);
		vkGetDeviceQueue(device, queueFamilyIndices.presentFamily.value(), 0, &presentQueue);

		VkSurfaceFormatKHR surfaceFormat = {};
		for (const VkSurfaceFormatKHR& availableFormat : swapchainSupportDetails.surfaceFormats)
		{
			std::cout << "  (" << string_VkFormat(availableFormat.format) << ", ";
			std::cout << string_VkColorSpaceKHR(availableFormat.colorSpace) << ")\n";
		
			// UNORM will output linear color values, SRGB will output gamma corrected values automatically.
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				surfaceFormat = availableFormat;
			}
		}

		std::cout << "Surface format selected: (" << string_VkFormat(surfaceFormat.format) << ", ";
		std::cout << string_VkColorSpaceKHR(surfaceFormat.colorSpace) << ")\n";

		// Save the surface image format for later.
		imageFormat = surfaceFormat.format;

		// Choose the present mode.
		std::cout << "Available present modes: \n";
		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
		for (const VkPresentModeKHR& availablePresentMode : swapchainSupportDetails.presentModes)
		{
			std::cout << "  " << string_VkPresentModeKHR(availablePresentMode) << '\n';
			
			if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
			{
				presentMode = availablePresentMode;
			}
		}

		std::cout << "Present mode selected: " << string_VkPresentModeKHR(presentMode) << '\n';

		const VkSurfaceCapabilitiesKHR& surfaceCapabilities = swapchainSupportDetails.surfaceCapabilities;

		// Get the current surface extent.
		imageExtent = surfaceCapabilities.currentExtent;

		// Get the swapchain image count supported.
		const uint32_t minImageCount = surfaceCapabilities.minImageCount;
		const uint32_t maxImageCount = surfaceCapabilities.maxImageCount;

		std::cout << "Swapchain image counts supported: [" << minImageCount << ", " << maxImageCount << "]\n";

		// Select the number of swapchain framebuffers to use.
		framebufferCount = minImageCount + 1;
		if (maxImageCount > 0 && maxImageCount < minImageCount + 1)
		{
			framebufferCount = maxImageCount;
		}

		std::cout << "Swapchain images used: " << framebufferCount << '\n';

		// Create the swapchain.
		VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
		swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCreateInfo.pNext = nullptr;
		swapchainCreateInfo.surface = surface;
		swapchainCreateInfo.minImageCount = framebufferCount;
		swapchainCreateInfo.imageFormat = imageFormat;
		swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
		swapchainCreateInfo.imageExtent = imageExtent;
		swapchainCreateInfo.imageArrayLayers = 1;
		swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainCreateInfo.queueFamilyIndexCount = 0;
		swapchainCreateInfo.pQueueFamilyIndices = nullptr;
		swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
		swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchainCreateInfo.presentMode = presentMode;
		swapchainCreateInfo.clipped = VK_TRUE;
		swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Vulkan swapchain!");
		}

		// Get swapchain images.
		vkGetSwapchainImagesKHR(device, swapchain, &framebufferCount, nullptr);
		swapchainImages.resize(framebufferCount);
		vkGetSwapchainImagesKHR(device, swapchain, &framebufferCount, swapchainImages.data());

		// Create swapchain image views.
		swapchainImageViews.resize(framebufferCount);
		for (int i = 0; i < framebufferCount; i++)
		{
			VkImageViewCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.image = swapchainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = imageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(device, &createInfo, nullptr, &swapchainImageViews[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create swapchain image view!");
			}
		}

		VmaAllocatorCreateInfo vmaInfo = {};
		vmaInfo.physicalDevice = physicalDevice;
		vmaInfo.device = device;
		vmaInfo.instance = instance;
		vmaInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

		vmaCreateAllocator(&vmaInfo, &allocator);

		VkCommandPoolCreateInfo cmdPoolInfo = {};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.pNext = nullptr;
		cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		cmdPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		VkResult result = vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &immCmdPool);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create immediate mode command pool!");
		}

		VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
		cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufferAllocInfo.pNext = nullptr;
		cmdBufferAllocInfo.commandPool = immCmdPool;
		cmdBufferAllocInfo.commandBufferCount = 1;
		cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		result = vkAllocateCommandBuffers(device, &cmdBufferAllocInfo, &immCmdBuffer);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create immediate mode command buffer!");
		}

		swapchainCmdPools.resize(framebufferCount);
		swapchainCmdBuffers.resize(framebufferCount);
		for (uint32_t i = 0; i < framebufferCount; i++)
		{
			result = vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &swapchainCmdPools[i]);
			if (result != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create swapchain frame buffer command pool!");
			}

			VkCommandBufferAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.pNext = nullptr;
			allocInfo.commandPool = swapchainCmdPools[i];
			allocInfo.commandBufferCount = 1;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

			result = vkAllocateCommandBuffers(device, &allocInfo, &swapchainCmdBuffers[i]);
			if (result != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create swapchain frame buffer command buffer!");
			}
		}

		VkFenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.pNext = nullptr;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		result = vkCreateFence(device, &fenceCreateInfo, nullptr, &immFence);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Faile to create immediate mode fence!");
		}

		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreInfo.pNext = nullptr;
		semaphoreInfo.flags = 0;

		swapchainFences.resize(framebufferCount);
		swapchainSemaphores.resize(framebufferCount);
		renderSemaphores.resize(framebufferCount);
		for (uint32_t i = 0; i < framebufferCount; i++)
		{
			vkCreateFence(device, &fenceCreateInfo, nullptr, &swapchainFences[i]);
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &swapchainSemaphores[i]);
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderSemaphores[i]);
		}

		VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT;
		VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
			VK_IMAGE_USAGE_TRANSFER_DST_BIT |
			VK_IMAGE_USAGE_STORAGE_BIT |
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;

		offscreenFramebuffer = std::make_unique<VulkanImage>(
			device,
			allocator,
			format,
			imageExtent,
			usageFlags,
			aspectFlags
		);

		computeDescriptor = std::make_unique<VulkanDescriptor>(device);
		computeDescriptor->AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		computeDescriptor->Build(VK_SHADER_STAGE_COMPUTE_BIT);
		computeDescriptor->InitializePool(1);
		computeDescriptor->Allocate();

		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		imageInfo.imageView = offscreenFramebuffer->GetView();

		std::vector<VkWriteDescriptorSet> writes;

		VkWriteDescriptorSet imageWrite = {};
		imageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		imageWrite.pNext = nullptr;
		imageWrite.dstBinding = 0;
		imageWrite.dstSet = computeDescriptor->GetSet();
		imageWrite.descriptorCount = 1;
		imageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		imageWrite.pImageInfo = &imageInfo;
		writes.push_back(imageWrite);

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

		// COMPUTE PIPELINE

		computePipeline = std::make_unique<VulkanPipeline>(device);
			
		computePipeline->CreateLayout(computeDescriptor->GetLayout());

		VkShaderModule computeShader;
		if (!VulkanPipeline::LoadShaderModule(device, "./src/shader/shader.comp.spv", &computeShader))
		{
			throw std::runtime_error("Failed to build compute shader!");
		}

		computePipeline->LoadShader(VK_SHADER_STAGE_COMPUTE_BIT, computeShader);

		computePipeline->CreatePipeline();

		vkDestroyShaderModule(device, computeShader, nullptr);
	}

	VulkanBackend::~VulkanBackend()
	{
		vkDeviceWaitIdle(device);

		offscreenFramebuffer.reset();
		computeDescriptor.reset();
		computePipeline.reset();

		for (uint32_t i = 0; i < framebufferCount; i++)
		{
			vkDestroyFence(device, swapchainFences[i], nullptr);
			vkDestroySemaphore(device, swapchainSemaphores[i], nullptr);
			vkDestroySemaphore(device, renderSemaphores[i], nullptr);
		}

		vkDestroyFence(device, immFence, nullptr);

		for (auto& cmdPool : swapchainCmdPools)
		{
			vkDestroyCommandPool(device, cmdPool, nullptr);
		}

		vkDestroyCommandPool(device, immCmdPool, nullptr);

		vmaDestroyAllocator(allocator);

		for (VkImageView imageView : swapchainImageViews)
		{
			vkDestroyImageView(device, imageView, nullptr);
		}

		vkDestroySwapchainKHR(device, swapchain, nullptr);
		vkDestroyDevice(device, nullptr);
		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyInstance(instance, nullptr);
	}

	void VulkanBackend::Draw()
	{
		currentFrame = currentFrame % framebufferCount;

		vkWaitForFences(device, 1, &swapchainFences[currentFrame], true, 1000000000);
		vkResetFences(device, 1, &swapchainFences[currentFrame]);

		uint32_t swapchainImageIndex;
		vkAcquireNextImageKHR(device, swapchain, 1000000000, swapchainSemaphores[currentFrame], nullptr, &swapchainImageIndex);

		VkCommandBuffer cmd = swapchainCmdBuffers[currentFrame];
		vkResetCommandBuffer(cmd, 0);

		VkCommandBufferBeginInfo begin = {};
		begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin.pNext = nullptr;
		begin.pInheritanceInfo = nullptr;
		begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(cmd, &begin);

		TransitionImage(cmd, offscreenFramebuffer->GetImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

		VkClearColorValue clearValue;
		float flash = std::abs(std::sin(frameNumber / 120.0f));
		clearValue = { { 0.0f, 0.0f, flash, 1.0f } };

		VkImageSubresourceRange clearRange = {};
		clearRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		clearRange.baseMipLevel = 0;
		clearRange.levelCount = VK_REMAINING_MIP_LEVELS;
		clearRange.baseArrayLayer = 0;
		clearRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

		vkCmdClearColorImage(cmd, offscreenFramebuffer->GetImage(), VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);

		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline->GetPipeline());
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline->GetLayout(), 0, 1, &computeDescriptor->GetSet(), 0, nullptr);
		vkCmdDispatch(cmd, std::ceil(imageExtent.width), std::ceil(imageExtent.height), 1);

		TransitionImage(cmd, offscreenFramebuffer->GetImage(), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		TransitionImage(cmd, swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VulkanImage::CopyImageToImage(cmd, offscreenFramebuffer->GetImage(), swapchainImages[swapchainImageIndex], imageExtent, imageExtent);

		TransitionImage(cmd, swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

		vkEndCommandBuffer(cmd);

		VkCommandBufferSubmitInfo cmdSubmit = {};
		cmdSubmit.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
		cmdSubmit.pNext = nullptr;
		cmdSubmit.commandBuffer = cmd;
		cmdSubmit.deviceMask = 0;

		VkSemaphoreSubmitInfo waitInfo = {};
		waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		waitInfo.pNext = nullptr;
		waitInfo.semaphore = swapchainSemaphores[currentFrame];
		waitInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR;
		waitInfo.deviceIndex = 0;
		waitInfo.value = 1;

		VkSemaphoreSubmitInfo signalInfo = {};
		signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		signalInfo.pNext = nullptr;
		signalInfo.semaphore = renderSemaphores[currentFrame];
		signalInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
		signalInfo.deviceIndex = 0;
		signalInfo.value = 1;

		VkSubmitInfo2 submit = {};
		submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		submit.pNext = nullptr;
		submit.waitSemaphoreInfoCount = 1;
		submit.pWaitSemaphoreInfos = &waitInfo;
		submit.signalSemaphoreInfoCount = 1;
		submit.pSignalSemaphoreInfos = &signalInfo;
		submit.commandBufferInfoCount = 1;
		submit.pCommandBufferInfos = &cmdSubmit;

		vkQueueSubmit2(graphicsQueue, 1, &submit, swapchainFences[currentFrame]);

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;
		presentInfo.pSwapchains = &swapchain;
		presentInfo.swapchainCount = 1;
		presentInfo.pWaitSemaphores = &renderSemaphores[currentFrame];
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pImageIndices = &swapchainImageIndex;

		vkQueuePresentKHR(graphicsQueue, &presentInfo);

		currentFrame++;
		frameNumber++;
	}

	int VulkanBackend::RateDeviceSuitability(VkPhysicalDevice device)
	{
		int score = 0;

		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
		
		std::cout << "Device found: " << deviceProperties.deviceName << std::endl;

		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			score += 1000;
		}

		score += deviceProperties.limits.maxImageDimension2D;

		if (!deviceFeatures.geometryShader)
		{
			score = 0;
		}

		if (!FindQueueFamilies(device).IsComplete())
		{
			score = 0;
		}

		return score;
	}

	QueueFamilyIndices VulkanBackend::FindQueueFamilies(VkPhysicalDevice device) const
	{
		QueueFamilyIndices indices;
		QueueFamilyIndices selectedIndices;

		// Get a list of the queue families supported by the physical device.
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		std::cout << "  Number of queue families: " << queueFamilyCount << std::endl;

		// Iterate through the available queue families.
		int i = 0;
		for (const VkQueueFamilyProperties& queueFamily : queueFamilies)
		{
			std::cout << "    Queue Family[" << i << "]: \n";

			// Check if this queue family has graphics support.
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
				std::cout << "      Supports graphics.\n";
			}

			if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				indices.computeFamily = i;
				std::cout << "      Supports compute.\n";
			}

			if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
			{
				indices.transferFamily = i;
				std::cout << "      Supports transfer.\n";
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (presentSupport)
			{
				indices.presentFamily = i;
				std::cout << "      Supports present.\n";
			}

			// Check if we have found support for all of graphics, compute, transfer, and present.
			if (indices.IsComplete())
			{
				selectedIndices = indices;
				break;
			}

			i++;
		}

		return selectedIndices;
	}

	SwapchainSupportDetails VulkanBackend::QuerySwapchainSupport(VkPhysicalDevice device) const
	{
		SwapchainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.surfaceCapabilities);

		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
		details.surfaceFormats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.surfaceFormats.data());

		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &formatCount, details.presentModes.data());

		return details;
	}

	void VulkanBackend::TransitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout)
	{
		VkImageMemoryBarrier2 barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		barrier.pNext = nullptr;
		barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
		barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		barrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
		barrier.oldLayout = currentLayout;
		barrier.newLayout = newLayout;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
		barrier.image = image;

		VkDependencyInfo dependency = {};
		dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependency.pNext = nullptr;
		dependency.imageMemoryBarrierCount = 1;
		dependency.pImageMemoryBarriers = &barrier;

		vkCmdPipelineBarrier2(cmd, &dependency);
	}
}