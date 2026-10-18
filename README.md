# Proyecto Final - Computacion Grafica Teoría

## Como empezar (solo la primera vez)

1. Clonar el repo (con GitHub Desktop o con `git clone`).
2. Extraer `Librerias.zip` **dentro de la carpeta del repo**.
   Al final tiene que quedar asi:
   ```
   Proyecto_Final_CG_Lab/
     Librerias/
       dll/  glm/  include/  lib/
     Models/
     shaders/
     src/
     Textures/
     Proyecto_Final.slnx
     ...
   ```
   Si queda `Librerias/Librerias/...` o las carpetas `glm`, `include`, etc. sueltas afuera, hay que acomodarlo.
3. Abrir `Proyecto_Final.slnx` con Visual Studio 2026.
4. Arriba revisar que diga **Debug** y **x64**, y darle a **F5** (o al boton verde).

Si al compilar sale `Falta la carpeta Librerias...` es que el paso 2 no quedo bien.

## Controles

| Tecla       | Que hace                     |
|-------------|------------------------------|
| W A S D     | Mover la camara              |
| Mouse       | Voltear la camara            |
| ESC         | Cerrar                       |

Capturas de cada fase del proyecto: [EVIDENCIAS.md](EVIDENCIAS.md).

---

## Mapa del proyecto

```
Proyecto_Final_CG_Lab/
|-- Proyecto_Final.slnx        <- esto es lo que se abre con VS
|-- Proyecto_Final.vcxproj     <- configuracion del proyecto (rutas, librerias, etc.)
|-- README.md                  <- este archivo
|
|-- src/                       <- todo el codigo C++
|   |-- main.cpp               <- LA ESCENA: aqui se carga y se dibuja todo
|   |-- Window.h/.cpp          <- ventana, teclado y mouse (aqui se agregan teclas)
|   |-- Camera.h/.cpp          <- camara en primera persona
|   |-- Shader_light.h/.cpp    <- compila los shaders y les manda datos (luces, matrices)
|   |-- CommonValues.h         <- maximo de luces puntuales y spotlights
|   |-- Mesh_tn.h/.cpp         <- malla (VAO/VBO/IBO) con textura y normales
|   |-- Texture.h/.cpp         <- carga de imagenes como textura
|   |-- Model.h/.cpp           <- carga de modelos (.obj, etc.) con Assimp
|   |-- Skybox.h/.cpp          <- skybox (cubemap)
|   |-- Light.h/.cpp           <- clase base de las luces
|   |-- DirectionalLight.h/.cpp<- luz tipo sol
|   |-- PointLight.h/.cpp      <- luz tipo foco
|   |-- SpotLight.h/.cpp       <- luz tipo linterna (cono)
|   `-- Material.h/.cpp        <- brillo especular de los objetos
|
|-- shaders/
|   |-- shader_light.vert/.frag<- shader de todos los objetos (Phong con texturas)
|   `-- skybox.vert/.frag      <- shader del skybox
|
|-- Models/                    <- modelos (.obj + .mtl)
|-- Textures/                  <- TODAS las imagenes (tambien las texturas de los modelos)
|   |-- plain.png              <- textura blanca, se usa si un modelo no tiene textura (no borrar)
|   `-- Skybox/                <- las 6 caras del skybox
|
|-- Librerias/   (NO esta en GitHub, viene en Librerias.zip)
|   |-- glm/                   <- matematicas (vectores, matrices)
|   |-- include/               <- headers de GLEW, GLFW, Assimp y stb_image
|   |-- lib/                   <- .lib para el linker
|   `-- dll/                   <- DLLs, se copian solas a build/ al compilar
|
`-- build/       (NO se sube, lo genera VS al compilar: el .exe y archivos temporales)
```

En el Explorador de soluciones de VS los archivos estan agrupados en carpetas virtuales:
**Escena** (main.cpp), **Base**, **Graficos**, **Iluminacion** y **Shaders**.

### Como esta acomodado main.cpp

Esta dividido en 4 bloques marcados con `// ==================`:

1. **Variables globales**: aqui se declaran texturas, modelos, materiales y luces.
2. **Geometria hecha a mano**: plaza, plano de vegetacion, cubo y calle del circuito (`CreateObjects`, `CrearCubo` y `CrearCalle`).
   `DibujarBloque` usa el cubo para dibujar los edificios provisionales como bloques de color.
3. **Carga de recursos** (dentro de `main`): se cargan texturas, modelos, skybox, materiales y luces.
4. **Ciclo principal** (el `while`): se dibuja todo cada frame. Primero los objetos opacos y al final los transparentes.

