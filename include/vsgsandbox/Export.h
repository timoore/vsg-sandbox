#ifndef VSGSANDBOX_EXPORT_H
#define VSGSANDBOX_EXPORT_H 1

#if (defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGWIN32__))
#if defined(vsgsandbox_EXPORTS)
#define VSGSANDBOX_DECLSPEC __declspec(dllexport)
#elif defined(VSGSANDBOX_SHARED_LIBRARY)
#define VSGSANDBOX_DECLSPEC __declspec(dllimport)
#else
#define VSGSANDBOX_DECLSPEC
#endif
#else
#define VSGSANDBOX_DECLSPEC
#endif

#endif

