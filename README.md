# cppvk
vulkanとGLFW3とGLMの練習。

# LICENSE
Third party library.
- [GLM](https://github.com/g-truc/glm/blob/master/copying.txt)
- [Vulkan](https://github.com/SaschaWillems/Vulkan/blob/master/LICENSE.md)
- [GLFW3](https://www.glfw.org/license)

## VkInstance flow to create...
Process to create vkinstance.
```
auto instance = cppvk::InstanceBuilder::get()
			.enabledExtensionNames(extensions)
			.enabledLayerNames(validationLayers)
			.build();
```

## VkSurfaceKHR flow to create...
Processing to create vulkan surface in windows environment.
```
auto surface = cppvk::WinSurfaceBuilder::get(instance)
		    .hwnd(wPtr)
		    .build();
```


## 