# SimulacionFisicaVideojuegos

## 1. Resumen

“Tiro al Pato 2.0” es un minijuego donde el jugador dispara proyectiles para derribar patos (sólidos rígidos) en una escena con viento y agua. El proyecto demuestra un sistema de partículas con emisores, fuerzas físicas (gravedad y viento), gestión de vida/limpieza y un sistema de rígidos con parámetros variables.

## 2. Temática y objetivo del juego

El objetivo es **maximizar la puntuación** derribando patos antes de que desaparezcan. La dificultad viene del movimiento de los patos y la influencia del viento.

## 3. Arquitectura y diagrama de clases

Incluye un diagrama con estos bloques (mínimo):

* **Partículas**: `Particle`, `ProjectileManager`, `ParticleSystem`, `IEmitter` (`PointEmitter`, `BoxEmitter`), `ForceGenerator` (`GravityForce`, `WindForce`).
* **Rígidos**: `DuckManager` → `Duck` (con `PxRigidDynamic* body` + `RenderItem* renderItem`), `GameManager`.

Si quieres hacerlo rápido, pega esto en diagrams.net como guía (o PlantUML si usas):

* `ParticleSystem` **contiene** `emitters`, `particles`, `forces`.
* `ForceGenerator` es interfaz -> `GravityForce`, `WindForce`.
* `IEmitter` es interfaz -> `PointEmitter`, `BoxEmitter`.
* `DuckManager` contiene lista de `Duck`.

## 4. Modelos físicos y ecuaciones usadas

### 4.1 Partículas (integración)

Integración semi-implícita:

* ( a = F/m + g )
* ( v_{t+dt} = v_t + a,dt )
* ( x_{t+dt} = x_t + v_{t+dt},dt )
  Con **damping independiente del frame**: ( v \leftarrow v \cdot damping^{dt} )

### 4.2 Fuerza de gravedad (ForceGenerator)

* **Gravedad**: ( \mathbf{F}_g = m \mathbf{g} )

Parámetro usado:

* ( \mathbf{g} = (0, -9.8, 0) ) (ajustable según la escala del juego).

### 4.3 Fuerza de viento (ForceGenerator)

Viento tipo drag/turbulencia:

* ( \mathbf{v}*{rel} = \mathbf{v}*{wind} - \mathbf{v} )
* ( \mathbf{F}*w = k_1,\mathbf{v}*{rel} + k_2,|\mathbf{v}*{rel}|,\mathbf{v}*{rel} )

Parámetros usados:

* ( \mathbf{v}_{wind} = (10, 0, 0) )
* ( k_1 = 1.0 ), ( k_2 = 0.1 )

### 4.4 Flotación (Arquímedes aprox + drag)

Cuando un pato entra en la zona de agua, se calcula una aproximación del volumen sumergido y se aplica empuje vertical
según Arquímedes:

- Fracción sumergida: `f ∈ [0,1]` a partir de cuánto del cuerpo queda bajo `waterLevel`  
- Volumen sumergido: `Vsub = V * f`
- Empuje: `Fb = rhoFluid * Vsub * g` (vertical)
- Resistencia en agua: `Fd = -c * v * f`

El “buceo y rebote” al cruzar la superficie se debe a la inercia + amortiguación (sistema subamortiguado), y se puede
interpretar como una “segunda oportunidad” de disparo.


## 5. Efectos incorporados

* Disparo de proyectiles desde la cámara.
* Viento afectando a partículas (y también a los patos).
* Spawning automático de patos con vida limitada.
* Patos con **forma/tamaño/densidad aleatoria** (masa e inercia distintas).
* Flotación + drag cuando entran en el agua.
* Colisión “bala–pato” por distancia (hit → score + feedback físico + giro).

## 6. Cumplimiento del enunciado (archivo / líneas)

Copia el estilo de tu memoria intermedia  y rellena con líneas reales (en VS: activa números de línea o usa `nl -ba fichero.cpp`).

| Requisito (enunciado)                           | Implementación                                           | Archivo / líneas                              |
| ----------------------------------------------- | -------------------------------------------------------- | --------------------------------------------- |
| 2 generadores de partículas distintos           | `PointEmitter`, `BoxEmitter` añadidos a `ParticleSystem` | `main.cpp` / `Emitter.*`                      |
| Creación/destrucción con límites                | vida + AABB (ParticleSystem) + vida (proyectiles)        | `ParticleSystem.cpp`, `ProjectileManager.cpp` |
| Destruir todo al salir                          | `cleanupPhysics` + destructores managers                 | `main.cpp`, `DuckManager::~DuckManager`       |
| Partículas con distinta masa                    | presets de proyectil (masa simulada)                     | `ProjectileManager.cpp`                       |
| Sistema de rígidos                              | patos `PxRigidDynamic`                                   | `DuckManager.cpp`                             |
| Rígidos con tamaño/forma/masa/inercia distintos | geometría aleatoria + `updateMassAndInertia(density)`    | `DuckManager::SpawnDuck`                      |
| 2 generadores de fuerza (no muelles)            | `GravityForce`, `WindForce`                              | `GravityForce.*`, `WindForce.*`, `main.cpp`   |
| Interacción usuario teclado/ratón               | ESPACIO disparar / R reiniciar                           | `main.cpp::keyPress`                          |
| Ejemplo muelle o flotación                      | flotación en `DuckManager::Update`                       | `DuckManager.cpp`                             |

## 7. Manual de usuario (README.md)

* **ESPACIO**: iniciar partida en menú / disparar en juego.
* **R**: reiniciar en Game Over.
