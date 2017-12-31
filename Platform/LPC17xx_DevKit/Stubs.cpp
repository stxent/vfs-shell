/*
 * Stubs.cpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

void *__dso_handle = nullptr;

namespace std {

void __throw_bad_alloc()
{
  while (1);
}

void __throw_bad_function_call(void)
{
  while (1);
}

void __throw_length_error(char const *)
{
  while (1);
}

void __throw_logic_error(const char *)
{
  while (1);
}

void __throw_out_of_range(const char *)
{
  while (1);
}

}

extern "C" void __assert_func(const char *, int, const char *, const char *)
{
  while (1);
}

extern "C" void __cxa_deleted_virtual()
{
  while (1);
}

extern "C" void __cxa_pure_virtual()
{
  while (1);
}
