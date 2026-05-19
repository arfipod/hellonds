# Evangelio NDS

Lector de los cuatro evangelios para Nintendo DS, construido con BlocksDS y los
JSON de `res` en la version de la Conferencia Episcopal Espanola.

Firma: Angel R.

## Modos

- Lectura seguida: empieza en Mateo 1,1 y avanza versiculo a versiculo por los
  cuatro evangelios.
- Buscar por cita: selector de evangelio, capitulo y versiculo.
- Evangelio al azar: abre una cita aleatoria.

## Controles

- Menu: `A` lectura seguida, `B` buscar por cita, `X` cita aleatoria.
- Lector: `A`, `R` o derecha para avanzar; `L` o izquierda para retroceder;
  arriba/abajo para desplazar textos largos; `Y` para buscar cita; `B` para menu.
- Buscador: izquierda/derecha cambia el campo, arriba/abajo ajusta el valor,
  `L`/`R` hacen saltos rapidos, `A` abre la cita.

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
