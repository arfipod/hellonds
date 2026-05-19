# Evangelio NDS

Lector de los cuatro evangelios para Nintendo DS, construido con BlocksDS y los
JSON de `res` en la version de la Conferencia Episcopal Espanola.

Firma: Angel R.

## Modos

- Lectura seguida: empieza en Mateo 1,1 y avanza versiculo a versiculo por los
  cuatro evangelios.
- Buscar por cita: selector de evangelio, capitulo y versiculo.
- Evangelio al azar: abre una cita aleatoria.
- Modo libro: usa las dos pantallas como paginas completas con varios
  versiculos.

## Controles

- Menu: `A` lectura seguida, `B` buscar por cita, `X` cita aleatoria, `Y` modo
  libro.
- Lector: `A`, `R` o derecha para avanzar; `L` o izquierda para retroceder;
  arriba/abajo para desplazar textos largos; `Y` alterna modo libro; `SELECT`
  busca cita; `B` vuelve al menu.
- Modo libro: `A`, `R` o derecha pasan pagina; `L` o izquierda vuelven pagina;
  `Y` vuelve al lector normal; `SELECT` busca cita; `X` cita aleatoria; `B`
  vuelve al menu.
- Buscador: izquierda/derecha cambia el campo, arriba/abajo ajusta el valor,
  `L`/`R` hacen saltos rapidos, `A` abre la cita, `Y` la abre en modo libro.

## Arquitectura

- `source/main.c`: arranque de hardware, timers, lectura de botones y bucle
  principal.
- `source/gospel_app.*`: maquina de estados de la aplicacion, menus, lector,
  modo libro y buscador.
- `source/gospel_text.*`: ajuste de texto a 31 columnas y composicion de
  paginas de dos pantallas.
- `source/gospel_console.*`: inicializacion de consola y glifo personalizado
  para la eñe.
- `source/gospel_random.*`: generador pseudoaleatorio y mezcla de entropia de
  timers/input.
- `source/gospel_data.*`: datos generados a partir de los JSON de `res`.
- `tools/generate_gospel_data.py`: regenerador de `gospel_data.c`.

## Generar datos y compilar

Si cambias los JSON, regenera la tabla C:

```bash
python3 tools/generate_gospel_data.py
```

Compila desde esta carpeta:

```bash
make clean
make
```

El resultado esperado es `gospel_nds.nds`.

Si el entorno no esta cargado:

```bash
source /opt/wonderful/bin/wf-env
cd gospel_nds
python3 tools/generate_gospel_data.py
make clean
make
```
