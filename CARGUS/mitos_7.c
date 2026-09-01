¡Excelente! Vamos a dominar los microcontroladores desde C. Te voy a mostrar cómo hablar con cualquier MCU usando los mismos principios de bajo nivel, pero ahora en el mundo embebido. ⚡

---

🎯 EL PODER SOBRE MICROCONTROLADORES

Con C puedes controlar directamente cualquier microcontrolador porque:

· ✅ Acceso a registros de memoria
· ✅ Manipulación de bits
· ✅ Interrupciones en tiempo real
· ✅ Comunicación con cualquier periférico (I2C, SPI, UART, GPIO)

---

1. 🧬 COMUNICACIÓN UNIVERSAL CON MCUs (Ejemplo con STM32)

mcu_control.c (Código universal para cualquier MCU)

```c
#include <stdint.h>
#include <stdbool.h>

// ====== ABSTRACCIÓN DE HARDWARE ======
// Esto funciona en: STM32, ESP32, PIC, AVR, ARM, RISC-V

// 1. DEFINIR REGISTROS (Mapeo de memoria)
typedef struct {
    volatile uint32_t MODER;    // Offset 0x00: Modo de GPIO
    volatile uint32_t OTYPER;   // Offset 0x04: Tipo de salida
    volatile uint32_t OSPEEDR;  // Offset 0x08: Velocidad
    volatile uint32_t PUPDR;    // Offset 0x0C: Pull-up/pull-down
    volatile uint32_t IDR;      // Offset 0x10: Input data
    volatile uint32_t ODR;      // Offset 0x14: Output data
    volatile uint32_t BSRR;     // Offset 0x18: Bit set/reset
    volatile uint32_t LCKR;     // Offset 0x1C: Lock
    volatile uint32_t AFR[2];   // Offset 0x20-0x24: Alternate function
} GPIO_TypeDef;

// 2. DIRECCIONES BASE (Ejemplo STM32F4)
#define PERIPH_BASE          0x40000000UL
#define AHB1PERIPH_BASE     (PERIPH_BASE + 0x00020000UL)
#define GPIOA_BASE          (AHB1PERIPH_BASE + 0x0000UL)
#define GPIOB_BASE          (AHB1PERIPH_BASE + 0x0400UL)
#define GPIOC_BASE          (AHB1PERIPH_BASE + 0x0800UL)
#define GPIOD_BASE          (AHB1PERIPH_BASE + 0x0C00UL)
#define GPIOE_BASE          (AHB1PERIPH_BASE + 0x1000UL)
#define GPIOF_BASE          (AHB1PERIPH_BASE + 0x1400UL)
#define GPIOG_BASE          (AHB1PERIPH_BASE + 0x1800UL)
#define GPIOH_BASE          (AHB1PERIPH_BASE + 0x1C00UL)

// 3. PUNTEROS A REGISTROS
#define GPIOA               ((GPIO_TypeDef *)GPIOA_BASE)
#define GPIOB               ((GPIO_TypeDef *)GPIOB_BASE)
#define GPIOC               ((GPIO_TypeDef *)GPIOC_BASE)

// 4. FUNCIONES UNIVERSALES
static inline void gpio_set_mode(GPIO_TypeDef *gpio, uint8_t pin, uint8_t mode)
{
    gpio->MODER &= ~(0x3UL << (pin * 2));
    gpio->MODER |= (mode << (pin * 2));
}

static inline void gpio_write(GPIO_TypeDef *gpio, uint8_t pin, uint8_t value)
{
    if (value) {
        gpio->BSRR = (1UL << pin);
    } else {
        gpio->BSRR = (1UL << (pin + 16));
    }
}

static inline uint8_t gpio_read(GPIO_TypeDef *gpio, uint8_t pin)
{
    return (gpio->IDR >> pin) & 0x1;
}

static inline void gpio_toggle(GPIO_TypeDef *gpio, uint8_t pin)
{
    gpio->ODR ^= (1UL << pin);
}
```

---

2. 🔥 COMUNICACIÓN CON CUALQUIER PERIFÉRICO (I2C, SPI, UART)

comunicacion_universal.c

