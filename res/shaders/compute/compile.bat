@echo off

C:\VulkanSDK\1.2.182.0\Bin\glslc.exe generate_im.comp -o generate_im.spv
C:\VulkanSDK\1.2.182.0\Bin\glslc.exe generate_cubemap.comp -o generate_cubemap.spv
C:\VulkanSDK\1.2.182.0\Bin\glslc.exe generate_pm.comp -o generate_pm.spv

pause