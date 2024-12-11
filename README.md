# QPP Console Lang Converter

The converter converts csv translation file to react-i18n json files.

The converter is developed with Qt6 in C++.

It is recommended to use MSYS2 to build.

## Deploy Qt6 DLLs

```
windeployqt6 --no-translations --no-system-d3d-compiler --release ./qpp-console-lang-converter.exe
```

## Find required DLLS

```
ldd ./qpp-console-lang-converter.exe
```

Copy the DLLs (in MSYS2 console)

```
dep=$(ldd ./qpp-console-lang-converter.exe | awk '{if (match($3,"/ucrt64")) {print $3}})
mkdir libs
cp -L -n $dep libs
```