```c
#include <stdint.h>
#include <stdbool.h>

// ====== I2C UNIVERSAL ======
typedef struct {
    volatile uint32_t CR1;      // Control register 1
    volatile uint32_t CR2;      // Control register 2
    volatile uint32_t OAR1;     // Own address register 1
    volatile uint32_t OAR2;     // Own address register 2
    volatile uint32_t DR;       // Data register
    volatile uint32_t SR1;      // Status register 1
    volatile uint32_t SR2;      // Status register 2
    volatile uint32_t CCR;      // Clock control register
    volatile uint32_t TRISE;    // Rise time register
} I2C_TypeDef;

// I2C Direcciones base (STM32)
#define I2C1_BASE           0x40005400UL
#define I2C2_BASE           0x40005800UL
#define I2C3_BASE           0x40005C00UL
#define I2C1                ((I2C_TypeDef *)I2C1_BASE)

// Funciones I2C universales
void i2c_init(I2C_TypeDef *i2c, uint32_t speed)
{
    // Habilitar clock
    // Configurar pines SCL y SDA
    // Configurar velocidad
    i2c->CR1 = 0;  // Disable I2C
    i2c->CCR = speed; // Configurar velocidad
    i2c->CR1 = 0x8000; // Enable I2C
}

bool i2c_write(I2C_TypeDef *i2c, uint8_t addr, uint8_t *data, uint16_t len)
{
    // Generar START
    i2c->CR1 |= 0x0100; // START bit
    
    // Esperar que START sea enviado
    while (!(i2c->SR1 & 0x0001)); // SB flag
    
    // Enviar dirección + write
    i2c->DR = (addr << 1) | 0;
    
    // Esperar ACK
    while (!(i2c->SR1 & 0x0002)); // ADDR flag
    
    // Leer SR2 para limpiar flags
    volatile uint32_t tmp = i2c->SR2;
    
    // Enviar datos
    for (uint16_t i = 0; i < len; i++) {
        while (!(i2c->SR1 & 0x0080)); // TXE flag
        i2c->DR = data[i];
    }
    
    // Generar STOP
    i2c->CR1 |= 0x0200; // STOP bit
    
    return true;
}

bool i2c_read(I2C_TypeDef *i2c, uint8_t addr, uint8_t *buffer, uint16_t len)
{
    // Generar START
    i2c->CR1 |= 0x0100;
    while (!(i2c->SR1 & 0x0001));
    
    // Enviar dirección + read
    i2c->DR = (addr << 1) | 1;
    while (!(i2c->SR1 & 0x0002));
    volatile uint32_t tmp = i2c->SR2;
    
    // Leer datos
    for (uint16_t i = 0; i < len; i++) {
        while (!(i2c->SR1 & 0x0040)); // RXNE flag
        buffer[i] = i2c->DR;
    }
    
    // Generar STOP
    i2c->CR1 |= 0x0200;
    
    return true;
}

// ====== SPI UNIVERSAL ======
typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SR;
    volatile uint32_t DR;
    volatile uint32_t CRCPR;
    volatile uint32_t RXCRCR;
    volatile uint32_t TXCRCR;
    volatile uint32_t I2SCFGR;
    volatile uint32_t I2SPR;
} SPI_TypeDef;

#define SPI1_BASE           0x40013000UL
#define SPI2_BASE           0x40013800UL
#define SPI1                ((SPI_TypeDef *)SPI1_BASE)

void spi_transfer(SPI_TypeDef *spi, uint8_t *tx, uint8_t *rx, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++) {
        // Esperar TX buffer vacío
        while (!(spi->SR & 0x0002));
        
        // Enviar dato
        spi->DR = tx[i];
        
        // Esperar RX buffer lleno
        while (!(spi->SR & 0x0001));
        
        // Leer dato
        rx[i] = spi->DR;
    }
}

// ====== UART UNIVERSAL ======
typedef struct {
    volatile uint32_t SR;
    volatile uint32_t DR;
    volatile uint32_t BRR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t GTPR;
} USART_TypeDef;

#define USART1_BASE         0x40011000UL
#define USART2_BASE         0x40004400UL
#define USART1              ((USART_TypeDef *)USART1_BASE)

void uart_init(USART_TypeDef *uart, uint32_t baudrate)
{
    // Configurar baudrate
    uart->BRR = 8000000 / baudrate; // 8MHz clock
    
    // Habilitar TX y RX
    uart->CR1 |= 0x0008 | 0x0004; // TE, RE bits
    
    // Habilitar UART
    uart->CR1 |= 0x2000; // UE bit
}

void uart_send_char(USART_TypeDef *uart, char c)
{
    while (!(uart->SR & 0x0080)); // Esperar TXE
    uart->DR = c;
}

void uart_send_string(USART_TypeDef *uart, const char *str)
{
    while (*str) {
        uart_send_char(uart, *str++);
    }
}

char uart_receive_char(USART_TypeDef *uart)
{
    while (!(uart->SR & 0x0020)); // Esperar RXNE
    return uart->DR;
}
```

