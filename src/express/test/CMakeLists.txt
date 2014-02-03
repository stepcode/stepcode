include_directories(..)

sc_addexec( print_schemas "../fedex.c;print_schemas.c" "express;base" )
sc_addexec( print_attrs "../fedex.c;print_attrs.c" "express;base" )

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

