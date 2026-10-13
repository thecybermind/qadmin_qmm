for %%G in (COD11MP CODMP CODUOMP JAMP JASP JK2MP JK2SP MOHAA MOHBT MOHSH Q2R Q3A QUAKE2 RTCWMP RTCWSP SIN SOF SOF2MP STEF2 STVOYHM STVOYSP WET) do (
    if [%%G] NEQ [Q2R] msbuild .\msvc\qadmin_qmm.vcxproj /p:Configuration=Debug-%%G /p:Platform=x86
    if errorlevel 1 exit /b errorlevel
    msbuild .\msvc\qadmin_qmm.vcxproj /p:Configuration=Debug-%%G /p:Platform=x64
    if errorlevel 1 exit /b errorlevel
)
