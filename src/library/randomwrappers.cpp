/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "randomwrappers.h"
#include "logging.h"
#include "hook.h"

namespace libtas {

DEFINE_ORIG_POINTER(random)
DEFINE_ORIG_POINTER(srandom)
DEFINE_ORIG_POINTER(initstate)
DEFINE_ORIG_POINTER(setstate)
DEFINE_ORIG_POINTER(random_r)
DEFINE_ORIG_POINTER(srandom_r)
DEFINE_ORIG_POINTER(initstate_r)
DEFINE_ORIG_POINTER(setstate_r)
DEFINE_ORIG_POINTER(rand)
DEFINE_ORIG_POINTER(srand)
DEFINE_ORIG_POINTER(rand_r)
DEFINE_ORIG_POINTER(drand48)
DEFINE_ORIG_POINTER(erand48)
DEFINE_ORIG_POINTER(lrand48)
DEFINE_ORIG_POINTER(nrand48)
DEFINE_ORIG_POINTER(mrand48)
DEFINE_ORIG_POINTER(jrand48)
DEFINE_ORIG_POINTER(srand48)
DEFINE_ORIG_POINTER(seed48)
DEFINE_ORIG_POINTER(lcong48)
DEFINE_ORIG_POINTER(drand48_r)
DEFINE_ORIG_POINTER(erand48_r)
DEFINE_ORIG_POINTER(lrand48_r)
DEFINE_ORIG_POINTER(nrand48_r)
DEFINE_ORIG_POINTER(mrand48_r)
DEFINE_ORIG_POINTER(jrand48_r)
DEFINE_ORIG_POINTER(srand48_r)
DEFINE_ORIG_POINTER(seed48_r)
DEFINE_ORIG_POINTER(lcong48_r)

/* Override */ long int random (void) throw()
{
    static int count = 0;
    LINK_NAMESPACE_GLOBAL(random);
    long int ret = orig::random();
    debuglog(LCF_RANDOM, __func__, " call ", count++, ", returning ", ret);
    return ret;
}

/* Override */ void srandom (unsigned int seed) throw()
{
    debuglog(LCF_RANDOM, __func__, " call with seed ", seed);
    LINK_NAMESPACE_GLOBAL(srandom);
    return orig::srandom(seed);
}

/* Override */ char *initstate (unsigned int seed, char *statebuf,
            size_t statelen) throw()
{
    debuglog(LCF_RANDOM, __func__, " call with seed ", seed);
    LINK_NAMESPACE_GLOBAL(initstate);
    return orig::initstate(seed, statebuf, statelen);
}

/* Override */ char *setstate (char *statebuf) throw()
{
    DEBUGLOGCALL(LCF_RANDOM);
    LINK_NAMESPACE_GLOBAL(setstate);
    return orig::setstate(statebuf);
}

/* Override */ int random_r (struct random_data *buf,
             int32_t *result) throw()
{
    DEBUGLOGCALL(LCF_RANDOM);
    LINK_NAMESPACE_GLOBAL(random_r);
    return orig::random_r(buf, result);
}

/* Override */ int srandom_r (unsigned int seed, struct random_data *buf) throw()
{
    debuglog(LCF_RANDOM, __func__, " call with seed ", seed);
    LINK_NAMESPACE_GLOBAL(srandom_r);
    return orig::srandom_r(seed, buf);
}

/* Override */ int initstate_r (unsigned int seed, char *statebuf, size_t statelen,
            struct random_data *buf) throw()
{
    debuglog(LCF_RANDOM, __func__, " call with seed ", seed);
    LINK_NAMESPACE_GLOBAL(initstate_r);
    return orig::initstate_r(seed, statebuf, statelen, buf);
}

/* Override */ int setstate_r (char *statebuf, struct random_data *buf) throw()
{
    DEBUGLOGCALL(LCF_RANDOM);
    LINK_NAMESPACE_GLOBAL(setstate_r);
    return orig::setstate_r(statebuf, buf);
}

/* Override */ int rand (void) throw()
{
    static int count = 0;
    LINK_NAMESPACE_GLOBAL(rand);
    int ret = orig::rand();
    debuglog(LCF_RANDOM, __func__, " call ", count++, ", returning ", ret);
    return ret;
}

/* Override */ void srand (unsigned int seed) throw()
{
    debuglog(LCF_RANDOM, __func__, " call with seed ", seed);
    LINK_NAMESPACE_GLOBAL(srand);
    return orig::srand(seed);
}

/* Override */ int rand_r (unsigned int *seed) throw()
{
    DEBUGLOGCALL(LCF_RANDOM);
    LINK_NAMESPACE_GLOBAL(rand_r);
    return orig::rand_r(seed);
}

/* Override */ double drand48 (void) throw()
{
    DEBUGLOGCALL(LCF_RANDOM);
    LINK_NAMESPACE_GLOBAL(drand48);
    return orig::drand48();
}

/* Override */ double erand48 (unsigned short int xsubi[3]) throw()
{
    DEBUGLOGCALL(LCF_RANDOM);
    LINK_NAMESPACE_GLOBAL(erand48);
    return orig::erand48(xsubi);
}

/* Override */ long int lrand48 (void) throw()
{
    DEBUGLOGCALL(LCF_RANDOM);
    LINK_NAMESPACE_GLOBAL(lrand48);
    return orig::lrand48();
}

/* Override */ long int nrand48 (unsigned short int xsubi[3]) throw()
{
    DEBUGLOGCALL(LCF_RANDOM);
    LINK_NAMESPACE_GLOBAL(nrand48);
    return orig::nrand48(xsubi);
}

/* Override */ long int mrand48 (void) throw()
{
    DEBUGLOGCALL(LCF_RANDOM);
    LINK_NAMESPACE_GLOBAL(mrand48);
    return orig::mrand48();
}

/* Override */ long int jrand48 (unsigned short int xsubi[3]) throw()
{
    DEBUGLOGCALL(LCF_RANDOM);
    LINK_NAMESPACE_GLOBAL(jrand48);
    return orig::jrand48(xsubi);
}

/* Override */ void srand48 (long int seedval) throw()
{
    debuglog(LCF_RANDOM, __func__, " call with seed ", seedval);
    LINK_NAMESPACE_GLOBAL(srand48);
    return orig::srand48(seedval);
}

/* Override */ unsigned short int *seed48 (unsigned short int seed16v[3]) throw()
{
    debuglog(LCF_RANDOM, __func__, " call with seed ", seed16v[0], " ", seed16v[1], " ", seed16v[2]);
    LINK_NAMESPACE_GLOBAL(seed48);
    return orig::seed48(seed16v);
}

/* Override */ void lcong48 (unsigned short int param[7]) throw()
{
    DEBUGLOGCALL(LCF_RANDOM);
    LINK_NAMESPACE_GLOBAL(lcong48);
    return orig::lcong48(param);
}

/* Override */ int drand48_r (struct drand48_data *buffer, double *result) throw()
{
    DEBUGLOGCALL(LCF_RANDOM);
    LINK_NAMESPACE_GLOBAL(drand48_r);
    return orig::drand48_r(buffer, result);
}

/* Override */ int erand48_r (unsigned short int xsubi[3],
              struct drand48_data *buffer, double *result) throw()
{
    DEBUGLOGCALL(LCF_RANDOM);
    LINK_NAMESPACE_GLOBAL(erand48_r);
    return orig::erand48_r(xsubi, buffer, result);
}

/* Override */ int lrand48_r (struct drand48_data *buffer, long int *result) throw()
{
    DEBUGLOGCALL(LCF_RANDOM);
    LINK_NAMESPACE_GLOBAL(lrand48_r);
    return orig::lrand48_r(buffer, result);
}

/* Override */ int nrand48_r (unsigned short int xsubi[3],
              struct drand48_data *buffer, long int *result) throw()
{
    DEBUGLOGCALL(LCF_RANDOM);
    LINK_NAMESPACE_GLOBAL(nrand48_r);
    return orig::nrand48_r(xsubi, buffer, result);
}

/* Override */ int mrand48_r (struct drand48_data *buffer, long int *result) throw()
{
    DEBUGLOGCALL(LCF_RANDOM);
    LINK_NAMESPACE_GLOBAL(mrand48_r);
    return orig::mrand48_r(buffer, result);
}

/* Override */ int jrand48_r (unsigned short int xsubi[3],
              struct drand48_data *buffer, long int *result) throw()
{
    DEBUGLOGCALL(LCF_RANDOM);
    LINK_NAMESPACE_GLOBAL(jrand48_r);
    return orig::jrand48_r(xsubi, buffer, result);
}

/* Override */ int srand48_r (long int seedval, struct drand48_data *buffer) throw()
{
    debuglog(LCF_RANDOM, __func__, " call with seed ", seedval);
    LINK_NAMESPACE_GLOBAL(srand48_r);
    return orig::srand48_r(seedval, buffer);
}

/* Override */ int seed48_r (unsigned short int seed16v[3],
             struct drand48_data *buffer) throw()
{
    debuglog(LCF_RANDOM, __func__, " call with seed ", seed16v[0], " ", seed16v[1], " ", seed16v[2]);
    LINK_NAMESPACE_GLOBAL(seed48_r);
    return orig::seed48_r(seed16v, buffer);
}

/* Override */ int lcong48_r (unsigned short int param[7],
              struct drand48_data *buffer) throw()
{
    DEBUGLOGCALL(LCF_RANDOM);
    LINK_NAMESPACE_GLOBAL(lcong48_r);
    return orig::lcong48_r(param, buffer);
}

}
