#ifndef OSAC_H
#define OSAC_H

#if defined(DEBUG)
#define OSAC_INFO printf
#define OSAC_ERR  printf
#else
#define OSAC_INFO
#define OSAC_ERR
#endif
#endif // OSAC_H