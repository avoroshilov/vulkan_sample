# Vulkan base sample
This project uses shared [Core project](https://github.com/avoroshilov/core) (via git submodules) - in case of offline project assembly, please manually sync the Core repository and paste files into the `core` folder.

## Sample description
Basic illustration how to use Vulkan graphics API. Could be used as a simple visualizer for e.g. physics engine, or basic AI experiments.

What this sample implements:
* Z-PrePass
* Shadowmapping + Hardware PCF
* Static and Dynamic Meshes
* Forward Shading (Normalized Lambertian diffuse and Blinn-Phong specular)
* Immediate-mode-style Debug Rendering
* App screenshot capture

<img src="vkscreenshot.png" alt="Vulkan sample" />

## License
[Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License](https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode)
