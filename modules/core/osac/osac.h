#ifndef OSAC_H
#define OSAC_H

#if defined(DEBUG)
#define OSAC_INFO(...) printf(__VA_ARGV__)
#define OSAC_ERR(...)  printf(__VA_ARGV__)
#else
#define OSAC_INFO(...)
#define OSAC_ERR(...)
#endif
#endif // OSAC_H