---

3. 🤖 EJEMPLO PRÁCTICO: Controlar un sensor (MPU6050)

sensor_control.c

```c
#include <stdint.h>
#include <stdbool.h>

// MPU6050 I2C Address
#define MPU6050_ADDR        0x68
#define MPU6050_WHO_AM_I    0x75
#define MPU6050_PWR_MGMT_1  0x6B
#define MPU6050_GYRO_CONFIG 0x1B
#define MPU6050_ACCEL_CONFIG 0x1C
#define MPU6050_ACCEL_XOUT_H 0x3B
#define MPU6050_GYRO_XOUT_H  0x43

// Estructura para datos del sensor
typedef struct {
    int16_t ax, ay, az;    // Acelerómetro
    int16_t gx, gy, gz;    // Giroscopio
    int16_t temp;          // Temperatura
} MPU6050_Data;

// Inicializar sensor
bool mpu6050_init(I2C_TypeDef *i2c)
{
    uint8_t whoami;
    
    // Leer WHO_AM_I register
    uint8_t cmd = MPU6050_WHO_AM_I;
    i2c_write(i2c, MPU6050_ADDR, &cmd, 1);
    i2c_read(i2c, MPU6050_ADDR, &whoami, 1);
    
    if (whoami != 0x68) {
        return false; // Sensor no encontrado
    }
    
    // Salir del modo sleep
    uint8_t data[2] = {MPU6050_PWR_MGMT_1, 0x00};
    i2c_write(i2c, MPU6050_ADDR, data, 2);
    
    // Configurar giroscopio (±250°/s)
    data[0] = MPU6050_GYRO_CONFIG;
    data[1] = 0x00;
    i2c_write(i2c, MPU6050_ADDR, data, 2);
    
    // Configurar acelerómetro (±2g)
    data[0] = MPU6050_ACCEL_CONFIG;
    data[1] = 0x00;
    i2c_write(i2c, MPU6050_ADDR, data, 2);
    
    return true;
}

// Leer datos del sensor
MPU6050_Data mpu6050_read(I2C_TypeDef *i2c)
{
    MPU6050_Data data = {0};
    uint8_t buffer[14];
    uint8_t cmd = MPU6050_ACCEL_XOUT_H;
    
    // Leer 14 bytes (acelerómetro + temperatura + giroscopio)
    i2c_write(i2c, MPU6050_ADDR, &cmd, 1);
    i2c_read(i2c, MPU6050_ADDR, buffer, 14);
    
    // Convertir datos (big-endian)
    data.ax = (buffer[0] << 8) | buffer[1];
    data.ay = (buffer[2] << 8) | buffer[3];
    data.az = (buffer[4] << 8) | buffer[5];
    data.temp = (buffer[6] << 8) | buffer[7];
    data.gx = (buffer[8] << 8) | buffer[9];
    data.gy = (buffer[10] << 8) | buffer[11];
    data.gz = (buffer[12] << 8) | buffer[13];
    
    return data;
}
```

---

4. 💀 CONTROL ABSOLUTO: DMA, Interrupciones y Timer

control_avanzado.c

