# FreeRTOS Kernel provenance

- Repository: https://github.com/FreeRTOS/FreeRTOS-Kernel
- Version: V11.3.0
- Commit: `9b777ae5c5b8e9e456065a00294d1e5f5f9facf5`
- License: MIT, retained in `LICENSE.md`

## Imported components

- Kernel source and public headers
- GCC `ARM_CM4F` and `ARM_CM4_MPU` ports
- Common MPU wrapper sources
- `heap_4.c`

The CMSIS-RTOS v2 wrapper, `FreeRTOSConfig.h`, and `rtos_expand.*` are maintained
by this project and are not copied from the FreeRTOS Kernel repository.

## Local integration

- `configRECORD_STACK_HIGH_ADDRESS` is enabled so diagnostics can report each
  task's configured stack size.
- `tasks.c` and `task.h` expose the small stack/name helpers required by
  CmBacktrace and the task status screen.
- Application allocation tracking remains in `src/ram/user_memory.*`; the
  kernel itself uses the upstream `pvPortMalloc` and `vPortFree` interfaces.
- The MPU integration uses an application-allocated heap so the large heap is
  not placed in the privileged-data MPU region.
- The Cortex-M4 MPU port makes the general peripheral region privileged-only;
  unprivileged tasks receive explicit peripheral regions when required.
- Cortex-M4 MPU privilege and buffer checks avoid querying task MPU settings
  before the scheduler has selected a current task.
