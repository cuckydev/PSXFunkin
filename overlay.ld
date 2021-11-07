. = __bss_end;
. = ALIGN(16);
__heap_base = .;

SECTIONS {
	OVERLAY __heap_base : NOCROSSREFS SUBALIGN(4)
	{
		.menu
		{
			. = . + 2;
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
			. = . + 2;
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
		.week2
		{
			. = . + 2;
			KEEP(src/week2/*.o(.text))
			__week2_ctor = .;
			KEEP(src/week2/*.o(.text.startup._GLOBAL__*))
			KEEP(src/week2/*.o(.text.*))
			KEEP(src/week2/*.o(.rodata*))
			KEEP(src/week2/*.o(.sdata*))
			KEEP(src/week2/*.o(.data*))
			KEEP(src/week2/*.o(.sbss*))
			KEEP(src/week2/*.o(.bss*))
			KEEP(src/week2/*.o(.ctors))
			
			. = ALIGN(4);
			__week2_end = .;
		}
		.week3
		{
			. = . + 2;
			KEEP(src/week3/*.o(.text))
			__week3_ctor = .;
			KEEP(src/week3/*.o(.text.startup._GLOBAL__*))
			KEEP(src/week3/*.o(.text.*))
			KEEP(src/week3/*.o(.rodata*))
			KEEP(src/week3/*.o(.sdata*))
			KEEP(src/week3/*.o(.data*))
			KEEP(src/week3/*.o(.sbss*))
			KEEP(src/week3/*.o(.bss*))
			KEEP(src/week3/*.o(.ctors))
			
			. = ALIGN(4);
			__week3_end = .;
		}
	}
}
