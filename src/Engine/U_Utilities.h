#ifndef UTILITIES_H_INCLUDED
#define UTILITIES_H_INCLUDED

#define FIX_ANGLES(a) (a > 2*M_PI) ? a -= 2*M_PI : (a < 0) ? a += 2*M_PI : a

#endif