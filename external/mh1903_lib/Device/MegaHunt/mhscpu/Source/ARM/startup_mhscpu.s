;/**************************************************************************//**
; * @file     startup_<Device>.s
; * @brief    CMSIS Cortex-M# Core Device Startup File for
; *           Device <Device>
; * @version  V3.10
; * @date     23. November 2012
; *
; * @note
; *
; ******************************************************************************/
;/* Copyright (c) 2012 ARM LIMITED
;
;   All rights reserved.
;   Redistribution and use in source and binary forms, with or without
;   modification, are permitted provided that the following conditions are met:
;   - Redistributions of source code must retain the above copyright
;     notice, this list of conditions and the following disclaimer.
;   - Redistributions in binary form must reproduce the above copyright
;     notice, this list of conditions and the following disclaimer in the
;     documentation and/or other materials provided with the distribution.
;   - Neither the name of ARM nor the names of its contributors may be used
;     to endorse or promote products derived from this software without
;     specific prior written permission.
;   *
;   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
;   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
;   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
;   ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
;   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
;   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
;   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
;   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
;   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
;   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
;   POSSIBILITY OF SUCH DAMAGE.
;   ---------------------------------------------------------------------------*/
;/*
;//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------
;*/


; <h> Stack Configuration
;   <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Stack_Size      EQU     0x00008000

                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp


; <h> Heap Configuration
;   <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Heap_Size       EQU     0x00008000

                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE   Heap_Size
__heap_limit


                PRESERVE8
                THUMB


; Vector Table Mapped to Address 0 at Reset

                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size

__Vectors       DCD     __initial_sp              ; Top of Stack
                DCD     Reset_Handler             ; Reset Handler
                DCD     NMI_Handler               ; NMI Handler
                DCD     HardFault_Handler         ; Hard Fault Handler
                DCD     MemManage_Handler         ; MPU Fault Handler
                DCD     BusFault_Handler          ; Bus Fault Handler
                DCD     UsageFault_Handler        ; Usage Fault Handler
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     SVC_Handler               ; SVCall Handler
                DCD     DebugMon_Handler          ; Debug Monitor Handler
                DCD     0                         ; Reserved
                DCD     PendSV_Handler            ; PendSV Handler
                DCD     SysTick_Handler           ; SysTick Handler

                ; External Interrupts
                DCD     DMA0_IRQHandler
				DCD     USB_IRQHandler
                DCD     USBDMA_IRQHandler
                DCD     LCD_IRQHandler
                DCD     SCI0_IRQHandler
                DCD     UART0_IRQHandler
                DCD     UART1_IRQHandler
                DCD 	SPI0_IRQHandler
                DCD		CRYPT0_IRQHandler
                DCD		TIM0_0_IRQHandler
                DCD		TIM0_1_IRQHandler
                DCD		TIM0_2_IRQHandler
                DCD		TIM0_3_IRQHandler
                DCD		EXTI0_IRQHandler
                DCD		EXTI1_IRQHandler
                DCD		EXTI2_IRQHandler
                DCD		RTC_IRQHandler
                DCD		SENSOR_IRQHandler
                DCD		TRNG_IRQHandler
                DCD		ADC0_IRQHandler
				DCD		SSC_IRQHandler
				DCD		TIM0_4_IRQHandler
				DCD		TIM0_5_IRQHandler
				DCD		KEYBOARD_IRQHandler
				DCD		MSR_IRQHandler
				DCD		EXTI3_IRQHandler
				DCD		SPI1_IRQHandler
				DCD		SPI2_IRQHandler
				DCD     SCI1_IRQHandler
                DCD     SCI2_IRQHandler
                DCD		SPI3_IRQHandler
                DCD		SPI4_IRQHandler
                DCD     UART2_IRQHandler
                DCD     UART3_IRQHandler
                DCD     0
                DCD     QSPI_IRQHandler
                DCD     I2C0_IRQHandler
                DCD     EXTI4_IRQHandler
                DCD     EXTI5_IRQHandler
				DCD		TIM0_6_IRQHandler
				DCD		TIM0_7_IRQHandler
				DCD		CSI2_IRQHandler
				DCD     DCMI_IRQHandler
                DCD     EXTI6_IRQHandler
                DCD     EXTI7_IRQHandler
				DCD		SDIOM_IRQHandler
				DCD		QR_IRQHandler
			    DCD     GPU_IRQHandler
				DCD     0	
				DCD     AWD_IRQHandler
				DCD     DAC_IRQHandler					
