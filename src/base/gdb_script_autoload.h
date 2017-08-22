#ifndef GDB_SCRIPT_AUTOLOAD_H
#define GDB_SCRIPT_AUTOLOAD_H


// https://sourceware.org/gdb/onlinedocs/gdb/dotdebug_005fgdb_005fscripts-section.html, section 23.4.2.1 Script File Entries

/* Embed a section that tells gdb to auto-load a python script.
 * In spite of sourceware documentation to the contrary, it appears that gdb
 * only tries the file name as absolute path and as something relative to the
 * CWD - thus, script_name is practically useless if it isn't an absolute path.
 *
 * Note: The "MS" section flags are to remove duplicates.  */
#define DEFINE_GDB_PY_SCRIPT(script_name) \
  asm("\
.pushsection \".debug_gdb_scripts\", \"MS\",@progbits,1\n\
.byte 1 /* Python */\n\
.asciz \"" script_name "\"\n\
.popsection \n\
");

#endif //GDB_SCRIPT_AUTOLOAD_H
