# <p align="center"> Instituto Tecnológico de Estudios Superiores de Occidente </p>
![Texto alternativo](https://oci02.img.iteso.mx/Identidades-De-Instancia/ITESO/Logos%20ITESO/Logo-ITESO-Vertical-SinFondo.png)
<p align="center"> Daniel Jiram Rodriguez Padilla <br>
703331 <br>
ie703331@iteso.mx<br>
Mauricio Martin del Campo <br>
734429 <br>
mauricio.martindelc@iteso.mx <br>
Aplicación basada en un planificador Rate Monotonic <br>
Práctica 1 <br>
Equipo 2 <br>
Sistemas Operativos en Tiempo Real <br>
Axel Ramírez Herrera <br>
20 de mayo de 2025 </p>

##  Diagrama Estático
![Texto alternativo](https://i.imgur.com/OKl7DpL.png)

##  Diagrama Dinámico
![Texto alternativo]([https://i.imgur.com/TtVdZ3V.png)

## Medición de la Carga del CPU
Los callbacks implementados en este proyecto tienen como objetivo central gestionar el tiempo de manera periódica y no bloqueante, permitiendo un flujo constante en la lógica del juego. Para ello, se aprovecha el módulo PIT (Periodic Interrupt Timer) de la K64, configurando canales específicos para controlar eventos de tiempo como el avance de frames y la reproducción de sonidos. <br>
Los callbacks permiten separar la lógica temporal del resto del código, reduciendo el uso de ciclos bloqueantes (`delay`) y permitiendo que el juego responda eficientemente a eventos como disparos, aciertos o tiempo agotado. <br>
En esta práctica, los canales se utilizan de la siguiente manera: <br>
- **PIT 0:** Se emplea como temporizador principal del juego. Su callback está vinculado al sistema de animación y lógica del juego (por ejemplo, avance de frames de los patos). Permite ejecutar funciones a una frecuencia constante, como por ejemplo a 60 Hz para actualizar el estado del juego.
- **PIT 2:** Se utiliza exclusivamente como temporizador de retardo no bloqueante. Al iniciar un "delay" mediante `pit_delay_start()`, este canal comienza a contar y, una vez cumplido el tiempo, su callback activa una bandera (`delay_done`) que puede ser consultada por el sistema para continuar la ejecución.

## Decisiones de Diseño
**Daniel:** Esta práctica representó un reto importante en cuanto al diseño de sistemas interactivos en tiempo real. A diferencia de ejercicios anteriores más centrados en tareas secuenciales, aquí fue necesario integrar múltiples componentes asincrónicos como el botón con interrupciones, el ADC con lógica de disparo, el PIT para temporización no bloqueante, y la UART como salida visual principal. Me pareció especialmente interesante cómo la estructura modular permitió escalar el sistema sin generar dependencias cruzadas, y cómo el uso de flags y estados bien definidos facilitó el flujo de juego. Esta práctica también me ayudó a entender mejor la relación entre precisión temporal y experiencia de usuario en interfaces basadas solo en texto.<br>
**Mauricio:**  Esta ha sido una de las prácticas más creativas y completas hasta ahora. Implicó un enfoque diferente, más enfocado en experiencia de usuario y lógica de juego, pero con una fuerte base técnica detrás. Desde configurar correctamente el ADC para detectar luz en el momento exacto, hasta implementar un sistema de animación y control de flujo por PIT, cada parte del sistema requería trabajar en armonía. Algo que me gustó mucho fue poder diseñar una mecánica completa desde cero usando solo UART como interfaz, y ver cómo decisiones pequeñas —como el tiempo entre frames o cuándo limpiar una bandera— afectaban directamente la jugabilidad. Sin duda, esta práctica me dejó más confianza para implementar sistemas interactivos basados en eventos y ciclos no bloqueantes.
