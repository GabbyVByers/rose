
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
glslc "source/point.vert" -o "point_vertex.spv"
glslc "source/point.frag" -o "point_fragment.spv"
glslc "source/sprite.vert" -o "sprite_vertex.spv"
glslc "source/sprite.frag" -o "sprite_fragment.spv"
glslc "source/triangle.vert" -o "triangle_vertex.spv"
glslc "source/triangle.frag" -o "triangle_fragment.spv"

@echo off
pause

