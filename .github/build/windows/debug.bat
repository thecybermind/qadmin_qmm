for %%x in (
        Q3A
        RTCWMP
		RTCWSP
		WET
		JAMP
		JASP
		JK2MP
		JK2SP
		SOF2MP
		STVOYHM
		STVOYSP
		STEF2
		MOHAA
		MOHBT
		MOHSH
		QUAKE2
       ) do (
         msbuild .\msvc\qadmin_qmm.vcxproj /p:Configuration=Debug-%%x /p:Platform=x86
         msbuild .\msvc\qadmin_qmm.vcxproj /p:Configuration=Debug-%%x /p:Platform=x64
       )

msbuild .\msvc\qadmin_qmm.vcxproj /p:Configuration=Debug-Q2R /p:Platform=x64
