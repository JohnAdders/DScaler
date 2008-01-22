PROJECT (DScaler)

MACRO ( CreatePluginProject ProjectName )
 
   # get the file lists
   SET ( Dir ${CMAKE_HOME_DIRECTORY}/Plugins/${ProjectName} )
 
   # add your other extensions here (this could be probably done nicer)
   FILE ( GLOB_RECURSE DirSources ${Dir}/*.c ${Dir}/*.cpp ${Dir}/*.h ${Dir}/*.inl ${Dir}/*.rc ${Dir}/*.ico )

   add_library(${ProjectName} SHARED ${DirSources} )
 
ENDMACRO ( CreatePluginProject )


MACRO ( CreateExeProject ProjectName )
 
   # get the file lists
   SET ( Dir ${CMAKE_HOME_DIRECTORY}/${ProjectName} )
 
   # add your other extensions here (this could be probably done nicer)
   FILE ( GLOB_RECURSE DirSources ${Dir}/*.c ${Dir}/*.cpp ${Dir}/*.h ${Dir}/*.inl ${Dir}/*.rc ${Dir}/*.ico )

   add_executable(${ProjectName} WIN32 ${DirSources} )
 
ENDMACRO ( CreateExeProject )

CreatePluginProject( DI_Bob )
CreatePluginProject( DI_Weave )

CreateExeProject( DScaler )
CreateExeProject( Mapconv )
CreateExeProject( Disasm )
CreateExeProject( RegSpy )
