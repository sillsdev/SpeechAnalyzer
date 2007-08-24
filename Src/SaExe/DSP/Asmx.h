#ifndef ASMX_H
 #define ASMX_H  // Assembler Extensions

/* Constant definitions                                                                */
 #define __AHSHIFT           3  // selector adjustment

#ifdef WIN32  // 32-bit compiler
/* Assembler instructions for various program definitions.                             */
 #define ADC_EAX_0 adc eax, 0
 #define ADD_EAX   add eax, 
 #define ADD_EAX_32768 add eax, 32768
 #define ADD_EAX_EBX add eax, ebx
 #define ADC_EDX add edx,
 #define ADC_EDX_ECX  add edx, ecx
 #define ADD_EDX_0x00001000 add edx, 0x00001000
 #define ADD_EDX_0x00100000 add edx, 0x00100000
 #define ADD_EDX_0x20000000 add edx, 0x20000000
 #define ADD_EDX add edx,
 #define BSF_BX_AX bsf bx,ax
 #define BSR_CX_AX bsr cx,ax
 #define CMP_EAX_0 cmp eax, 0
 #define CMP_EDX_0 cmp edx, 0
 #define CMP_EDX_32767 cmp edx, 32767
 #define CMP_EDX_NEG_32768 cmp edx, -32768
 #define CMP_EDX_0x00007FFF cmp edx, 0x00007fff
 #define CMP_EDX_0x007FFFFF cmp edx, 0x007fffff
 #define CMP_EDX_0xFF800000 cmp edx, 0xff800000
 #define CMP_EDX_0xFFFF8000 cmp edx, 0xffff8000
 #define IMUL imul
 #define MOV_EAX mov eax, 
 #define MOV_EAX_0x00000000 mov eax, 0
 #define MOV_EAX_0xFFFFFFFF mov eax, 0xffffffff
 #define MOV_EBX_EAX mov ebx, eax
 #define MOV_ECX_EDX mov ecx, edx
 #define MOV_EDX mov edx, 
 #define MOV_EDX_32767 mov edx, 32767 
 #define MOV_EDX_NEG_32768 mov edx, -32768
 #define MOV_EDX_0x00000000  mov edx, 0
 #define MOV_EDX_0x00800000  mov edx, 0x00800000 
 #define MOV_EDX_0x007FFFFF  mov edx, 0x007FFFFF
 #define MOV_EDX_0x7FFFFFFF  mov edx, 0x7fffffff
 #define MOV_EDX_0x80000000  mov edx, 0x80000000
 #define MOV_EDX_0xFF800000  mov edx, 0xff800000
 #define MOVSX_AX_BY_ES_BX   movsx ax, BYTE PTR es:[bx]
 #define MOVSX_EAX_BY_ES_BX  movsx eax, BYTE PTR es:[bx] 
 #define MOVSX_EAX_WO_ES_BX  movsx eax, WORD PTR es:[bx]
 #define MOVSX_ECX movsx ecx,
 #define MOVSX_EDX movsx edx,
 #define NEG_EAX neg eax
 #define SAR_EDX sar edx, 
 #define SBB_EAX_0 sbb eax, 0
 #define SBB_EDX sbb edx,
 #define SHL_EAX shl eax, 
 #define SHL_EBX shl ebx,
 #define SHLD_EAX_EDX shld eax, edx,
 #define SHLD_EDX_EAX shld edx, eax,
 #define SHLD_EDX_ECX shld edx, ecx,
 #define SHR_EAX shr eax,
 #define SHR_EDX shr edx,
 #define SUB_EAX_128 sub eax, 128
 #define SUB_EAX_32768 sub eax, 32768
 #define SUB_EDX_0x00001000 sub edx, 0x00001000
 #define SUB_EDX_0x00100000 sub edx, 0x00100000
 #define SUB_EDX_0x20000000 sub edx, 0x20000000
 #define SUB_EDX sub edx,
 #define XOR_EAX_0xFFFFFF80 xor eax, 0xffffff80
 
