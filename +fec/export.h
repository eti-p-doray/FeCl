
#ifndef MEX_EXPORT_H
#define MEX_EXPORT_H

#ifdef MEX_STATIC_DEFINE
#  define MEX_EXPORT
#  define MEX_NO_EXPORT
#else
#  ifndef MEX_EXPORT
#    ifdef LdpcCode_set_iterationCount_EXPORTS
        /* We are building this library */
#      define MEX_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define MEX_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef MEX_NO_EXPORT
#    define MEX_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef MEX_DEPRECATED
#  define MEX_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef MEX_DEPRECATED_EXPORT
#  define MEX_DEPRECATED_EXPORT MEX_EXPORT MEX_DEPRECATED
#endif

#ifndef MEX_DEPRECATED_NO_EXPORT
#  define MEX_DEPRECATED_NO_EXPORT MEX_NO_EXPORT MEX_DEPRECATED
#endif

#define DEFINE_NO_DEPRECATED 0
#if DEFINE_NO_DEPRECATED
# define MEX_NO_DEPRECATED
#endif

#endif
