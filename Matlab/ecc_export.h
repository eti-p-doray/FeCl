
#ifndef ECC_EXPORT_H
#define ECC_EXPORT_H

#ifdef ECC_STATIC_DEFINE
#  define ECC_EXPORT
#  define ECC_NO_EXPORT
#else
#  ifndef ECC_EXPORT
#    ifdef LdpcCode_constructor_EXPORTS
        /* We are building this library */
#      define ECC_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define ECC_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef ECC_NO_EXPORT
#    define ECC_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef ECC_DEPRECATED
#  define ECC_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef ECC_DEPRECATED_EXPORT
#  define ECC_DEPRECATED_EXPORT ECC_EXPORT ECC_DEPRECATED
#endif

#ifndef ECC_DEPRECATED_NO_EXPORT
#  define ECC_DEPRECATED_NO_EXPORT ECC_NO_EXPORT ECC_DEPRECATED
#endif

#define DEFINE_NO_DEPRECATED 0
#if DEFINE_NO_DEPRECATED
# define ECC_NO_DEPRECATED
#endif

#endif
