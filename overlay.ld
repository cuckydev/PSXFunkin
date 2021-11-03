. = __bss_end;
. = ALIGN(16);
__heap_base = .;

SECTIONS {
	OVERLAY __heap_base : NOCROSSREFS SUBALIGN(4)
	{
		.menu
		{
			KEEP(src/menu/*.o(.text))
			__menu_ctor = .;
			KEEP(src/menu/*.o(.text.startup._GLOBAL__*))
			KEEP(src/menu/*.o(.text.*))
			KEEP(src/menu/*.o(.rodata*))
			KEEP(src/menu/*.o(.sdata*))
			KEEP(src/menu/*.o(.data*))
			KEEP(src/menu/*.o(.sbss*))
			KEEP(src/menu/*.o(.bss*))
			KEEP(src/menu/*.o(.ctors))
			
			. = ALIGN(4);
			__menu_end = .;
		}
		.week1
		{
			KEEP(src/week1/*.o(.text))
			__week1_ctor = .;
			KEEP(src/week1/*.o(.text.startup._GLOBAL__*))
			KEEP(src/week1/*.o(.text.*))
			KEEP(src/week1/*.o(.rodata*))
			KEEP(src/week1/*.o(.sdata*))
			KEEP(src/week1/*.o(.data*))
			KEEP(src/week1/*.o(.sbss*))
			KEEP(src/week1/*.o(.bss*))
			KEEP(src/week1/*.o(.ctors))
			
			. = ALIGN(4);
			__week1_end = .;
		}
	}
}
