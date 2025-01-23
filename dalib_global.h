#ifndef DALIB_GLOBAL_H
#define DALIB_GLOBAL_H


#ifdef DALIB
#define DA_EXPORT Q_DECL_EXPORT
#else
#define DA_EXPORT Q_DECL_IMPORT
#endif

#endif // DALIB_GLOBAL_H
