tree of projects:
	/homework1$ tree
	.
	├── lib
	│   ├── ADC_JOYSTICK.c
	│   ├── ADC_JOYSTICK.h
	│   ├── GPIO.c
	│   ├── GPIO.h
	│   ├── HC_SR04.c
	│   ├── HC_SR04.h
	│   ├── L298N.c
	│   ├── L298N.h
	│   ├── core_cm3.h
	│   ├── detect_line.c
	│   ├── detect_line.h
	│   ├── misc.c
	│   ├── misc.h
	│   ├── stm32f10x.h
	│   ├── stm32f10x_adc.c
	│   ├── stm32f10x_adc.h
	│   ├── stm32f10x_conf.h
	│   ├── stm32f10x_gpio.c
	│   ├── stm32f10x_gpio.h
	│   ├── stm32f10x_rcc.c
	│   ├── stm32f10x_rcc.h
	│   ├── system_stm32f10x.c
	│   └── system_stm32f10x.h
	├── makefile
	├── readme.md
	├── src
	│   ├── bai1.c
	│   ├── bai2.c
	│   ├── bai3.c
	│   ├── bai4.c
	│   └── main.c
	├── startup_stm32f10x_md.s
	└── stm32f103c8t6.ld
run:
	config makefile: select source to build
		#APP = src/bai1.c
		#APP = src/bai2.c
		 APP = src/bai3.c
		#APP = src/bai4.c
cmd at folder contain makefile: 
	make clean -> make -> make flash