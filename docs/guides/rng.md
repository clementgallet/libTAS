---
layout: page
title: RNG sources
permalink: /rng/
---

This page list possible sources that games may use for their Random Number Generator (RNG), and which options control them in libTAS.

* TOC
{:toc}

## Sources

Besides of the [regular in-game sources of randomness](https://tasvideos.org/LuckManipulation), games may get data from other sources to update their PRNG. Some of these sources are configurable, while libTAS provides fixed values for others.

For more information about these different sources, see [the Tech details page](how).

### Configurable

* System time: can be configured on the main window

### Currently not configurable

* reading `/proc/cputime`
* using the current process pid
* reading `/dev/urandom`
