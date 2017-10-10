/*
Linker script for STM32F20x
Copyright RAISONANCE 2007 (modified by Lanchon 1-Feb-2008)
You can use, copy and distribute this file freely, but without any waranty.
Configure memory sizes, end of stack and boot mode for your project here.
*/

__Stack_Size = 1024 ;

PROVIDE ( _Stack_Size = __Stack_Size ) ;

__Stack_Init = _estack  - __Stack_Size ;

/*"PROVIDE" allows to easily override these values from an object file or the commmand line.*/
PROVIDE ( _Stack_Init = __Stack_Init ) ;

/*
There will be a link error if there is not this amount of RAM free at the end.
*/
_Minimum_Stack_Size = 0x100 ;

/* Memory Spaces Definitions */
MEMORY
{
	ISR_VEC (rx)	: ORIGIN = 0x00000000, LENGTH = 16K     		/* must start at address 0, contains initial vector table */

	/*0x00008000 - 0x0000FFFF, 32k*/
	EEPROM_SIMU (r)	: ORIGIN = 0x00008000, LENGTH = 32K				/* Needs to be as long as 2 flash pages and mapped to flash page boundaries */
	SCRIPT (r)		: ORIGIN = 0x00010000, LENGTH = 64K
	FLASH (rx)		: ORIGIN = 0x00020000, LENGTH = (512K - 128K)
	OPTION_BYTES (r): ORIGIN = 0x1FFFF800, LENGTH = 16
	RAM (xrw)		: ORIGIN = 0x20000000, LENGTH = 128K  /* also change _estack below */
}

laStartRamTest_g = ORIGIN(RAM);
laEndRamTest_g = ORIGIN(RAM) + LENGTH(RAM) - 1; 

laStartFlash_g = ORIGIN(FLASH);								/* begin of application flash area */
laEndFlash_g = ORIGIN(FLASH) + LENGTH(FLASH) - 1;			/* end of application flash area */

laStartScript_g = ORIGIN(SCRIPT);							/* begin of script flash area */
laEndScript_g = ORIGIN(SCRIPT) + LENGTH(SCRIPT) - 1;		/* end of script flash area */

/* highest address of the user mode stack */
_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
    /* for Cortex devices, the beginning of the startup code is stored in the .isr_vector section, which goes to FLASH */
    /* the program code is stored in the .text section, which goes to Flash */

    .isrVec :
    {
		KEEP(*(.isr_vector))
    } >ISR_VEC

    /* This is the initialized data section
    The program executes knowing that the data is in the RAM
    but the loader puts the initial values in the FLASH (inidata).
    It is one task of the startup to copy the initial values from FLASH to RAM. */
    .data  : AT ( _sidata )
    {
	    . = ALIGN(4);
        /* This is used by the startup in order to initialize the .data secion */
        _sdata = . ;
        
        *(.data)
        *(.data.*)

	    . = ALIGN(4);
	    /* This is used by the startup in order to initialize the .data secion */
   	 _edata = . ;
 	 
    } >RAM

    .eepromSimu :
    {
        laStartEepromSimu_g = .;
        laEndEepromSimu_g = . + LENGTH(EEPROM_SIMU);    /* Length of 2 Pages */
    
    } >EEPROM_SIMU
    
   .text :
    {
        laStartFirmwareCrc_g = .;
        KEEP(*(.applicationDescr));    /* must be first segement, referenced by firmware loader */

        . = ALIGN(4);
        *(.text)                   /* remaining code */
        *(.text.*)                   /* remaining code */
        *(.rodata)                 /* read-only data (constants) */
        *(.rodata*)
        *(.glue_7)
        *(.glue_7t)

        . = ALIGN(4);
         _etext = .;

    } >FLASH

	.crc (_etext + SIZEOF(.data)) :
	{
	    . = ALIGN(4);
		laEndFirmwareCrc_g = . - 1;
        *(.firmwareCrc)
	} >FLASH

    /* This is used by the startup in order to initialize the .data secion */
    _sidata = _etext;


    /* This is the uninitialized data section */
    .bss :
    {
	    . = ALIGN(4);
        /* This is used by the startup in order to initialize the .bss secion */
        _sbss = .;
        
        *(.bss)
        *(COMMON)
        
	    . = ALIGN(4);
	    /* This is used by the startup in order to initialize the .bss secion */
   	 _ebss = . ;
    } >RAM
    
    PROVIDE ( end = _ebss );
    PROVIDE ( _end = _ebss );
    
    .scriptFlash :
    {
         *(.scriptFlashLen)
        . = ALIGN (4);
         *(.scriptFlash)
    } >SCRIPT

    .option_bytes :
    {
		
    } >OPTION_BYTES

    /* after that it's only debugging information. */
    
    /* remove the debugging information from the standard libraries */
    /DISCARD/ :
    {
     libc.a ( * )
     libm.a ( * )
     libgcc.a ( * )
     }

    /* Stabs debugging sections.  */
    .stab          0 : { *(.stab) }
    .stabstr       0 : { *(.stabstr) }
    .stab.excl     0 : { *(.stab.excl) }
    .stab.exclstr  0 : { *(.stab.exclstr) }
    .stab.index    0 : { *(.stab.index) }
    .stab.indexstr 0 : { *(.stab.indexstr) }
    .comment       0 : { *(.comment) }
    /* DWARF debug sections.
       Symbols in the DWARF debugging sections are relative to the beginning
       of the section so we begin them at 0.  */
    /* DWARF 1 */
    .debug          0 : { *(.debug) }
    .line           0 : { *(.line) }
    /* GNU DWARF 1 extensions */
    .debug_srcinfo  0 : { *(.debug_srcinfo) }
    .debug_sfnames  0 : { *(.debug_sfnames) }
    /* DWARF 1.1 and DWARF 2 */
    .debug_aranges  0 : { *(.debug_aranges) }
    .debug_pubnames 0 : { *(.debug_pubnames) }
    /* DWARF 2 */
    .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
    .debug_abbrev   0 : { *(.debug_abbrev) }
    .debug_line     0 : { *(.debug_line) }
    .debug_frame    0 : { *(.debug_frame) }
    .debug_str      0 : { *(.debug_str) }
    .debug_loc      0 : { *(.debug_loc) }
    .debug_macinfo  0 : { *(.debug_macinfo) }
    /* SGI/MIPS DWARF 2 extensions */
    .debug_weaknames 0 : { *(.debug_weaknames) }
    .debug_funcnames 0 : { *(.debug_funcnames) }
    .debug_typenames 0 : { *(.debug_typenames) }
    .debug_varnames  0 : { *(.debug_varnames) }
/*
   _end = .;
   PROVIDE(end = .);*/
}
