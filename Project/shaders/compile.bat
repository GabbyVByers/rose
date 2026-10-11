
::
::             _
::             \`*-.
::              )  _`-.
::             .  : `. .     *paws at u cutely :3*
::             : _   '  \
::             ; *` _.   `*-._
::             `-.-'          `-.
::    *meow*     ;       `       `.
::               :.       .        \
::               . \  .   :   .-'   .
::               '  `+.;  ;  '      :
::               :  '  |    ;       ;-.
::               ; '   : :`-:     _.`* ;
::            .*' /  .*' ; .*`- +'  `*'
::            `*-*   `*-*  `*-*'
::

@echo on
glslc "source/sprite.vert" -o "sprite_vertex.spv"
glslc "source/sprite.frag" -o "sprite_fragment.spv"
glslc "source/mesh.vert" -o "mesh_vertex.spv"
glslc "source/mesh.frag" -o "mesh_fragment.spv"

@echo off
pause

