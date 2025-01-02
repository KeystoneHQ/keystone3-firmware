#define PLUS9(x) (x + 9)

#define SOFTWARE_VERSION_MAJOR              PLUS9(1)
#define SOFTWARE_VERSION_MAJOR_OFFSET       9
#define SOFTWARE_VERSION_MINOR              0
#define SOFTWARE_VERSION_BUILD              0
#define SOFTWARE_VERSION_BETA               0
#define SOFTWARE_VERSION                    (SOFTWARE_VERSION_MAJOR * 10000 + SOFTWARE_VERSION_MINOR * 100 + SOFTWARE_VERSION_BUILD)
#define SOFTWARE_VERSION_SUFFIX             "-cypherpunk"