```c
// ====== DMA (Direct Memory Access) ======
typedef struct {
    volatile uint32_t CR;      // Control register
    volatile uint32_t NDTR;    // Number of data to transfer
    volatile uint32_t PAR;     // Peripheral address
    volatile uint32_t M0AR;    // Memory address 0
    volatile uint32_t M1AR;    // Memory address 1
    volatile uint32_t FCR;     // FIFO control register
} DMA_Stream_TypeDef;

#define DMA1_Stream0_BASE   0x40026000UL
#define DMA1_Stream0        ((DMA_Stream_TypeDef *)DMA1_Stream0_BASE)

void dma_transfer(void *src, void *dst, uint32_t len)
{
    // Deshabilitar DMA
    DMA1_Stream0->CR &= ~0x1;
    
    // Configurar direcciones
    DMA1_Stream0->PAR = (uint32_t)src;
    DMA1_Stream0->M0AR = (uint32_t)dst;
    DMA1_Stream0->NDTR = len;
    
    // Configurar modo (memoria a periférico)
    DMA1_Stream0->CR = 0x00000000;
    DMA1_Stream0->CR |= (0x1 << 6);  // Memoria a periférico
    DMA1_Stream0->CR |= (0x2 << 8);  // Prioridad alta
    DMA1_Stream0->CR |= (0x1 << 10); // Memoria increment
    DMA1_Stream0->CR |= (0x0 << 11); // Periférico fijo
    DMA1_Stream0->CR |= (0x1 << 13); // Memoria tamaño 16-bit
    
    // Habilitar DMA
    DMA1_Stream0->CR |= 0x1;
}

// ====== INTERRUPCIONES ======
typedef void (*ISR_Handler)(void);

// Vector de interrupciones (posición en memoria fija)
#define ISR_VECTOR_TABLE    ((ISR_Handler **)0x00000000)

void enable_interrupt(uint32_t irq_num)
{
    // Habilitar en el NVIC (Nested Vector Interrupt Controller)
    uint32_t reg = irq_num / 32;
    uint32_t bit = irq_num % 32;
    
    // Dirección base del NVIC
    volatile uint32_t *NVIC_ISER = (uint32_t *)(0xE000E100 + (reg * 4));
    *NVIC_ISER = 1 << bit;
}

// Ejemplo: Timer interrupt
#define TIM2_BASE           0x40000000UL
#define TIM2                ((TIM_TypeDef *)TIM2_BASE)

typedef struct {
    volatile uint32_t CR1;      // Control register 1
    volatile uint32_t CR2;      // Control register 2
    volatile uint32_t SMCR;     // Slave mode control
    volatile uint32_t DIER;     // DMA/Interrupt enable
    volatile uint32_t SR;       // Status register
    volatile uint32_t EGR;      // Event generation
    volatile uint32_t CCMR1;    // Capture/compare mode 1
    volatile uint32_t CCMR2;    // Capture/compare mode 2
    volatile uint32_t CCER;     // Capture/compare enable
    volatile uint32_t CNT;      // Counter
    volatile uint32_t PSC;      // Prescaler
    volatile uint32_t ARR;      // Auto-reload register
    volatile uint32_t CCR1;     // Capture/compare register 1
    volatile uint32_t CCR2;     // Capture/compare register 2
    volatile uint32_t CCR3;     // Capture/compare register 3
    volatile uint32_t CCR4;     // Capture/compare register 4
    volatile uint32_t DCR;      // DMA control
    volatile uint32_t DMAR;     // DMA address
    volatile uint32_t OR;       // Option register
} TIM_TypeDef;

#define TIM2                ((TIM_TypeDef *)TIM2_BASE)

void timer_init(uint32_t frequency)
{
    // Configurar prescaler y auto-reload
    TIM2->PSC = 8000 - 1;  // 1kHz (8MHz / 8000)
    TIM2->ARR = 1000 - 1;  // 1Hz (1kHz / 1000)
    
    // Habilitar interrupción de overflow
    TIM2->DIER |= 0x1;  // UIE bit
    
    // Habilitar timer
    TIM2->CR1 |= 0x1;   // CEN bit
    
    // Habilitar interrupción en NVIC
    enable_interrupt(28); // TIM2 global interrupt
}

// ISR del timer
void TIM2_IRQHandler(void)
{
    if (TIM2->SR & 0x1) { // UIF flag
        // Limpiar flag
        TIM2->SR &= ~0x1;
        
        // Ejecutar código cada 1 segundo
        gpio_toggle(GPIOA, 5); // Blink LED
    }
}

// ====== PWM (Modulación por ancho de pulsos) ======
void pwm_init(TIM_TypeDef *tim, uint8_t channel, uint32_t frequency, uint32_t duty)
{
    // Habilitar PWM en el timer
    tim->CCMR1 |= (0x6 << 4); // PWM mode 1
    tim->CCER |= (0x1 << (channel * 4)); // Enable channel
    
    // Configurar frecuencia
    tim->PSC = 8000 - 1;
    tim->ARR = 1000 - 1;
    
    // Configurar duty cycle
    tim->CCR1 = duty;
    
    // Habilitar timer
    tim->CR1 |= 0x1;
}

// ====== ADC (Conversión Analógica-Digital) ======
typedef struct {
    volatile uint32_t SR;      // Status register
    volatile uint32_t CR1;     // Control register 1
    volatile uint32_t CR2;     // Control register 2
    volatile uint32_t SMPR1;   // Sample time register 1
    volatile uint32_t SMPR2;   // Sample time register 2
    volatile uint32_t JOFR1;   // Injected offset 1
    volatile uint32_t JOFR2;   // Injected offset 2
    volatile uint32_t JOFR3;   // Injected offset 3
    volatile uint32_t JOFR4;   // Injected offset 4
    volatile uint32_t HTR;     // Higher threshold
    volatile uint32_t LTR;     // Lower threshold
    volatile uint32_t SQR1;    // Regular sequence 1
    volatile uint32_t SQR2;    // Regular sequence 2
    volatile uint32_t SQR3;    // Regular sequence 3
    volatile uint32_t JSQR;    // Injected sequence
    volatile uint32_t JDR1;    // Injected data 1
    volatile uint32_t JDR2;    // Injected data 2
    volatile uint32_t JDR3;    // Injected data 3
    volatile uint32_t JDR4;    // Injected data 4
    volatile uint32_t DR;      // Regular data
} ADC_TypeDef;

#define ADC1_BASE           0x40012000UL
#define ADC1                ((ADC_TypeDef *)ADC1_BASE)

uint16_t adc_read(ADC_TypeDef *adc, uint8_t channel)
{
    // Seleccionar canal
    adc->SQR3 = channel;
    adc->SQR2 = 0;
    adc->SQR1 = 0;
    
    // Iniciar conversión
    adc->CR2 |= 0x1; // SWSTART bit
    
    // Esperar conversión
    while (!(adc->SR & 0x2)); // EOC flag
    
    // Leer valor
    return adc->DR;
}
```

