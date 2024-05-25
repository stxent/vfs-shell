/*
 * Stubs.cpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <xcore/accel.h>
#include <stdio.h>

struct _reent;
struct _stat;

void *__dso_handle = nullptr;

extern "C" void __assert_func(const char *, int, const char *, const char *)
{
  invokeDebugger();
  while (1);
}

extern "C" void __malloc_lock(_reent *)
{
}

extern "C" void __malloc_unlock(_reent *)
{
}

extern "C" int _close(int)
{
  return -1;
}

extern "C" int _fstat(int, _stat *)
{
  return -1;
}

extern "C" int _getpid()
{
	return 1;
}

extern "C" int _isatty(int)
{
	return 1;
}

extern "C" int _kill(int, int)
{
	return -1;
}

extern "C" off_t _lseek(int, off_t, int)
{
  return 0;
}

extern "C" int _open(const char *, int)
{
  return -1;
}

extern "C" int _read(int, char *, int)
{
  return -1;
}

extern "C" int _write(int, char *, int)
{
  return -1;
}
