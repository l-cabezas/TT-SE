
#include "lcd.h"
#include <stdint.h>
#include "MKL46Z4.h"

#include "Touch_Sen.h"



uint16_t liminf=50;
uint16_t limsup=500;
int vabien=1;
int empieza =0;
int lento =0;
int rapido =0;


// LEFT_SWITCH (SW2) = PTC12
void sw_ini()
{
  SIM->COPC = 0;
  PORTC->PCR[12] |= PORT_PCR_MUX(1) | PORT_PCR_PE(1);
  GPIOC->PDDR &= ~(1 << 12);
  
  PORTC->PCR[12] |= PORT_PCR_IRQC(0xA); // IRQ on falling edge
  
  // IRQ#31: Pin detect for PORTS C & D
  NVIC_SetPriority(31, 0); // Max priority for IRQ#31
  NVIC_EnableIRQ(31);      // Enable IRQ#31
}

void PORTDIntHandler(void)
{
    NVIC_DisableIRQ(31);      // Disable IRQ#31
}


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
    sw_ini();
    Touch_Init();
    __WFI();
    
    
    

    while(1)
    {
    	
	lcd_set_go();
	
	while(limsup < 1100){
	    	for ( i = 0; i < 250000; i++); // Delay con que se mide cada valor de x
		x = Touch_Scan_LH();    // para la depuración, recuperar el valor que devuelve el touchpad
		//lcd_display_dec(x);// para la depuración, mostrar el valor que devuelve el touchpad
		//lcd_display_dec(liminf); //para al depuracion, mostrar como aumenta el limite superior (cambiar a limite inferior)
		
		if((x>100)){ //si todavia no se empezo a tocar el touchpad, aproximadamente el valor de x es 70-80, por lo tanto, en el momento en que se empieza a deslizar, siempre es superior a 				     //85-100
			if((x>=liminf) && (x<=limsup)){ //en el caso en que el dedo se encuentre dentro de los limites de lo correcto
				liminf = liminf +25; //paso del limite inferior.
				limsup = limsup +25; //paso del limtie superior. Incrementar el valor del paso (más que 25) hace que haya que desplazar el dedo más deprisa
				empieza = 1; //es porque al iniciar el touchpad primero hay un pico en los valores que se leen. De esta forma solo se modifica la variable de "vabien" una vez que este entre los limites superior y inferior, que es despues de este mínimo instante incial, cuando se empieza a tocar el touchpad
			}else{
				if(empieza){//de esta forma descartamos el instante inicial,es decir una vez que el valor que devuelve el touchpad ha estado entre los limites superior e inferior
					lcd_clear();//limpiamos el lcd para que deje de aparecer el go que aparece inicialmente
						
					if(x<liminf){lento=1;rapido=0;}; //si el valor devuelto por el touchpad es menor que el limite inferior es que va demasiado despacio
					if(x>limsup){rapido = 1;lento=0;};//si el valor devuelto por el touchpad es mayor que el limite inferior es que va demasiado deprisa
					vabien=0;
						
					break; 
				};
					
			}
		}

	}
	
		
	if(vabien){ //se ha conseguido deslizar a la velocidad adecuada
		GPIOD->PCOR =(1<<5);//encender el verde (poner un 0)
	    	GPIOE->PSOR =(1<<29);//apagar el led rojo (poner un 1)
		lcd_set_pass(); //imprimir un ok por la pantalla (funcion en lcd.c)
		for ( i = 0; i < 2000000; i++);
		break; //se rompe el bucle y se termina el juego
	}else{ //se ha deslizado o muy despacio o muy rapido
	    	GPIOE->PCOR =(1<<29);
		GPIOD->PSOR =(1<<5);
		if(rapido){lcd_set_fast();}; //imprimir fast por la pantalla (funcion en lcd.c)
		if(lento){lcd_set_slow();}; //imprimir slow por la pantalla (funcion en lcd.c)
		vabien = 1; //reiniciamos los valores iniciales apra volver a empezar la aprtida
		empieza = 0; 
		rapido =0;
		lento =0;
		liminf=50;
		limsup=500;
		for ( i = 0; i < 1000000; i++); // espera de 1 segundo mientras se muestra el mensaje
		lcd_clear(); //limpiamos el lcd para el siguiente intento
		GPIOE->PSOR =(1<<29);//apagar el led rojo (es el unico que puede estar encendido porque si no se hubiese roto el bucle)	
	};
    }

}