### Reglas importantes

- El programa corre desde la carpeta del proyecto, asi que las rutas son relativas a ella:
  `"Textures/algo.png"`, `"Models/algo.obj"`. Usar `/` en las rutas.
- Las texturas de los modelos **siempre** se buscan en `Textures/`, aunque el .mtl diga otra ruta
  (solo se toma el nombre del archivo).
- Las normales apuntan **hacia afuera** de cada cara, igual que en Blender (un piso lleva
  normal `(0, 1, 0)`, la cara de enfrente de un cubo `(0, 0, 1)`). Si una cara se ve oscura cuando
  deberia estar iluminada, probablemente su normal esta al reves.
- Los objetos con transparencia se dibujan **al final**, despues de todos los opacos.
- Si se cambia `MAX_POINT_LIGHTS` o `MAX_SPOT_LIGHTS` en `CommonValues.h`, hay que poner
  el mismo numero en `shaders/shader_light.frag`.
- La consola que se abre junto con la ventana muestra los errores (texturas o modelos que no
  se encontraron, errores de shaders, etc.). Revisarla siempre que algo no salga.

---

## Guia: como agregar cosas

### Plantilla para dibujar cualquier objeto

Dentro del `while`, en la parte de objetos opacos (copiar, pegar y cambiar lo necesario):

```cpp
//nombre del objeto
model = glm::mat4(1.0);
model = glm::translate(model, glm::vec3(x, y, z));
model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
glUniform3fv(uniformColor, 1, glm::value_ptr(color));
miTextura.UseTexture();                                                //no va si es un Model
Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
meshListModel[i]->RenderMeshModel();   //o  MiModelo_M.RenderModel();
```

El orden importa: primero translate, luego rotate y al final scale.

### Una textura nueva

1. Poner la imagen en `Textures/`.
2. En variables globales: `Texture pastoTexture;`
3. En la carga de recursos:
   ```cpp
   pastoTexture = Texture("Textures/pasto.png");
   pastoTexture.LoadTextureA();   //png o tga (con alfa). Para jpg usar LoadTexture()
   ```
4. Antes de dibujar el objeto: `pastoTexture.UseTexture();`

Primero se asigna la ruta y despues se carga, en ese orden.

### Un modelo nuevo (.obj)

1. Exportar desde Blender / 3ds Max / Maya como `.obj` con su `.mtl`.
2. Poner el `.obj` y el `.mtl` en `Models/` y **sus texturas en `Textures/`**.
3. Revisar en el `.mtl` que la linea `map_Kd` tenga el nombre correcto de la textura
   (la ruta no importa, solo el nombre).
4. En variables globales: `Model Arbol_M;`
5. En la carga de recursos:
   ```cpp
   Arbol_M = Model();
   Arbol_M.LoadModel("Models/arbol.obj");
   ```
6. En el `while` usar la plantilla y al final `Arbol_M.RenderModel();` (sin `UseTexture`,
   el modelo usa sus propias texturas).

Si sale blanco es que no encontro su textura (se le pone `plain.png`), revisar la consola.
Si sale muy grande o muy chico, ajustar con `glm::scale`.

### Un objeto hecho a mano (geometria propia)

1. Hacer una funcion como `CrearCubo()` con su arreglo de vertices e indices.
   Cada vertice lleva 8 valores: `x, y, z,  u, v,  nx, ny, nz`.
2. Crear la malla y meterla a la lista:
   ```cpp
   MeshModel* obj = new MeshModel();
   obj->CreateMeshModel(vertices, indices, numFloats, numIndices);
   meshListModel.push_back(obj); //meshListModel[3], [4], ...
   ```
   Ojo: el tercer parametro es la **cantidad de floats** del arreglo (vertices x 8),
   no la cantidad de vertices.
3. Si no quieres escribir las normales a mano, se pueden calcular con `calcAverageNormals(...)`
   **antes** de `CreateMeshModel`.
4. Llamar la funcion en `main()` junto a `CreateObjects()` y `CrearCubo()` y anotar que indice le toco.

### Un material

```cpp
Material Material_metal;                    //variables globales
Material_metal = Material(2.0f, 128);       //carga de recursos: (intensidad especular, shininess)
Material_metal.UseMaterial(uniformSpecularIntensity, uniformShininess);  //antes de dibujar
```

### Una luz

Las luces se crean en la carga de recursos, en el siguiente lugar libre del arreglo, y se
suma 1 al contador:

