for %%f in (*.frag, *.vert) do (
	%VK_SDK_PATH%\Bin\glslangValidator.exe -V "%%~nxf" -o "%%~nxf.spv"
)