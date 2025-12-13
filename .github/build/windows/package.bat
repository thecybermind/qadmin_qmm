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
         copy ..\bin\Release-%%x\x86\qadmin_qmm_%%x.dll .\
         copy ..\bin\Release-%%x\x64\qadmin_qmm_x86_64_%%x.dll .\         
       )
copy ..\bin\Release-Q2R\x64\qadmin_qmm_x86_64_Q2R.dll .\         
popd
