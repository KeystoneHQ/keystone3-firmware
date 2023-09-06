/*
 * This file is part of the CmBacktrace Library.
 *
 * Copyright (c) 2020, Armink, <armink.ztl@gmail.com>
 *                     Chenxuan, <chenxuan.zhao@icloud.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * NOTE: DO NOT include this file on the header file.
 * Encoding: GB18030
 * Created on: 2020-09-06
 */

[PRINT_MAIN_STACK_CFG_ERROR]  = "�����޷���ȡ��ջ��Ϣ��������ջ���������",
[PRINT_FIRMWARE_INFO]         = "�̼����ƣ�%s��Ӳ���汾�ţ�%s������汾�ţ�%s",
[PRINT_ASSERT_ON_THREAD]      = "���߳�(%s)�з�������",
[PRINT_ASSERT_ON_HANDLER]     = "���жϻ���������·�������",
[PRINT_THREAD_STACK_INFO]     = "=========== �̶߳�ջ��Ϣ ===========",
[PRINT_MAIN_STACK_INFO]       = "============ ����ջ��Ϣ ============",
[PRINT_THREAD_STACK_OVERFLOW] = "�����߳�ջ(%08x)�������",
[PRINT_MAIN_STACK_OVERFLOW]   = "������ջ(%08x)�������",
[PRINT_CALL_STACK_INFO]       = "�鿴���ຯ������ջ��Ϣ�������У�addr2line -e %s%s -a -f %s",
[PRINT_CALL_STACK_ERR]        = "��ȡ��������ջʧ��",
[PRINT_FAULT_ON_THREAD]       =  "���߳�(%s)�з��������쳣",
[PRINT_FAULT_ON_HANDLER]      = "���жϻ���������·��������쳣",
[PRINT_REGS_TITLE]            = "========================= �Ĵ�����Ϣ =========================",
[PRINT_HFSR_VECTBL]           = "����Ӳ����ԭ��ȡ�ж�����ʱ����",
[PRINT_MFSR_IACCVIOL]         = "�����洢���������ԭ����ͼ�Ӳ�������ʵ�����ȡָ��",
[PRINT_MFSR_DACCVIOL]         = "�����洢���������ԭ����ͼ�Ӳ�������ʵ��������д����",
[PRINT_MFSR_MUNSTKERR]        = "�����洢���������ԭ�򣺳�ջʱ��ͼ���ʲ������������",
[PRINT_MFSR_MSTKERR]          = "�����洢���������ԭ����ջʱ��ͼ���ʲ������������",
[PRINT_MFSR_MLSPERR]          = "�����洢���������ԭ�򣺶��Ա��渡��״̬ʱ��������",
[PRINT_BFSR_IBUSERR]          = "�������ߴ���ԭ��ָ�����ߴ���",
[PRINT_BFSR_PRECISERR]        = "�������ߴ���ԭ�򣺾�ȷ���������ߴ���",
[PRINT_BFSR_IMPREISERR]       = "�������ߴ���ԭ�򣺲���ȷ���������ߴ���",
[PRINT_BFSR_UNSTKERR]         = "�������ߴ���ԭ�򣺳�ջʱ��������",
[PRINT_BFSR_STKERR]           = "�������ߴ���ԭ����ջʱ��������",
[PRINT_BFSR_LSPERR]           = "�������ߴ���ԭ�򣺶��Ա��渡��״̬ʱ��������",
[PRINT_UFSR_UNDEFINSTR]       = "�����÷�����ԭ����ͼִ��δ����ָ��",
[PRINT_UFSR_INVSTATE]         = "�����÷�����ԭ����ͼ�л��� ARM ״̬",
[PRINT_UFSR_INVPC]            = "�����÷�����ԭ����Ч���쳣������",
[PRINT_UFSR_NOCP]             = "�����÷�����ԭ����ͼִ��Э������ָ��",
#if (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M33)
    [PRINT_UFSR_STKOF]        = "�����÷�����ԭ��Ӳ����⵽ջ���",
#endif
[PRINT_UFSR_UNALIGNED]        = "�����÷�����ԭ����ͼִ�зǶ������",
[PRINT_UFSR_DIVBYZERO0]       = "�����÷�����ԭ����ͼִ�г� 0 ����",
[PRINT_DFSR_HALTED]           = "�������Դ���ԭ��NVIC ͣ������",
[PRINT_DFSR_BKPT]             = "�������Դ���ԭ��ִ�� BKPT ָ��",
[PRINT_DFSR_DWTTRAP]          = "�������Դ���ԭ�����ݼ���ƥ��",
[PRINT_DFSR_VCATCH]           = "�������Դ���ԭ�򣺷�����������",
[PRINT_DFSR_EXTERNAL]         = "�������Դ���ԭ���ⲿ��������",
[PRINT_MMAR]                  = "�����洢���������ĵ�ַ��%08x",
[PRINT_BFAR]                  = "�������ߴ���ĵ�ַ��%08x",