```cpp
pointLights[1] = PointLight(1.0f, 0.5f, 0.0f,    //color r g b
    0.0f, 1.0f,                                   //intensidad ambiental, difusa
    -3.0f, 2.0f, 0.0f,                            //posicion
    1.0f, 0.09f, 0.032f);                         //atenuacion: constante, lineal, exponente
pointLightCount++;
```

- Maximo 3 puntuales y 4 spotlights (`CommonValues.h`).
- `spotLights[0]` es la linterna de la camara.
- Para mover una luz mientras corre el programa, dentro del `while`:
  `spotLights[1].SetPos(nuevaPos);` o `spotLights[1].SetFlash(pos, dir);`
- Para prender/apagar luces se puede cambiar el contador que se manda en
  `SetPointLights(pointLights, pointLightCount)`.

### Una tecla nueva / mover algo con el teclado

Se hace igual que la variable `muevex` (teclas Y/U):

1. `Window.h`: agregar la variable en `private` (ej. `GLfloat rotaPuerta;`) y su getter en `public`
   (`GLfloat getrotaPuerta() { return rotaPuerta; }`).
2. `Window.cpp`: darle valor inicial en **los dos** constructores.
3. `Window.cpp`, en `ManejaTeclado`:
   ```cpp
   if (key == GLFW_KEY_P && action == GLFW_PRESS)
   {
       theWindow->rotaPuerta += 10.0f;
   }
   ```
   (`action == GLFW_PRESS` = una vez por pulsacion, `action != GLFW_RELEASE` = mientras se mantenga)
4. En `main.cpp` usarla: `mainWindow.getrotaPuerta()`.

No usar W, A, S, D (son de la camara) ni ESC.

### Cambiar el skybox

Poner las 6 imagenes en `Textures/Skybox/` y cambiar las rutas en `main.cpp`.
El orden tiene que ser: **rt, lf, dn, up, bk, ft** (derecha, izquierda, abajo, arriba, atras, enfrente).

### Objetos con transparencia

- La imagen debe ser .png con canal alfa y cargarse con `LoadTextureA()`.
- Se dibujan al final del `while`, entre `glEnable(GL_BLEND)` y `glDisable(GL_BLEND)`
  (el plano de vegetacion, `meshListModel[1]`, sirve para eso).

### Una clase o archivo .cpp nuevo

Crear el archivo dentro de `src/` y agregarlo al proyecto con clic derecho en la carpeta virtual >
Agregar > Elemento existente. Si se usa "Agregar > Clase", cambiar la ubicacion a la carpeta `src`
(VS los pone en la raiz por defecto). Despues hacer commit tambien del `.vcxproj` y el `.vcxproj.filters`.

---

## Trabajo en equipo con Git

- **Antes de empezar a trabajar: Pull.** Al terminar: commit y push. Asi evitamos conflictos.
- Commits chicos y con mensaje claro (`agrega modelo de la casa`, `arregla luz de la lampara`).
- `main.cpp` lo tocamos todos: cada quien agrega sus cosas en su propio bloque con un comentario
  (`//--- objetos de Fulano ---`) y no reacomodar ni re-formatear codigo de otros.
- No subir nada de `Librerias/` ni `build/` (ya esta en el `.gitignore`).
- GitHub no acepta archivos de mas de 100 MB. Mantener modelos y texturas ligeros
  (texturas de 1024 o 2048 px estan bien).
- Si alguien agrega una libreria nueva, se actualiza `Librerias.zip` y se le pasa a todos.

---

## Problemas comunes

| Problema | Solucion |
|---|---|
| `Falta la carpeta Librerias` al compilar | Extraer `Librerias.zip` como dice "Como empezar". |
| `No se encontro la textura: ...` en la consola | Revisar nombre y extension exactos y que este en `Textures/`. |
| `Fallo al cargar el modelo: ...` | Revisar la ruta en `LoadModel` y que el .obj este en `Models/`. |
| Modelo blanco | No encontro su textura: revisar `map_Kd` del .mtl y que la imagen este en `Textures/`. |
| Una cara se ve negra u oscura | Normal al reves (tiene que apuntar hacia afuera) o no le llega ninguna luz. |
| Transparencia con fondo negro/blanco | Usar `LoadTextureA()` y dibujarlo al final con blending. |
| `Error al compilar el ... shader` | La consola dice la linea del error dentro del shader. |
| `LNK1168: cannot open ...exe for writing` | El programa sigue abierto, cerrarlo y volver a compilar. |
| Falta una DLL al correr | Volver a compilar (se copian solas de `Librerias/dll`) o revisar que el zip este completo. |