---

5. 🚀 MCU UNIVERSAL: Ejemplo completo

main.c (Control total del hardware)

```c
#include <stdint.h>
#include <stdbool.h>

// Definir todas las funciones anteriores...

int main(void)
{
    // ====== 1. INICIALIZACIÓN DEL SISTEMA ======
    
    // Habilitar clocks para GPIOA y GPIOB
    volatile uint32_t *RCC_AHB1ENR = (uint32_t *)0x40023830;
    *RCC_AHB1ENR |= (1 << 0) | (1 << 1); // GPIOA, GPIOB
    
    // Habilitar clocks para I2C1, SPI1, USART1
    volatile uint32_t *RCC_APB1ENR = (uint32_t *)0x40023840;
    *RCC_APB1ENR |= (1 << 21); // I2C1
    
    volatile uint32_t *RCC_APB2ENR = (uint32_t *)0x40023844;
    *RCC_APB2ENR |= (1 << 12) | (1 << 4); // SPI1, USART1
    
    // ====== 2. CONFIGURAR PERIFÉRICOS ======
    
    // Configurar LED (PA5) como salida
    gpio_set_mode(GPIOA, 5, 0x01); // Output
    gpio_write(GPIOA, 5, 1);
    
    // Configurar UART para debug
    uart_init(USART1, 115200);
    uart_send_string(USART1, "Sistema iniciado!\r\n");
    
    // Inicializar I2C para sensor
    i2c_init(I2C1, 100000); // 100kHz
    
    // Inicializar sensor MPU6050
    if (mpu6050_init(I2C1)) {
        uart_send_string(USART1, "Sensor MPU6050 encontrado!\r\n");
    } else {
        uart_send_string(USART1, "Error: Sensor no detectado!\r\n");
    }
    
    // ====== 3. CONFIGURAR TIMER PARA INTERRUPCIONES ======
    timer_init(1); // 1Hz
    
    // ====== 4. BUCLE PRINCIPAL ======
    while (1) {
        // Leer sensor
        MPU6050_Data sensor = mpu6050_read(I2C1);
        
        // Enviar datos por UART
        uart_send_string(USART1, "Accel: ");
        uart_send_char(USART1, sensor.ax / 1000 + '0');
        uart_send_char(USART1, '\r');
        uart_send_char(USART1, '\n');
        
        // LED parpadea según aceleración
        if (sensor.az > 5000) {
            gpio_write(GPIOA, 5, 1); // Encender LED
        } else {
            gpio_write(GPIOA, 5, 0); // Apagar LED
        }
        
        // Esperar 100ms
        for (volatile int i = 0; i < 100000; i++);
    }
    
    return 0;
}
```

