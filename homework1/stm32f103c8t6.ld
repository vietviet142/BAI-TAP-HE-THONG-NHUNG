/* Entry point */
ENTRY(Reset_Handler)

/* Memory layout */
MEMORY
{
  FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
  RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 20K
}

/* Stack pointer (match với startup: _estack) */
_estack = ORIGIN(RAM) + LENGTH(RAM);

/* Sections */
SECTIONS
{
  /* Vector table + code */
  .text :
  {
    KEEP(*(.isr_vector))     /* vector table */
    *(.text*)                /* code */
    *(.rodata*)              /* const data */
    *(.glue_7)
    *(.glue_7t)
    *(.eh_frame)

    _etext = .;              /* end of code (dùng cho copy data) */
  } > FLASH

  /* Data section (copy từ FLASH -> RAM) */
  .data : AT(_etext)
  {
    _sdata = .;              /* start data */
    *(.data*)
    _edata = .;              /* end data */
  } > RAM

  /* BSS (zero init) */
  .bss :
  {
    _sbss = .;               /* start bss */
    *(.bss*)
    *(COMMON)
    _ebss = .;               /* end bss */
  } > RAM

  /* Stack + heap (optional) */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + 0x1000;          /* heap + stack reserve (4KB tùy chỉnh) */
    . = ALIGN(8);
  } > RAM
}

/* Symbol cho startup dùng */
_sidata = LOADADDR(.data);