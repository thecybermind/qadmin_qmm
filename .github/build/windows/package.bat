mkdir package
pushd package
del /q *
rem copy ..\README.md .\
rem copy ..\LICENSE .\

for %%x in (
        Q3A
        RTCWMP
		RTCWSP
		WET
		JAMP
		JK2MP
		SOF2MP
		STVOYHM
		STEF2
		MOHAA
		MOHBT
		MOHSH
		QUAKE2
       ) do (
         copy ..\bin\Release-%%x\x86\advchat_qmm_%%x.dll .\
         copy ..\bin\Release-%%x\x64\advchat_qmm_x86_64_%%x.dll .\         
       )
copy ..\bin\Release-Q2R\x64\advchat_qmm_x86_64_Q2R.dll .\         
popd
