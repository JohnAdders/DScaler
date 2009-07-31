
DSRendps.dll: dlldata.obj DSRend_p.obj DSRend_i.obj
	link /dll /out:DSRendps.dll /def:DSRendps.def /entry:DllMain dlldata.obj DSRend_p.obj DSRend_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del DSRendps.dll
	@del DSRendps.lib
	@del DSRendps.exp
	@del dlldata.obj
	@del DSRend_p.obj
	@del DSRend_i.obj
