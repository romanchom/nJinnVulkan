for %%f in (*.obj) do (
    meshConverterApp.exe -i "%%~nxf" -o "%%~nf.sbm" -v -p float -t float -n float --tangent -r 16 -a -d -s
)