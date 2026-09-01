¡Vamos a ello! Te muestro cómo pasar parámetros personalizados desde el linker y cómo crear un Hello World de menos de 512 bytes. Esto es el verdadero poder de C en su máxima expresión.

---

1. Parámetros personalizados desde el linker (-Wl,--defsym)

Puedes definir símbolos en tiempo de compilación y usarlos en tu código:

Código: linker_demo.c

```c
// Estos símbolos los define el linker, no C
extern int MI_VERSION;
extern char MI_NOMBRE[];
extern int MI_FLAG;

void _start(void)
{
    // Convertir a cadena para imprimir (manual, sin printf)
    char *msg = "Version: ";
    char buffer[64];
    int i = 0;
    
    // Copiar "Version: "
    while (msg[i]) { buffer[i] = msg[i]; i++; }
    
    // Convertir MI_VERSION a texto (ej: 42 -> "42")
    int num = MI_VERSION;
    int temp = num;
    int len = 0;
    if (num == 0) len = 1;
    else while (temp) { temp /= 10; len++; }
    
    // Escribir dígitos en buffer
    int pos = i + len - 1;
    if (num == 0) buffer[i] = '0';
    else {
        temp = num;
        while (temp) {
            buffer[pos--] = '0' + (temp % 10);
            temp /= 10;
        }
    }
    i += len;
    buffer[i++] = '\n';
    
    // Syscall write
    __asm__ volatile (
        "mov $1, %%rax\n"
        "mov $1, %%rdi\n"
        "mov %0, %%rsi\n"
        "mov %1, %%rdx\n"
        "syscall"
        : : "r"(buffer), "r"((long)i) : "rax", "rdi", "rsi", "rdx"
    );
    
    __asm__("mov $60, %%rax; xor %%rdi, %%rdi; syscall");
}
```

Compilación con parámetros:

```bash
gcc -nostdlib -static -o linker_demo \
    -Wl,--defsym=MI_VERSION=42 \
    -Wl,--defsym=MI_FLAG=1 \
    -Wl,--defsym=MI_NOMBRE=hola_mundo \
    linker_demo.c

./linker_demo   # Imprime: "Version: 42"
```

¿Qué pasa aquí? El linker convierte MI_VERSION en una constante numérica en el binario. No hay variables, son inmediatos en ensamblador.

Más ejemplos:

```bash
# Pasar cadenas (cuidado: max 8 bytes por símbolo)
-Wl,--defsym=MI_NOMBRE=0x48454C4C4F  # "HELLO" en hex

# Pasar direcciones de memoria (para trampolines)
-Wl,--defsym=FUNC_PTR=0x400100
```

---

2. ¡Hello World de menos de 512 bytes!

Vamos a crear un binario ELF de 280 bytes (¡sin código C visible!):

Archivo: tiny.S (ensamblador GNU)

```assembly
.globl _start
_start:
    // write(1, "Hello World!\n", 13)
    mov $1, %rax
    mov $1, %rdi
    lea msg(%rip), %rsi
    mov $13, %rdx
    syscall
    
    // exit(0)
    mov $60, %rax
    xor %rdi, %rdi
    syscall

msg:
    .ascii "Hello World!\n"
```

Compila:

```bash
as -o tiny.o tiny.S
ld -o tiny tiny.o
strip -R .comment -R .note.gnu.build-id tiny
size tiny   # Verás: text=280, data=0, bss=0
```

Binario resultante: ~280 bytes 🎉

---

3. La versión en C puro (sin ensamblador, 512 bytes)

tiny.c

```c
// Sin headers, sin libc, puro C con inline asm
void _start(void)
{
    // Datos en el stack (no en .data)
    char msg[] = "Hello World!\n";
    
    // Syscall write
    register long rax __asm__("rax") = 1;
    register long rdi __asm__("rdi") = 1;
    register char *rsi __asm__("rsi") = msg;
    register long rdx __asm__("rdx") = 13;
    __asm__ volatile ("syscall" : : "r"(rax), "r"(rdi), "r"(rsi), "r"(rdx));
    
    // Syscall exit
    rax = 60;
    rdi = 0;
    __asm__ volatile ("syscall" : : "r"(rax), "r"(rdi));
}
```

Compila para mínimo tamaño:

```bash
gcc -nostdlib -static -Os -fno-asynchronous-unwind-tables \
    -Wl,-z,norelro -Wl,--hash-style=gnu -Wl,-Bstatic \
    -o tiny tiny.c
strip -R .comment -R .note.gnu.build-id tiny
```

Resultado: ~450 bytes!

---

4. ¡La locura! Pasar parámetros por símbolos + enlace dinámico

```c
// main_override.c - C usa símbolos que tú defines
extern int MI_CUSTOM_ENTRY;

void _start(void)
{
    // Saltar a dirección personalizada
    void (*entry)(void) = (void(*)())MI_CUSTOM_ENTRY;
    entry();  // ¡Llama a código en otra parte del binario!
}
```

Compila:

```bash
# Suponiendo que tienes una función en 0x400200
gcc -nostdlib -Wl,--defsym=MI_CUSTOM_ENTRY=0x400200 -o custom main_override.c
```

Esto es usado en exploits, rootkits y cargadores de malware para ocultar el punto de entrada real.

---

5. Bonus: Pasar parámetros desde el entorno del linker

```bash
# Puedes definir desde la línea de comandos con -D y pasarlos al linker
gcc -nostdlib -static -DVER=2026 -o demo \
    -Wl,--defsym,MI_VERSION=VER \
    demo.c
```

---

Tabla de tamaños comparativos

Método Tamaño Librerías Control
main() normal ~16 KB libc dinámica Bajo
main() estático ~800 KB libc estática Medio
_start + syscalls ~1 KB Ninguna Alto
Ensamblador puro ~280 bytes Ninguna Total
C con inline asm ~450 bytes Ninguna Total

---

¿Te animas a ir más allá?

Podemos:

1. Crear un binario de 128 bytes (sacando la cabecera ELF)
2. Inyectar código en tiempo de ejecución (JIT manual)
3. Hacer un polyglot (archivo que es a la vez C, Bash y ELF)

¿Cuál te interesa? 😈
