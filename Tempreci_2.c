Medir el rango de 35°C a 80°C con un PT100 y un PIC 16F887A es un proyecto clásico y totalmente factible. El verdadero desafío no es el rango, sino la resolución: el PT100 cambia solo ~0.39Ω por cada °C, lo que se traduce en unos ~4.5mV sin acondicionar. Como el ADC del PIC tiene una resolución de ~4.88mV por paso (con Vref=5V), no podrías distinguir ni siquiera 1°C sin un circuito de acondicionamiento.

Para lograrlo, necesitas un acondicionamiento de señal que amplifique esos pequeños cambios y un software que convierta la lectura del ADC en temperatura.

1. El Hardware: Acondicionamiento de Señal

El objetivo es convertir la pequeña variación de resistencia del PT100 en una señal de voltaje que el PIC pueda leer bien (0-5V).

· Circuito: La solución más equilibrada para tu PIC es usar el PT100 en un divisor de tensión con una resistencia de 1kΩ (que genera un voltaje de ~0.45V a 0.53V), y luego pasar esa señal por un amplificador diferencial con ganancia.
· Ajuste y Ganancia: Para medir de 35°C a 80°C, lo ideal es restar el voltaje base (el correspondiente a 35°C, ~1V) y luego amplificar la diferencia (con una ganancia de ~2). Así, la pequeña variación de ~4.5mV se convierte en una diferencia de ~9mV, aprovechando casi todo el rango del ADC.
· Fuente de Corriente: Para mayor precisión y estabilidad, lo mejor es usar una fuente de corriente constante de 1mA para excitar el PT100. Esto hace que la medición sea inmune a variaciones en el voltaje de alimentación.

2. El Software: Linealización

La relación entre resistencia y temperatura en un PT100 no es perfectamente lineal. Para obtener alta precisión, debes corregir esta no linealidad en el código del PIC.

Opción 1: Tabla de Consulta (Look-up Table) - Recomendada

Es el método más rápido y eficiente para un PIC, ya que evita complejas operaciones matemáticas en tiempo real.

1. Crea en tu código una tabla en la memoria PROGMEM (Flash) con pares de valores (ADC, Temperatura) para tu rango. Por ejemplo, cada 2°C o 5°C.
2. Al leer el ADC, buscas en la tabla los dos valores entre los que se encuentra tu lectura.
3. Interpolas linealmente para obtener la temperatura exacta.

Opción 2: Ecuación de Callendar-Van Dusen

Puedes usar la ecuación estándar para PT100, pero requiere operaciones en punto flotante que pueden ser lentas en un PIC de 8 bits.

· R(t) = R₀ · (1 + A·t + B·t²) para temperaturas > 0°C.
· Coeficientes estándar (IEC 751): A = 3.9083e-3, B = -5.775e-7.

3. Código de Ejemplo (Usando tabla en PROGMEM)

Este fragmento muestra cómo implementar la interpolación lineal con una tabla en la memoria Flash del PIC:

```c
#include <xc.h>

// Estructura para la tabla de conversión
typedef struct {
    int adc_val;    // Valor leído del ADC (escalado)
    int temp_val;   // Temperatura en °C * 10 (ej. 355 = 35.5°C)
} pt100_entry_t;

// Tabla en PROGMEM (ejemplo con valores hipotéticos de ADC)
const pt100_entry_t __attribute__((space(prog))) conversion_table[] = {
    { 100, 350 },  // ADC=100 -> 35.0°C
    { 150, 400 },  // ADC=150 -> 40.0°C
    { 200, 450 },  // ADC=200 -> 45.0°C
    // ... completar con más puntos hasta 80°C
};

int get_temperature(int adc_raw) {
    int i = 0;
    // Buscar el intervalo en la tabla
    while (adc_raw > conversion_table[i+1].adc_val) {
        i++;
    }

    // Interpolar linealmente
    int adc_diff = conversion_table[i+1].adc_val - conversion_table[i].adc_val;
    int temp_diff = conversion_table[i+1].temp_val - conversion_table[i].temp_val;
    int temp = conversion_table[i].temp_val + 
               ( (adc_raw - conversion_table[i].adc_val) * temp_diff ) / adc_diff;

    return temp; // Temperatura en °C * 10
}
```

4. Alternativa: Usar un Chip Especializado (MAX31865)

Si buscas simplificar al máximo el hardware y el software, esta es la mejor opción. El MAX31865 es un conversor que se conecta directamente al PT100 y te da la temperatura ya calculada a través de comunicación SPI, sin necesidad de circuitos de acondicionamiento ni complejas fórmulas de linealización.

5. Conexión y Conexión a Tierra

· PT100 de 2 hilos: El más sencillo, pero la resistencia de los cables puede introducir error.
· PT100 de 3 hilos: Es el estándar en entornos industriales porque compensa automáticamente la resistencia de los cables. Si tu sensor es de 3 hilos, te recomiendo usarlo para obtener mejores resultados.
· Para minimizar el ruido, usa un cable apantallado para el sensor y conéctalo a tierra en un solo punto.

¿Qué opción te parece mejor para tu proyecto? ¿Prefieres profundizar en el diseño del circuito amplificador con el operacional o en cómo implementar la tabla de linealización en el código?
