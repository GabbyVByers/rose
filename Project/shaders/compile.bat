
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
glslc "source/shape.vert" -o "shape_vertex.spv"
glslc "source/shape.frag" -o "shape_fragment.spv"

@echo off
pause