#else
/* Machine code for various unsupported 16 bit and 32 bit instructions.                */
 #define MC  __asm _emit
 #define ADC_EAX_0           MC 0x66 MC 0x15 MC 0x00 MC 0x00 MC 0x00 MC 0x00
 #define ADD_EAX             MC 0x66 MC 0x03 MC 0x46 MC
 #define ADD_EAX_32768       MC 0x66 MC 0x05 MC 0x00 MC 0x80 MC 0x00 MC 0x00
 #define ADD_EAX_EBX         MC 0x66 MC 0x03 MC 0xC3 
 #define ADC_EDX             MC 0x66 MC 0x13 MC 0x56 MC
 #define ADC_EDX_ECX         MC 0x66 MC 0x13 MC 0xD1  
 #define ADD_EDX_0x00001000  MC 0x66 MC 0x81 MC 0xC2 MC 0x00 MC 0x10 MC 0x00 MC 0x00
 #define ADD_EDX_0x00100000  MC 0x66 MC 0x81 MC 0xC2 MC 0x00 MC 0x00 MC 0x10 MC 0x00
 #define ADD_EDX_0x20000000  MC 0x66 MC 0x81 MC 0xC2 MC 0x00 MC 0x00 MC 0x00 MC 0x20
 #define ADD_EDX             MC 0x66 MC 0x03 MC 0x56 MC
 #define BSF_BX_AX           MC 0x0F MC 0xBC MC 0xD8
 #define BSR_CX_AX           MC 0x0F MC 0xBD MC 0xC8  
 #define CMP_EAX_0           MC 0x66 MC 0x3D MC 0x00 MC 0x00 MC 0x00 MC 0x00
 #define CMP_EDX_0           MC 0x66 MC 0x81 MC 0xFA MC 0x00 MC 0x00 MC 0x00 MC 0x00
 #define CMP_EDX_32767       MC 0x66 MC 0x81 MC 0xFA MC 0xFF MC 0x7F MC 0x00 MC 0x00
 #define CMP_EDX_NEG_32768   MC 0x66 MC 0x81 MC 0xFA MC 0x00 MC 0x80 MC 0xFF MC 0xFF 
 #define CMP_EDX_0x00007FFF  MC 0x66 MC 0x81 MC 0xFA MC 0xFF MC 0x7F MC 0x00 MC 0x00
 #define CMP_EDX_0x007FFFFF  MC 0x66 MC 0x81 MC 0xFA MC 0xFF MC 0xFF MC 0x7F MC 0x00
 #define CMP_EDX_0xFF800000  MC 0x66 MC 0x81 MC 0xFA MC 0x00 MC 0x00 MC 0x80 MC 0xFF
 #define CMP_EDX_0xFFFF8000  MC 0x66 MC 0x81 MC 0xFA MC 0x00 MC 0x80 MC 0xFF MC 0xFF
 #define IMUL                MC 0x66 MC 0xF7 MC 0x6E MC
 #define MOV_EAX             MC 0x66 MC 0x8B MC 0x46 MC
 #define MOV_EAX_0x00000000  MC 0x66 MC 0xB8 MC 0x00 MC 0x00 MC 0x00 MC 0x00 
 #define MOV_EAX_0xFFFFFFFF  MC 0x66 MC 0xB8 MC 0xFF MC 0xFF MC 0xFF MC 0xFF 
 #define MOV_EAX_TO          MC 0x66 MC 0x89 MC 0x46 MC
 #define MOV_EBX_EAX         MC 0x66 MC 0x8B MC 0xD8
 #define MOV_ECX_EDX         MC 0x66 MC 0x8B MC 0xCA
 #define MOV_EDX             MC 0x66 MC 0x8B MC 0x56 MC
 #define MOV_EDX_32767       MC 0x66 MC 0xBA MC 0xFF MC 0x7F MC 0x00 MC 0x00
 #define MOV_EDX_NEG_32768   MC 0x66 MC 0xBA MC 0x00 MC 0x80 MC 0xFF MC 0xFF
 #define MOV_EDX_0x00000000  MC 0x66 MC 0xBA MC 0x00 MC 0x00 MC 0x00 MC 0x00
 #define MOV_EDX_0x00800000  MC 0x66 MC 0xBA MC 0x00 MC 0x00 MC 0x80 MC 0x00
 #define MOV_EDX_0x007FFFFF  MC 0x66 MC 0xBA MC 0xFF MC 0xFF MC 0x7F MC 0x00
 #define MOV_EDX_0x7FFFFFFF  MC 0x66 MC 0xBA MC 0xFF MC 0xFF MC 0xFF MC 0x7F
 #define MOV_EDX_0x80000000  MC 0x66 MC 0xBA MC 0x00 MC 0x00 MC 0x00 MC 0x80
 #define MOV_EDX_0xFF800000  MC 0x66 MC 0xBA MC 0x00 MC 0x00 MC 0x80 MC 0xFF
 #define MOV_EDX_TO          MC 0x66 MC 0x89 MC 0x56 MC     
 #define MOVSX_AX_BY_ES_BX   MC 0x26 MC 0x0F MC 0xBE MC 0x07        
 #define MOVSX_EAX_BY_ES_BX  MC 0x66 MC 0x26 MC 0x0F MC 0xBE MC 0x07        
 #define MOVSX_EAX_WO_ES_BX  MC 0x66 MC 0x26 MC 0x0F MC 0xBF MC 0x07 
 #define MOVSX_ECX           MC 0x66 MC 0x0F MC 0xBF MC 0x4E MC
 #define MOVSX_EDX           MC 0x66 MC 0x0F MC 0xBF MC 0x56 MC
 #define NEG_EAX             MC 0x66 MC 0xE7 MC 0xD8   
 #define SAR_EDX             MC 0x66 MC 0xC1 MC 0xFA MC 
 #define SBB_EAX_0           MC 0x66 MC 0x1D MC 0x00 MC 0x00 MC 0x00 MC 0x00
 #define SBB_EDX             MC 0x66 MC 0x83 MC 0xDA MC 
 #define SHL_EAX             MC 0x66 MC 0xC1 MC 0xE0 MC 
 #define SHL_EBX             MC 0x66 MC 0xC1 MC 0xE3 MC 
 #define SHLD_EAX_EDX        MC 0x66 MC 0x0F MC 0xA4 MC 0xD0 MC      
 #define SHLD_EDX_EAX        MC 0x66 MC 0x0F MC 0xA4 MC 0xC2 MC      
 #define SHLD_EDX_ECX        MC 0x66 MC 0x0F MC 0xA4 MC 0xCA MC
 #define SHR_EAX             MC 0x66 MC 0xC1 MC 0xE8 MC  
 #define SHR_EDX             MC 0x66 MC 0xC1 MC 0xEA MC
 #define SUB_EAX_128         MC 0x66 MC 0x2D MC 0x80 MC 0xFF MC 0xFF MC 0xFF 
 #define SUB_EAX_32768       MC 0x66 MC 0x2D MC 0x00 MC 0x80 MC 0x00 MC 0x00
 #define SUB_EDX_0x00001000  MC 0x66 MC 0x81 MC 0xEA MC 0x00 MC 0x10 MC 0x00 MC 0x00
 #define SUB_EDX_0x00100000  MC 0x66 MC 0x81 MC 0xEA MC 0x00 MC 0x00 MC 0x10 MC 0x00
 #define SUB_EDX_0x20000000  MC 0x66 MC 0x81 MC 0xEA MC 0x00 MC 0x00 MC 0x00 MC 0x20
 #define SUB_EDX             MC 0x66 MC 0x83 MC 0xEA MC 
 #define XOR_EAX_0xFFFFFF80  MC 0x66 MC 0x35 MC 0x80 MC 0xFF MC 0xFF MC 0xFF
#endif 
#endif 
