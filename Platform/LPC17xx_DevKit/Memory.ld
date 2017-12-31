/*
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

ENTRY(RESET_ISR)
EXTERN(vectorTable)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 0x80000 /* 512 KiB */
  RAM (rwx)  : ORIGIN = 0x2007C000, LENGTH = 0x08000 /* 32 KiB */
}

stackStart = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
  .text : ALIGN(4)
  {
    _stext = .;

    *(.vectors)
    *(.text)
    *(.text*)
    *(.rodata)
    *(.rodata*)

    __preinit_array_start = .;
    KEEP(*(.preinit_array*))
    __preinit_array_end = .;
    __init_array_start = .;
    KEEP(*(.init_array*))
    __init_array_end = .;

    *(.init)
    *(.init*)
    *(.fini)
    *(.fini*)
    *(.ctors)
    *(.ctors*)
    *(.dtors)
    *(.dtors*)
    *(.glue_7)
    *(.glue_7t)
    *(.gnu*)
    *(.gcc*)

    _etext = .;

    . = ALIGN(4);
    _sidata = .;
  } >FLASH =0xFF

  . = ALIGN(4);
  .init_array :
  {
  __init_array_start = .;
    KEEP (*(.init_array*))
    __init_array_end = .;
  } >FLASH

  .data : AT(_sidata) ALIGN(4)
  {
    _sdata = .;

    *(.data)
    *(.data*)

    _edata = .;
  } >RAM

  .bss : ALIGN(4)
  {
    _sbss = .;

    *(.bss)
    *(.bss*)
    *(COMMON)

    _ebss = .;

    . = ALIGN(4);
    heapStart = .;
  } >RAM

  /DISCARD/ :
  {
    *(.init)
    *(.fini)

    *(.ARM.exidx*)
    *(.gnu.linkonce.armexidx.*)

    *(.ARM.extab*)
    *(.gnu.linkonce.armextab.*)
  }

  PROVIDE(end = heapStart);
  PROVIDE(_stack = stackStart);
}
