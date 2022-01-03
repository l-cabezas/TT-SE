
#include "lcd.h"
#include <stdint.h>
#include "MKL46Z4.h"

#include "Touch_Sen.h"



uint16_t liminf=100;
uint16_t limsup=300;
int vabien=1;
volatile uint32_t g_systickCounter;
int empieza =0;
int lento =0;
int rapido =0;



void leds_ini()
{
  SIM->COPC = 0;
  SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK;
  PORTD->PCR[5] = PORT_PCR_MUX(1);
  PORTE->PCR[29] = PORT_PCR_MUX(1);
  GPIOD->PDDR |= (1 << 5);
  GPIOE->PDDR |= (1 << 29);
  // inicialmente los dos leds estan apagados
  GPIOD->PSOR = (1 << 5);
  GPIOE->PSOR = (1 << 29);
}


void irclk_ini()
{
  MCG->C1 = MCG_C1_IRCLKEN(1) | MCG_C1_IREFSTEN(1);
  MCG->C2 = MCG_C2_IRCS(0); 
}

void delay(void)
{
  volatile int i;

  for (i = 0; i < 100000; i++);
}


int main()
{
    uint16_t x;
    uint32_t i;


    leds_ini();
    irclk_ini(); 
    lcd_ini();
    Touch_Init();


    while(1)
    {
    	while(limsup < 700){
    		for ( i = 0; i < 250000; i++); // Delay
	    	//x = Touch_Scan_LH();    // para la depuracion, recuperar el valor que devuelve el touchpad
	    	//lcd_display_dec(x);// para la depuracion, mostrar el valor que devuelve el touchpad
	    	//lcd_display_dec(limsup); //para al depuracion, mostrar como aumenta el limite superior (cambiar a limite inferior)
		if((x>100)){ //si todavia no se empezo a tocar el touchpad, aproximadamente el valor de x es 70-80, por lo tanto, en el momento en que se empieza a deslizar, siempre es superior a 100
			if((x>=liminf) && (x<=limsup)){ //en el caso en que el dedo se encuentre dentro de los limites de lo correcto
				liminf = liminf +25; //paso del limite inferior.
				limsup = limsup +25; //paso del limtie superior. incrementar el valor del paso (mas que 25) hace que haya que desplazar el dedo mas deprisa
				empieza = 1; //es porque al iniciar el touchpad primero hay un pico en los valores que se leen. de esta forma solo se modifica la variable de "vabien" una vez que este entre los limites superior y inferior, que es despues de este minimo instante incial, cuando se empieza a tocar el touchpad
			}else{
				if(empieza){//de esta forma descartamos el instante inicial,es decir una vez que el valor que devuelve el touchpad ha estado entre los limites superior e inferior
					vabien=0;
					if(x<liminf){rapido =0;lento=1;}; //si el valor devuelto por el touchpad es menor que el limite inferior es que va demasiado despacio
					if(x>limsup){rapido = 1; lento =0;};//si el valor devuelto por el touchpad es mayor que el limite inferior es que va demasiado deprisa
					break; //se podria eliminar si se quiere que solo se vea si se ha hecho bien o mal al terminar ca da intento (asi seria mas fiel al juego original) pero los intentos son mas dinamicos con este break
				};
				
			}
		}
		//para comprobar si vas bien a medida que mueves el dedo(facilita depuracion)
		/*if(vabien){
			GPIOD->PCOR =(1<<5);//encender el verde (poner un 0)
    			GPIOE->PSOR =(1<<29);//apagar el led rojo (poner un 1)
		}else{
			GPIOE->PCOR =(1<<29);
        		GPIOD->PSOR =(1<<5);
		};*/
	}
	for ( i = 0; i < 500000; i++); // espera
	//lcd_clear(); //se limpia el lcd de los valores de la depuracion (imprimir la posicion del dedo o los limites)
	if(vabien){
			GPIOD->PCOR =(1<<5);//encender el verde (poner un 0)
    			GPIOE->PSOR =(1<<29);//apagar el led rojo (poner un 1)
        		lcd_set_ok(); //imprimir un ok por la pantalla (funcion en lcd.c)
		}else{
    			GPIOE->PCOR =(1<<29);
        		GPIOD->PSOR =(1<<5);
	};
	if(lento){lcd_set_slow();}; //imprimir slow por la pantalla (funcion en lcd.c)
	if(rapido){lcd_set_fast();}; //imprimir fast por la pantalla (funcion en lcd.c)
	break;
    }

}