__Vectors_End

__Vectors_Size  EQU     __Vectors_End - __Vectors

                AREA    |.text|, CODE, READONLY


; Reset Handler

Reset_Handler   PROC
                EXPORT  Reset_Handler             [WEAK]
				IMPORT  SystemInit
                IMPORT  __main
					
                LDR     R0, =__Vectors
                LDR     R1, =0xE000ED08
                STR     R0, [R1]
                LDR     R0, [R0]
                MOV     SP, R0
				
                LDR     R0, =SystemInit
                BLX      R0

                LDR     R0, =__main
                BX      R0
                ENDP

       
				ALIGN
				
; Dummy Exception Handlers (infinite loops which can be modified)

NMI_Handler     PROC
                EXPORT  NMI_Handler               [WEAK]
                B       .
                ENDP
HardFault_Handler\
                PROC
                EXPORT  HardFault_Handler         [WEAK]
                B       .
                ENDP
MemManage_Handler\
                PROC
                EXPORT  MemManage_Handler         [WEAK]
                B       .
                ENDP
BusFault_Handler\
                PROC
                EXPORT  BusFault_Handler          [WEAK]
                B       .
                ENDP
UsageFault_Handler\
                PROC
                EXPORT  UsageFault_Handler        [WEAK]
                B       .
                ENDP
SVC_Handler     PROC
                EXPORT  SVC_Handler               [WEAK]
                B       .
                ENDP
DebugMon_Handler\
                PROC
                EXPORT  DebugMon_Handler          [WEAK]
                B       .
                ENDP
PendSV_Handler\
                PROC
                EXPORT  PendSV_Handler            [WEAK]
                B       .
                ENDP
SysTick_Handler\
                PROC
                EXPORT  SysTick_Handler           [WEAK]
                B       .
                ENDP

Default_Handler PROC
; ToDo:  Add here the export definition for the device specific external interrupts handler
                EXPORT		DMA0_IRQHandler			[WEAK]
				EXPORT		USB_IRQHandler			[WEAK]
				EXPORT		USBDMA_IRQHandler		[WEAK]
				EXPORT		LCD_IRQHandler			[WEAK]
				EXPORT		SCI0_IRQHandler			[WEAK]
				EXPORT		UART0_IRQHandler		[WEAK]
				EXPORT		UART1_IRQHandler		[WEAK]
				EXPORT 		SPI0_IRQHandler			[WEAK]
				EXPORT		CRYPT0_IRQHandler		[WEAK]
				EXPORT		TIM0_0_IRQHandler		[WEAK]
				EXPORT		TIM0_1_IRQHandler		[WEAK]
				EXPORT		TIM0_2_IRQHandler		[WEAK]
				EXPORT		TIM0_3_IRQHandler		[WEAK]
				EXPORT		EXTI0_IRQHandler		[WEAK]
				EXPORT		EXTI1_IRQHandler		[WEAK]
				EXPORT		EXTI2_IRQHandler		[WEAK]
				EXPORT		RTC_IRQHandler			[WEAK]
				EXPORT		SENSOR_IRQHandler		[WEAK]
				EXPORT		TRNG_IRQHandler			[WEAK]
				EXPORT		ADC0_IRQHandler			[WEAK]
				EXPORT		SSC_IRQHandler			[WEAK]
				EXPORT		TIM0_4_IRQHandler		[WEAK]
				EXPORT		TIM0_5_IRQHandler		[WEAK]	
				EXPORT		KEYBOARD_IRQHandler	    [WEAK]	
				EXPORT		MSR_IRQHandler			[WEAK]	
				EXPORT		EXTI3_IRQHandler		[WEAK]	
				EXPORT 		SPI1_IRQHandler			[WEAK]
				EXPORT 		SPI2_IRQHandler			[WEAK]
				EXPORT 		SCI1_IRQHandler         [WEAK]
                EXPORT      SCI2_IRQHandler         [WEAK]
                EXPORT		SPI3_IRQHandler         [WEAK]
                EXPORT		SPI4_IRQHandler         [WEAK]
                EXPORT      UART2_IRQHandler        [WEAK]
                EXPORT      UART3_IRQHandler        [WEAK]
                EXPORT      QSPI_IRQHandler         [WEAK]
                EXPORT      I2C0_IRQHandler         [WEAK]
                EXPORT      EXTI4_IRQHandler        [WEAK]
                EXPORT      EXTI5_IRQHandler        [WEAK]
				EXPORT		TIM0_6_IRQHandler		[WEAK]
				EXPORT		TIM0_7_IRQHandler		[WEAK]
				EXPORT 		CSI2_IRQHandler			[WEAK]	
				EXPORT      DCMI_IRQHandler         [WEAK]
				EXPORT      EXTI6_IRQHandler        [WEAK]
				EXPORT      EXTI7_IRQHandler        [WEAK]  
				EXPORT		SDIOM_IRQHandler		[WEAK]
				EXPORT		QR_IRQHandler			[WEAK]	
			    EXPORT      GPU_IRQHandler          [WEAK]
				EXPORT		AWD_IRQHandler	    [WEAK]
				EXPORT	 	DAC_IRQHandler      [WEAK]
