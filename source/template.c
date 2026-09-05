/*
Librería estándar input/output de C.
*/
#include <stdio.h>

/*
Librería de devkitpro que permite usar acciones del Wiimote, botones, acelerómetro, sensor de infrarrojos, etc.
*/
#include <wiiuse/wpad.h>

/*
Librería estándar que nos otorga los métodos para trabajar en la wii, el buffer de video, el sistema, el audio,
los mandos de Gamecube, el lector de disco y networking básico.
*/
#include <gccore.h>

/*
Librería estándar que maneja los bool
*/
#include <stdbool.h>
/*
Es el eXternal FrameBuffer, es el lienzo que se dibuja,
aqui es donde se "prepara" la imagen que posteriormente va a enviar a la Wii

Se define como un puntero vacio ya que este buffer es solo un conjunto de bytes, no un tipo de dato concreto.
*/

static void *xfb = NULL;

/*
Es el Render Mode. Aqui se guarda la información sobre la salida de la consola, si es panoramica, 4:3, NTCS, PAL...

Se define mediante un tipo de estructura del paquete gccore.h llamada GXRModeObj que contiene que agrupa todas las configuraciones mencionadas.
*/
static GXRModeObj *rmode = NULL;

/*
Se declara la función main que devuelve un numero entero 0 si es exitoso y cualquier otro valor si ha habido algun fallo, es una convención de C

El tema de recibir parametros con argc y argv no se usa en Wii apenas, pero se sigue haciendo por convención con C++
*/
int main(int argc, char **argv)
{
        bool runProgram = true;
        /* Inicia el sistema de video de la Wii  */
        VIDEO_Init();

        /* Inicia el controlador de los Wiimotes*/
        WPAD_Init();

        /*
        Esta función detecta automaticamente el modo en el que esta funcionando el video de la tele.
        PAL, NTSC, 4:3, 16:9 y devuleve todo eso a la variable que almacena la configuración de la tele.
        */
        rmode = VIDEO_GetPreferredMode(NULL);

        /*
        Permite acceder al buffer de memoeria de la Wii, aunque la Wii utiliza todo en la misma memoria fisica de manera logica se puede acceder mediante 2 metodos.

        K0: Se accede mediante cache, es mas rapida pero no representa el estado real de la memoria al ser cache.
        K1: Es la memoria accedida sin cache, es mas lenta pero representa el estado real de la memoria.

        Con esta función pasamos la memoria con cache a la sin cache para ver el estado real de la memoria.

        Es una instrucción estandar en el desarrollo de Homebrew, no es necesario memorizarla ya que esta en la mayoría de plantilla.

        */
        xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

        /*
        Esta función nos permite inicializar una consola de texto virtual, es decir un printf para la Wii.

        xfb es el buffer que creamos anteriormente para mostrar en pantalla.

        20 y 20 son los pixeles de margen, es decir nuestro texto estara 20 pixeles separado de la parte superior e izquierda,
        no es necesario, pero hará que el código sea mas fácil de leer.

        rmode->fbWidth y rmode->xfbHeight estamos obteniendo el tamaño del fubber en pixeles de la pantalla.
        Como hemos definido anteriormente el ancho y tipo de pantalla automaticamente este se adaptará a nuestra pantalla.

        rmode -> fbWidth * VI_DISPLAY_PIX_SZ, estamos obteniendo el tamañao en bytes del ancho de la pantalla,
        VI_DISPLAY_PIX_SZ es una contante que nos dice cuantos bytes ocupa un pixel, si lo multiplicamos por el número de pixeles del horizontal obtenemos el numero de bytes por linea.

        */

        console_init(xfb, 20, 20, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);

        /*
        Esta función configura el chip de video para funcionar con la configuración de la pantalla que hemos detectado anteriormente.
        */
        VIDEO_Configure(rmode);

        /*
        Le estamos pasando al hardware que queremos que la proxima imagen que se muestre la saquemos del buffer que hemos ido creando.
        */
        VIDEO_SetNextFramebuffer(xfb);

        /*
        Le estamos indicando que no deje la pantalla en negro mientras corga, necesitariamos hacer esto mientras carga algo, pero como no es el caso.
        */
        VIDEO_SetBlack(FALSE);

        /*
        Aplica todos los cambios anteriores al hardware real
        */
        VIDEO_Flush();

        /*
        Con este conjunto de instrucciones controlamos que solo leemos el buffer de pantalla cuando se haya terminado de actualizar,
        es decir estamos impidiendo que leamos el buff de la pantalla mientras aun se esta escribiendo el buffer de la pantalla impdiendo que veamos información rota.

        VIDEO_WaitVSync ya que el programa se queda esperando a que termine de dibujar un fotograma para dibujar el sigueinte en lugar de que no lo haga la cantidad de veces que pueda el procesador.

        */
        VIDEO_WaitVSync();
        if (rmode->viTVMode & VI_NON_INTERLACE)
                VIDEO_WaitVSync();

        /*
        Esto es la función estandar de C para imprimir texto por pantalla

        \x1b[2;0 es simplemente una orden que mueve el cursor que imprimer el texto a la fila 2 para que no aparezca en la esquina superior de la pantalla simplemente pegado.

        */
        printf("\x1b[0;2HHola Mundo, soy Carlos Chacon desde una Wii\x1b[2;2H(https://github.com/ChaconMoon/Hello-Wii-World)");
        printf("\x1b[4;2H\x1b[4mPulsa el boton HOME para salir del programa.");

        while (runProgram)
        {
                /* Se escanean los mandos en cada ciclo de la aplicación*/
                WPAD_ScanPads();

                /*Accedes a los botones que ha pulsado el primer mando, es decir el mando 1 */
                u32 buttomPressed = WPAD_ButtonsDown(0);

                /*
                Ejecuta VIDEO_WaitSync en cada vuelta del programa actualice la pantalla indefinidamente.
                */
                VIDEO_WaitVSync();

                /*
                En los mandos de la Web el valor del mando se representa como un valor de 32 bits con "32" posibles pulsaciones de mando, en los que cada pulsación activa uno de los bits.
                Aqui estamos comprobando si los botones pulsados, WPAD_BUTTON_HOME es una constante que contiene el valor en bits si se ha pulsado el botón home.

                Al hacer una operación AND con el valor si el botón home esta pulsado estamos comprobando si el bit del botón esta siendo pulsado porque este operador devolverá un número distinto de a 0.
                Es decir si los inputs contienen el bit que buscamos entramos en el bloque de la condición.
                */
                if (buttomPressed & WPAD_BUTTON_HOME)
                {
                        /* Damos la orden de parar la ejecución del programa*/
                        runProgram = false;
                        /* Pone toda la pantalla a negro*/
                        VIDEO_SetBlack(TRUE);

                        /* Escribimos el buffer de datos en la pantalla y sincronizamos*/
                        VIDEO_Flush();
                        VIDEO_WaitVSync();
                }
        }

        return 0;
}
