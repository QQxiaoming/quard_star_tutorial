OUTPUT_ARCH( "riscv" )

ENTRY( _start )

MEMORY
{ 
	flash (rxai!w) : ORIGIN = 0x20000000, LENGTH = 512k
	sram  (rxaiw)  : ORIGIN = 0x00008000, LENGTH = 32k
}

SECTIONS
{
  .text :
  {
    KEEP(*(.text))
  } >flash

  .data :
  {
    KEEP(*(.data))
  } >sram AT >flash
}