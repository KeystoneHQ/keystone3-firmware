.extern HardFault_Handler

.global Default_Handler

	.word _sidata  
	.word _sdata
	.word _edata
	.word _sbss
	.word _ebss

/********************************************************/
.section .text.Default_Handler,"ax",%progbits

Default_Handler:
Infinite_Loop:
		b Infinite_Loop
		
.size Default_Handler, .-Default_Handler
/********************************************************/

/*********************vector table***********************/
.section .isr_vector,"a",%progbits

__Vectors:
	.word _estack
	.word Reset_Handler
	.word NMI_Handler
	.word HardFault_Handler
	.word MemManage_Handler
	.word BusFault_Handler
	.word UsageFault_Handler
	.word 0
	.word 0
	.word 0
	.word 0
	.word SVC_Handler
	.word DebugMon_Handler
	.word 0
	.word PendSV_Handler
	.word SysTick_Handler
	
	@;External Interrupts
	.word DMA0_IRQHandler
	.word USB_IRQHandler
	.word USBDMA_IRQHandler
	.word LCD_IRQHandler
	.word SCI0_IRQHandler
	.word UART0_IRQHandler
	.word UART1_IRQHandler
	.word SPI0_IRQHandler
	.word CRYPT0_IRQHandler
	.word TIM0_0_IRQHandler
	.word TIM0_1_IRQHandler
	.word TIM0_2_IRQHandler
	.word TIM0_3_IRQHandler
	.word EXTI0_IRQHandler
	.word EXTI1_IRQHandler
	.word EXTI2_IRQHandler
	.word RTC_IRQHandler
	.word SENSOR_IRQHandler
	.word TRNG_IRQHandler
	.word ADC0_IRQHandler
	.word SSC_IRQHandler
	.word TIM0_4_IRQHandler
	.word TIM0_5_IRQHandler
	.word KEYBOARD_IRQHandler
	.word MSR_IRQHandler
	.word EXTI3_IRQHandler
	.word SPI1_IRQHandler
	.word SPI2_IRQHandler
	.word SCI1_IRQHandler
	.word SCI2_IRQHandler
	.word SPI3_IRQHandler
	.word SPI4_IRQHandler
	.word UART2_IRQHandler
	.word UART3_IRQHandler
	.word 0
	.word QSPI_IRQHandler
	.word I2C0_IRQHandler
	.word EXTI4_IRQHandler
	.word EXTI5_IRQHandler
	.word TIM0_6_IRQHandler
	.word TIM0_7_IRQHandler
	.word CSI2_IRQHandler
	.word DCMI_IRQHandler
	.word EXTI6_IRQHandler
	.word EXTI7_IRQHandler
	.word SDIOM_IRQHandler
	.word QR_IRQHandler
	.word GPU_IRQHandler
	.word 0
	.word 0	
	.word DAC_IRQHandler
__Vectors_End:

.equ __Vectors_Size,__Vectors_End-__Vectors
/********************************************************/

/*********************reset handle***********************/
.section .text.Reset_Handler
.type Reset_Handler, %function

Reset_Handler:
	movs r0, #0
	movs r1, #0
	movs r2, #0
	movs r3, #0
	movs r4, #0
	movs r5, #0
	movs r6, #0
	movs r7, #0

	LDR R0, =__Vectors
	LDR R1, =0xE000ED08
	STR R0,[R1]
	LDR R0,[R0]
	MOV SP,R0
	
	
	ldr r0, =0x0
	ldr r1, =_sidata
	ldr r2, =_sdata 
	ldr r3, =_edata 
	b LoopCopyDataInit

CopyDataInit:
	ldr r4, [r1, r0]
	str r4, [r2, r0]
	add r0, r0, #4

LoopCopyDataInit:
	add r4, r2, r0
	cmp r3, r4
	bgt CopyDataInit
	
	movs r0, #0
	ldr r1,=_sbss
	ldr r2,=_ebss
	b LoopFillZerobss

FillZerobss:
	str r0, [r1]
	add r1, r1, #4

LoopFillZerobss:
	cmp r2, r1
	bgt FillZerobss
	
	LDR R0, =main
	BX R0
/********************************************************/
	
.section .text
	
	.weak NMI_Handler
	.thumb_set NMI_Handler,Default_Handler
	
	.weak HardFault_Handler
	.thumb_set HardFault_Handler,Default_Handler
	
	.weak MemManage_Handler
	.thumb_set MemManage_Handler,Default_Handler
	
	.weak BusFault_Handler
	.thumb_set BusFault_Handler,Default_Handler
	
	.weak UsageFault_Handler
	.thumb_set UsageFault_Handler,Default_Handler
	
	.weak SVC_Handler
	.thumb_set SVC_Handler,Default_Handler
	
	.weak DebugMon_Handler
	.thumb_set DebugMon_Handler,Default_Handler
	
	.weak PendSV_Handler
	.thumb_set PendSV_Handler,Default_Handler
	
	.weak SysTick_Handler
	.thumb_set SysTick_Handler,Default_Handler
	
	.weak DMA0_IRQHandler
	.weak USB_IRQHandler
	.weak USBDMA_IRQHandler
	.weak LCD_IRQHandler
	.weak SCI0_IRQHandler
	.weak UART0_IRQHandler
	.weak UART1_IRQHandler
	.weak SPI0_IRQHandler
	.weak CRYPT0_IRQHandler
	.weak TIM0_0_IRQHandler
	.weak TIM0_1_IRQHandler
	.weak TIM0_2_IRQHandler
	.weak TIM0_3_IRQHandler
	.weak EXTI0_IRQHandler
	.weak EXTI1_IRQHandler
	.weak EXTI2_IRQHandler
	.weak RTC_IRQHandler
	.weak SENSOR_IRQHandler
	.weak TRNG_IRQHandler
	.weak ADC0_IRQHandler
	.weak SSC_IRQHandler
	.weak TIM0_4_IRQHandler
	.weak TIM0_5_IRQHandler
	.weak KEYBOARD_IRQHandler
	.weak MSR_IRQHandler
	.weak EXTI3_IRQHandler
	.weak SPI1_IRQHandler
	.weak SPI2_IRQHandler
	.weak SCI1_IRQHandler
	.weak SCI2_IRQHandler
	.weak SPI3_IRQHandler
	.weak SPI4_IRQHandler
	.weak UART2_IRQHandler
	.weak UART3_IRQHandler	
	.weak QSPI_IRQHandler
	.weak I2C0_IRQHandler
	.weak EXTI4_IRQHandler
	.weak EXTI5_IRQHandler
	.weak TIM0_6_IRQHandler
	.weak TIM0_7_IRQHandler
	.weak CSI2_IRQHandler
	.weak DCMI_IRQHandler
	.weak EXTI6_IRQHandler
	.weak EXTI7_IRQHandler
	.weak SDIOM_IRQHandler
	.weak QR_IRQHandler
	.weak GPU_IRQHandler
	.weak DAC_IRQHandler