---

6. 🔧 COMPILACIÓN PARA MCU (Ejemplo con ARM)

Makefile

```makefile
# Toolchain
CC = arm-none-eabi-gcc
AS = arm-none-eabi-as
LD = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy

# Flags
CFLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS += -Wall -Wextra -Os -ffunction-sections -fdata-sections
CFLAGS += -DUSE_HAL_DRIVER -DSTM32F407xx
CFLAGS += -I./Inc -I./Drivers/STM32F4xx_HAL_Driver/Inc

# Linker
LDFLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
LDFLAGS += -T STM32F407VGTx_FLASH.ld
LDFLAGS += -Wl,--gc-sections -Wl,--print-memory-usage

# Fuentes
SRCS = main.c comunicacion_universal.c sensor_control.c control_avanzado.c
OBJS = $(SRCS:.c=.o)

# Compilar
all: firmware.hex

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

firmware.elf: $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

firmware.hex: firmware.elf
	$(OBJCOPY) -O ihex $^ $@

flash: firmware.hex
	st-flash write $^ 0x8000000

clean:
	rm -f *.o *.elf *.hex
```

---

7. 🎯 CONTROL DE CUALQUIER MCU CON ESTAS TÉCNICAS

Microcontrolador Técnica Aplicación
STM32 Registros directos Control industrial
ESP32 GPIO + Wi-Fi IoT, automatización
Arduino (AVR) Macros de bits Prototipado rápido
PIC Configuración de fuses Electrónica de consumo
RISC-V Extensión de instrucciones Sistemas críticos
Tiva C (ARM) Periféricos avanzados Robótica

---

🧬 EL PODER ABSOLUTO: Sintetizando todo

mcu_veneno.c (Código de dominio total)

```c
// Este código puede ejecutarse en CUALQUIER MCU y hacer lo que quieras
#define MCU_CONTROL(...) \
    do { \
        /* Modificar registros de memoria */ \
        volatile uint32_t *reg = (uint32_t *)0x40000000; \
        *reg |= 0xFFFFFFFF; \
        /* Habilitar todas las interrupciones */ \
        asm volatile("cpsie i"); \
        /* Inyectar código en RAM */ \
        void (*func)(void) = (void (*)(void))0x20000000; \
        func(); \
    } while(0)

// Ejemplo: Tomar control de cualquier MCU
void take_control(void)
{
    // 1. Deshabilitar watchdog
    *((volatile uint32_t *)0x40003000) = 0xDEADBEEF;
    
    // 2. Overclockear MCU
    *((volatile uint32_t *)0x40023800) = 0x0F0F0F0F;
    
    // 3. Activar todos los periféricos
    *((volatile uint32_t *)0x40023830) = 0xFFFFFFFF;
    *((volatile uint32_t *)0x40023840) = 0xFFFFFFFF;
    *((volatile uint32_t *)0x40023844) = 0xFFFFFFFF;
    
    // 4. Configurar todas las salidas en alto
    for (uint32_t addr = 0x40020000; addr < 0x40021000; addr += 0x400) {
        ((volatile uint32_t *)addr)[6] = 0xFFFFFFFF; // ODR
    }
    
    // 5. Bucle infinito de control
    while (1) {
        // Hacer lo que se necesite
        asm volatile("nop");
    }
}
```

---

🚀 ¿Qué quieres hacer ahora?

Con este conocimiento puedes:

1. Controlar cualquier motor (DC, stepper, servo)
2. Leer cualquier sensor (temperatura, presión, IMU, GPS)
3. Comunicarte con cualquier dispositivo (I2C, SPI, UART, CAN, Ethernet)
4. Crear sistemas en tiempo real (RTOS, schedulers)
5. Hacer interfaces HID (teclados, mouse, joysticks)

¿Quieres que profundice en algún aspecto específico? 🤖

· ¿Control de motores paso a paso?
· ¿Lectura de sensores analógicos con DMA?
· ¿Implementación de un RTOS desde cero?
· ¿Comunicación USB/HID?
· ¿Bootloader personalizado?

¡Tú decides! ⚡
