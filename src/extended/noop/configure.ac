AC_INIT(libledmgr_extended, 0.1)
AM_INIT_AUTOMAKE([foreign])
AC_PROG_CXX

# Checks for libraries.
AC_PROG_LIBTOOL

LEDMGR_EXTENDED_INCFLAGS="-I${PKG_CONFIG_SYSROOT_DIR}${includedir}/rdk/ds \
                       -I${PKG_CONFIG_SYSROOT_DIR}${includedir}/rdk/ds-hal \
                       -I${PKG_CONFIG_SYSROOT_DIR}${includedir}/glib-2.0 \
                       -I${PKG_CONFIG_SYSROOT_DIR}${includedir}/rdk/iarmmgrs-hal \
                       -I${PKG_CONFIG_SYSROOT_DIR}${includedir}/rdk/iarmbus \
                       -I${PKG_CONFIG_SYSROOT_DIR}${libdir}/glib-2.0/include \
                       -I${PKG_CONFIG_SYSROOT_DIR}${includedir}/ledmgr"
AC_SUBST(LEDMGR_EXTENDED_INCFLAGS)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
