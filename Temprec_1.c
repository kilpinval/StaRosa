Para lograr una precisión de ±0.3 °C en todo el rango (35°C a 80°C) con un PIC 16F887A, necesitas que tu sistema completo (sensor + circuito + ADC + software) sea capaz de detectar cambios de ~115 µV en el sensor.

Te explico las matemáticas y lo que necesitas para seleccionar los componentes adecuados:

1. El cálculo exacto que debes cumplir

· El PT100 varía 0.385 Ω por cada 1 °C.
· ±0.3 °C = 0.1155 Ω de cambio.
· Si usas una fuente de corriente de 1 mA (la recomendada para PT100), ese cambio genera:
    V = 0.001 A * 0.1155 Ω = 115.5 µV (0.1155 milivoltios).

Problema: El ADC del PIC es de 10 bits y con Vref=5V tiene un escalón de 4.88 mV (4880 µV). Eso es 42 veces más grande que lo que necesitas medir. Sin amplificación, solo podrías ver cambios de ~12.6°C.

---

2. ¿Qué ganancia (amplificación) necesitas?

Para que el PIC pueda "ver" esos 115 µV, debes amplificar la señal.

· Si usas una ganancia de 50:
    115 µV * 50 = 5.75 mV. Eso equivale a 1.18 escalones del ADC (muy justo, cualquier ruido te desvía ±0.3°C).
· Si usas una ganancia de 100:
    115 µV * 100 = 11.5 mV. Equivale a 2.35 escalones del ADC. Aquí ya estás más holgado.

Recomendación clave: Para ±0.3 °C con el ADC de 10 bits, necesitas una ganancia de al menos 100x en el acondicionador de señal (amplificador operacional). Con ganancia 100, la resolución teórica del sistema será de ~0.12 °C por escalón, y con los promedios y el filtro Kalman que ya vimos, lograrás la estabilidad necesaria.

---

3. El factor oculto: La resistencia de los cables (¡Cuidado!)

Con ±0.3 °C, estás midiendo 0.115 Ω. Si usas un PT100 de 2 hilos, solo 1 metro de cable fino (que tenga 0.5 Ω de resistencia) te introduce 1.3 °C de error fijo.

Para tu objetivo, es OBLIGATORIO:

· Usar un PT100 de 3 hilos (el estándar industrial) y un circuito que cancele la resistencia de los cables (puente de Wheatstone o fuente de corriente con detección a 4 hilos).
· O usar un PT100 de 4 hilos para máxima precisión.

---

4. Selección del circuito (Hardware)

Para cumplir ±0.3 °C sin volverte loco ajustando potenciómetros, te recomiendo:

Opción Complejidad Precisión alcanzable ¿Válida para ±0.3°C?
Fuente de 1mA + Amplificador diferencial (G=100) con OP07 (bajo offset) Media Sí, si calibrar manualmente con resistencias de precisión Sí (requiere calibración por software sí o sí)
Chip dedicado MAX31865 (hace todo: excitación, amplificación, conversión 15 bits y linealización) Baja (conexión SPI directa) ±0.1°C (típica) Sí (la opción más segura)
ADC externo ADS1115 (16 bits) + Amplificador G=50 Media ±0.1°C Sí (más fácil que el ampli del PIC)

---

5. Conclusión y decisión final

Para que tu sistema tenga ±0.3 °C de precisión real (no solo resolución), el PIC por sí solo no basta. Debes seleccionar una de estas rutas:

1. Ruta A (Económica y educativa):
      Construir el circuito con un OP07 (o LM358 mejorado), ganancia de 100x, usar el PT100 en configuración de 3 hilos, y aplicar en el PIC la calibración de offset/ganancia (la que te expliqué antes) + sobremuestreo. Con esto lograrás ±0.3 °C si usas resistencias de precisión al 0.1% en el amplificador.
2. Ruta B (Infalible y rápida):
      Conectar el PT100 directamente al MAX31865 y leer la temperatura por SPI. Este chip te entrega la temperatura con resolución de 0.03125 °C y una precisión típica de ±0.1°C, sin necesidad de amplificadores externos ni cálculos complejos de linealización. Es la opción que te recomiendo si quieres resultados profesionales sin frustraciones.

¿Qué prefieres? Si eliges la Ruta A, te ayudo a calcular los valores exactos de las resistencias del amplificador para que la salida de 0-5V cubra justo tu rango de 35°C a 80°C. Si eliges la Ruta B, te doy el código de ejemplo para leer el MAX31865 por SPI desde tu PIC.
