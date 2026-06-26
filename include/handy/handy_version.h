#ifndef HANDY_VERSION_H
#define HANDY_VERSION_H

#define HANDY_VERSION_MAJOR 0
#define HANDY_VERSION_MINOR 1
#define HANDY_VERSION_PATCH 0

#define HANDY_VERSION_STRING "0.1.0"

#define HANDY_VERSION_AT_LEAST(major, minor, patch) \
    ((HANDY_VERSION_MAJOR > (major)) || \
     (HANDY_VERSION_MAJOR == (major) && HANDY_VERSION_MINOR > (minor)) || \
     (HANDY_VERSION_MAJOR == (major) && HANDY_VERSION_MINOR == (minor) && HANDY_VERSION_PATCH >= (patch)))

#endif
