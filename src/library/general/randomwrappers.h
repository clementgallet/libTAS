/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBTAS_RANDOM_H_INCL
#define LIBTAS_RANDOM_H_INCL

#include "hook.h"

#include <cstdint>
#include <cstddef>
#include <sys/types.h> // ssize_t

namespace libtas {

/* Write LENGTH bytes of randomness starting at BUFFER.  Return the
   number of bytes written, or -1 on error.  */
OVERRIDE ssize_t getrandom (void *buffer, size_t length, unsigned int flags);

/* These are the functions that actually do things.  The `random', `srandom',
   `initstate' and `setstate' functions are those from BSD Unices.
   The `rand' and `srand' functions are required by the ANSI standard.
   We provide both interfaces to the same random number generator.  */
/* Return a random long integer between 0 and RAND_MAX inclusive.  */
OVERRIDE long int random (void) __THROW;

/* Seed the random number generator with the given number.  */
OVERRIDE void srandom (unsigned int seed) __THROW;

/* Initialize the random number generator to use state buffer STATEBUF,
   of length STATELEN, and seed it with SEED.  Optimal lengths are 8, 16,
   32, 64, 128 and 256, the bigger the better; values less than 8 will
   cause an error and values greater than 256 will be rounded down.  */
OVERRIDE char *initstate (unsigned int seed, char *statebuf,
            size_t statelen) __THROW;

#ifdef __unix__
/* Switch the random number generator to state buffer STATEBUF,
 which should have been previously initialized by `initstate'.  */
OVERRIDE char *setstate (char *statebuf) __THROW;
#elif defined(__APPLE__) && defined(__MACH__)
OVERRIDE char *setstate (const char *statebuf);
#endif

// struct random_data
//   {
//     int32_t *fptr;      /* Front pointer.  */
//     int32_t *rptr;      /* Rear pointer.  */
//     int32_t *state;     /* Array of state values.  */
//     int rand_type;      /* Type of random number generator.  */
//     int rand_deg;       /* Degree of random number generator.  */
//     int rand_sep;       /* Distance between front and rear.  */
//     int32_t *end_ptr;       /* Pointer behind state table.  */
//   };

OVERRIDE int random_r (struct random_data *buf,
             int32_t *result) __THROW;

OVERRIDE int srandom_r (unsigned int seed, struct random_data *buf) __THROW;

OVERRIDE int initstate_r (unsigned int seed, char *statebuf, size_t statelen,
            struct random_data *buf) __THROW;

OVERRIDE int setstate_r (char *statebuf, struct random_data *buf) __THROW;

/* Return a random integer between 0 and RAND_MAX inclusive.  */
OVERRIDE int rand (void) __THROW;
/* Seed the random number generator with the given number.  */
OVERRIDE void srand (unsigned int seed) __THROW;

/* Reentrant interface according to POSIX.1.  */
OVERRIDE int rand_r (unsigned int *seed) __THROW;

/* System V style 48-bit random number generator functions.  */

/* Return non-negative, double-precision floating-point value in [0.0,1.0).  */
OVERRIDE double drand48 (void) __THROW;
OVERRIDE double erand48 (unsigned short int xsubi[3]) __THROW;

/* Return non-negative, long integer in [0,2^31).  */
OVERRIDE long int lrand48 (void) __THROW;
OVERRIDE long int nrand48 (unsigned short int xsubi[3]) __THROW;

/* Return signed, long integers in [-2^31,2^31).  */
OVERRIDE long int mrand48 (void) __THROW;
OVERRIDE long int jrand48 (unsigned short int xsubi[3]) __THROW;

/* Seed random number generator.  */
OVERRIDE void srand48 (long int seedval) __THROW;
OVERRIDE unsigned short int *seed48 (unsigned short int seed16v[3]) __THROW;
OVERRIDE void lcong48 (unsigned short int param[7]) __THROW;

/* Data structure for communication with thread safe versions.  This
   type is to be regarded as opaque.  It's only exported because users
   have to allocate objects of this type.  */
// struct drand48_data
//   {
//     unsigned short int __x[3];  /* Current state.  */
//     unsigned short int __old_x[3]; /* Old state.  */
//     unsigned short int __c; /* Additive const. in congruential formula.  */
//     unsigned short int __init;  /* Flag for initializing.  */
//     unsigned long long int __a;   /* Factor in congruential formula.  */
//   };

/* Return non-negative, double-precision floating-point value in [0.0,1.0).  */
OVERRIDE int drand48_r (struct drand48_data *buffer, double *result) __THROW;
OVERRIDE int erand48_r (unsigned short int xsubi[3],
              struct drand48_data *buffer, double *result) __THROW ;

/* Return non-negative, long integer in [0,2^31).  */
OVERRIDE int lrand48_r (struct drand48_data *buffer, long int *result) __THROW;
OVERRIDE int nrand48_r (unsigned short int xsubi[3],
              struct drand48_data *buffer, long int *result) __THROW;

/* Return signed, long integers in [-2^31,2^31).  */
OVERRIDE int mrand48_r (struct drand48_data *buffer, long int *result) __THROW;
OVERRIDE int jrand48_r (unsigned short int xsubi[3],
              struct drand48_data *buffer, long int *result) __THROW;

/* Seed random number generator.  */
OVERRIDE int srand48_r (long int seedval, struct drand48_data *buffer) __THROW;

OVERRIDE int seed48_r (unsigned short int seed16v[3],
             struct drand48_data *buffer) __THROW;

OVERRIDE int lcong48_r (unsigned short int param[7],
              struct drand48_data *buffer) __THROW;

}

#endif
