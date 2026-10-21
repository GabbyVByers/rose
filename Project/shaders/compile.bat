
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
glslc "source/triangles.vert" -o "triangles_vertex.spv"
glslc "source/triangles.frag" -o "triangles_fragment.spv"
glslc "source/points.vert" -o "points_vertex.spv"
glslc "source/points.frag" -o "points_fragment.spv"

@echo off
pause