; ToDo:  Add here the names for the device specific external interrupts handler
DMA0_IRQHandler			
USB_IRQHandler			
USBDMA_IRQHandler	
LCD_IRQHandler		
SCI0_IRQHandler			
UART0_IRQHandler		
UART1_IRQHandler	
SPI0_IRQHandler		
CRYPT0_IRQHandler	
TIM0_0_IRQHandler		
TIM0_1_IRQHandler	
TIM0_2_IRQHandler	
TIM0_3_IRQHandler		
EXTI0_IRQHandler		
EXTI1_IRQHandler		
EXTI2_IRQHandler		
RTC_IRQHandler		
SENSOR_IRQHandler		
TRNG_IRQHandler		
ADC0_IRQHandler			
SSC_IRQHandler			
TIM0_4_IRQHandler	
TIM0_5_IRQHandler		
KEYBOARD_IRQHandler	  
MSR_IRQHandler		
EXTI3_IRQHandler		
SPI1_IRQHandler			
SPI2_IRQHandler			
SCI1_IRQHandler       
SCI2_IRQHandler      
SPI3_IRQHandler       
SPI4_IRQHandler        
UART2_IRQHandler       
UART3_IRQHandler             
QSPI_IRQHandler         
I2C0_IRQHandler         
EXTI4_IRQHandler       
EXTI5_IRQHandler 
TIM0_6_IRQHandler
TIM0_7_IRQHandler
CSI2_IRQHandler
DCMI_IRQHandler
EXTI6_IRQHandler
EXTI7_IRQHandler
SDIOM_IRQHandler
QR_IRQHandler	
GPU_IRQHandler   
AWD_IRQHandler	  
DAC_IRQHandler

                B       .
                ENDP


                ALIGN


; User Initial Stack & Heap

                IF      :DEF:__MICROLIB

                EXPORT  __initial_sp
                EXPORT  __heap_base
                EXPORT  __heap_limit

                ELSE

                IMPORT  __use_two_region_memory
                EXPORT  __user_initial_stackheap

__user_initial_stackheap PROC
                LDR     R0, =  Heap_Mem
                LDR     R1, =(Stack_Mem + Stack_Size)
                LDR     R2, = (Heap_Mem +  Heap_Size)
                LDR     R3, = Stack_Mem
                BX      LR
                ENDP

                ALIGN

                ENDIF


                END

