for %%f in (*.frag, *.vert) do (
	%VK_SDK_PATH%\Bin\glslangValidator.exe -V -H "%%~nxf" -o "%%~nxf.spv"
)