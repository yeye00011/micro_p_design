#include <msp430.h> 

unsigned int cnt = 0;
int state = 0;
int a = 0;
unsigned int data[4] = { 0, 0, 0, 0 };
int encoder_data = 0;
int seg_state = 0;
unsigned int digits[10] = { 0xDB, 0x50, 0x1F, 0x5D, 0xD4, 0xCD, 0xCF, 0xD8, 0xDF, 0xDD };
unsigned int keyout = 0;
unsigned int seg1234 = 1; // 자릿수
unsigned int key_data[3] = { 0, 0, 0 };
unsigned int i = 0;
int pw = 0;
unsigned int cycle = 0;


/*
data [0] = 0x59;
data [1] = 0x09;
data [2] = 0x09;
data [3] = 0x8b;
*/

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

    //test LED
    P1DIR |= BIT0;  // PORT DIR
    P1OUT &= ~BIT0; // LED OFF


    P1OUT |= BIT1;  // SW PULL UP
    P1REN |= BIT1;
    P1IE |= BIT1;
    P1IES |= BIT1;
    P1IFG &= ~BIT1;


    TA0CTL = TASSEL_2 + MC_1 + TACLR; // timer for dynamic segment
    TA0CCTL0 = CCIE;
    TA0CCR0 = 6000;

    P3OUT &= 0x0000;
    P3DIR |= 0xffff;
    P4OUT &= ~0x0001;
    P4DIR |= 0x000f;

    //keypad
    P2DIR |= (BIT0 | BIT2 | BIT3); //output
    P2OUT |= (BIT0 | BIT2 | BIT3); //high
    P6REN |= (BIT3 | BIT4 | BIT5 | BIT6); //input
    P6OUT |= (BIT3 | BIT4 | BIT5 | BIT6); //pull up

    P2DIR |= (BIT0 | BIT2);
    P2OUT &= ~(BIT0 | BIT2); //set low

    P2DIR |= (BIT5 | BIT4); // PWM
    P2SEL |= (BIT5 | BIT4);

    TA2CTL = TASSEL_2 + MC_1; // timer for PWM speed
    TA2CCR0 = 1000;
    TA2CCTL2 = OUTMOD_6;
    TA2CCR2 = 0;
    TA2CCTL1 = OUTMOD_6;
    TA2CCR1 = 0;





    __bis_SR_register(GIE);


    while (1) {
        switch (state) {
        case 0:
            P1IE &= ~BIT2;
            P1IE &= ~BIT3;
            P1OUT |= BIT0;
            data[0] = 0x00;
            data[1] = 0x00;
            data[2] = 0x00;
            data[3] = 0x00;
            break;

        case 1:
            P1OUT &= ~BIT0;

            if (encoder_data < 833) {
                data[0] = 0x08;
                data[1] = 0x00;
                data[2] = 0x00;
                data[3] = 0x00;
            }
            else if (encoder_data >= 833 && encoder_data < 1666) {
                data[0] = 0x00;
                data[1] = 0x08;
                data[2] = 0x00;
                data[3] = 0x00;
            }
            else if (encoder_data >= 1666 && encoder_data < 2499) {
                data[0] = 0x00;
                data[1] = 0x00;
                data[2] = 0x08;
                data[3] = 0x00;
            }
            else if (encoder_data >= 2499 && encoder_data < 3332) {
                data[0] = 0x00;
                data[1] = 0x00;
                data[2] = 0x00;
                data[3] = 0x08;
            }
            else if (encoder_data >= 3332 && encoder_data < 4165) {
                data[0] = 0x00;
                data[1] = 0x00;
                data[2] = 0x00;
                data[3] = 0x80;
            }
            else if (encoder_data >= 4165 && encoder_data < 4998) {
                data[0] = 0x00;
                data[1] = 0x00;
                data[2] = 0x00;
                data[3] = 0x02;
            }
            else if (encoder_data >= 4998 && encoder_data < 5831) {
                data[0] = 0x00;
                data[1] = 0x00;
                data[2] = 0x00;
                data[3] = 0x01;
            }
            else if (encoder_data >= 5831 && encoder_data < 6664) {
                data[0] = 0x00;
                data[1] = 0x00;
                data[2] = 0x01;
                data[3] = 0x00;
            }
            else if (encoder_data >= 6664 && encoder_data < 7497) {
                data[0] = 0x00;
                data[1] = 0x01;
                data[2] = 0x00;
                data[3] = 0x00;
            }
            else if (encoder_data >= 7497 && encoder_data < 8330) {
                data[0] = 0x01;
                data[1] = 0x00;
                data[2] = 0x00;
                data[3] = 0x00;
            }
            else if (encoder_data >= 8330 && encoder_data < 9163) {
                data[0] = 0x40;
                data[1] = 0x00;
                data[2] = 0x00;
                data[3] = 0x00;
            }
            else if (encoder_data >= 9163 && encoder_data <= 9999) {
                data[0] = 0x10;
                data[1] = 0x00;
                data[2] = 0x00;
                data[3] = 0x00;
            }

            if (encoder_data >= 4165 && encoder_data < 4998) {
                seg_state = 1;

                P2OUT &= ~BIT2;           // 1행
                P2OUT |= (BIT0 | BIT3);

                if ((P6IN & BIT3) == 0) {
                    keyout = 1;
                }
                else if ((P6IN & BIT6) == 0) {
                    keyout = 4;
                }
                else if ((P6IN & BIT5) == 0) {
                    keyout = 7;
                }
                else if ((P6IN & BIT4) == 0) {
                    while ((P6IN & BIT4) == 0) { // chattering avoidance
                        keyout = 0;
                    }
                    seg1234++;
                }

                P2OUT &= ~BIT0;           // 2행
                P2OUT |= (BIT2 | BIT3);

                if ((P6IN & BIT3) == 0) {
                    keyout = 2;
                }
                else if ((P6IN & BIT6) == 0) {
                    keyout = 5;
                }
                else if ((P6IN & BIT5) == 0) {
                    keyout = 8;
                }
                else if ((P6IN & BIT4) == 0) {
                    keyout = 0;
                }

                P2OUT &= ~BIT3;           // 3행
                P2OUT |= (BIT0 | BIT2);

                if ((P6IN & BIT3) == 0) {
                    keyout = 3;
                }
                else if ((P6IN & BIT6) == 0) {
                    keyout = 6;
                }
                else if ((P6IN & BIT5) == 0) {
                    keyout = 9;
                }
                else if ((P6IN & BIT4) == 0) {
                    seg1234 = 1;

                    i = 0; // 3자리 overflow 방지
                    while (i < 3) {
                        key_data[i] = 0;
                        i++;
                    }
                }

                //logic 3자리 숫자를 받는 과정
                if (seg1234 == 1) {
                    key_data[2] = keyout;
                }
                else if (seg1234 == 2) {
                    key_data[1] = keyout;
                }
                else if (seg1234 == 3) {
                    key_data[0] = keyout;
                }
                else if (seg1234 == 4) { //3자리숫자로 변환하는 과정
                    seg1234 = 0;
                    pw = key_data[2] * 100 + key_data[1] * 10 + key_data[0] * 1;

                    if (pw == 486) {
                        state = 3;
                    }
                }
            }
            else {
                seg_state = 0;
            }
            break;

        case 3:
            P1IE &= ~BIT2;
            P1IE &= ~BIT3;
            seg_state = 0;
            TA0CCR0 = 32768; // 0.5s 토글과 CCR0값 증가로 화려한 LOVE! 와!

            if (i == 0) {
                data[0] = 0x8f;
                data[1] = 0xd3;
                data[2] = 0xdb;
                data[3] = 0x83;
                i = 1;
                __delay_cycles(500000);  //0.5s
            }
            else {
                data[0] = 0x00;
                data[1] = 0x00;
                data[2] = 0x00;
                data[3] = 0x00;
                i = 0;
                __delay_cycles(500000);  //0.5s
            }
            break;

        case 4:
            seg_state = 1;
            TA0CCR0 = 6000;
            TA2CCR1 = 486; // clock wise
            TA2CCR2 = 0;
            key_data[0] = 4;
            key_data[1] = 8;
            key_data[2] = 6;
            __delay_cycles(514000);
            TA2CCR2 = 864; // anti-clock wise
            TA2CCR1 = 0;
            key_data[0] = 8;
            key_data[1] = 6;
            key_data[2] = 4;
            __delay_cycles(136000);

            TA2CCR1 = 648; // clock wise
            TA2CCR2 = 0;
            key_data[0] = 6;
            key_data[1] = 4;
            key_data[2] = 8;
            __delay_cycles(352000);

            TA2CCR2 = 486; // anti-clock wise
            TA2CCR1 = 0;
            key_data[0] = 4;
            key_data[1] = 8;
            key_data[2] = 6;
            __delay_cycles(514000);

            TA2CCR1 = 864; // clock wise
            TA2CCR2 = 0;
            key_data[0] = 8;
            key_data[1] = 6;
            key_data[2] = 4;
            __delay_cycles(136000);

            TA2CCR2 = 648; // anti-clock wise
            TA2CCR1 = 0;
            key_data[0] = 6;
            key_data[1] = 4;
            key_data[2] = 8;
            __delay_cycles(352000);



            /*P1IE |= BIT3;
            P1IE |= BIT2;


            if(cycle < 10){

                    TA2CCR1 = 486; // clock wise
                    TA2CCR2 = 0;

                key_data[0] = 4;
                key_data[1] = 8;
                key_data[2] = 6;
            }
            else if(cycle < 20){

                    TA2CCR2 = 864; // anti-clock wise
                    TA2CCR1 = 0;

                key_data[0] = 8;
                key_data[1] = 6;
                key_data[2] = 4;
            }
            else if(cycle < 30){

                    TA2CCR1 = 648; // clock wise
                    TA2CCR2 = 0;
                key_data[0] = 6;
                key_data[1] = 4;
                key_data[2] = 8;
            }
            else if(cycle < 40){

                    TA2CCR2 = 486; // anti-clock wise
                    TA2CCR1 = 0;

                key_data[0] = 4;
                key_data[1] = 8;
                key_data[2] = 6;
            }
            else if(cycle < 50){

                    TA2CCR1 = 864; // clock wise
                    TA2CCR2 = 0;

                key_data[0] = 8;
                key_data[1] = 6;
                key_data[2] = 4;
            }
            else if(cycle < 60){

                    TA2CCR2 = 648; // anti-clock wise
                    TA2CCR1 = 0;

                key_data[0] = 6;
                key_data[1] = 4;
                key_data[2] = 8;
            }
            else{
                cycle = 0;
            }
            break;
*/
            break;
        case 5:
            P1IE &= ~BIT2;
            P1IE &= ~BIT3;
            TA0CCR0 = 6000;
            TA2CCR1 = 0;
            TA2CCR2 = 0;

            key_data[0] = 0x00;
            key_data[1] = 0x00;
            key_data[2] = 0x00;

            pw = 0;
            keyout = 0;
            seg1234 = 1;
            encoder_data = 0;
            state = 0;
            break;
        }
    }
}


#pragma vector=PORT1_VECTOR
__interrupt void port1(void) {
    if (P1IFG & BIT1 == BIT1) {
        while (P1IN & BIT1 == 0) {// chattering avoidance
        }
        if (state == 0) {
            if (a == 0) {
                TA1CTL = TASSEL_1 + MC_1 + TACLR; // timer for 0.3s
                TA1CCTL0 = CCIE;
                TA1CCR0 = 9830;  // 0.3s 9830
                a = 1;
            }
            else {
                state = 1;
                a = 0;
                TA1CTL = MC_0 + TACLR;

                P1IE |= BIT3;
                P1IES |= BIT3; //falling edge select
                P1IFG &= ~BIT3;
                P1IE |= BIT2;
                P1IES |= BIT2; //falling edge select
                P1IFG &= ~BIT2;
            }
        }
        else if (state == 3) {
            state = 4;
        }
        else if (state == 4) {
            state = 5;
        }
        P1IFG &= ~BIT1;
    }
    if (P1IFG & BIT3) { //A interrupt
        if ((P1IN & BIT2) != 0) {
            encoder_data--;
        }
        else {
            encoder_data++;
        }
        P1IFG &= ~BIT3;
    }
    if (P1IFG & BIT2) { //B interrupt
        if ((P1IN & BIT3) == 0) {
            encoder_data--;
        }
        else {
            encoder_data++;
        }
        P1IFG &= ~BIT2;
    }
    if (state == 4) {
        if (encoder_data < 9330 && encoder_data >= 8660) {
            cycle++;
            encoder_data = 0;
        }
        else if (encoder_data >= 670) {
            cycle++;
            encoder_data = 0;
        }
    }

    if (encoder_data > 9999) {
        encoder_data = 0;
    }
    else if (encoder_data < 0) {
        encoder_data = 9999;
    }
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void) {
    state = 0;
    a = 0;
    TA1CTL = MC_0 + TACLR;
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void) {
    cnt++;
    if (cnt > 3) {
        cnt = 0;
    }
    if (seg_state == 0) {
        switch (cnt) {
        case 0:
            P3OUT = data[0];
            P4OUT &= ~BIT0;
            P4OUT |= (BIT1 | BIT2 | BIT3);
            break;

        case 1:
            P3OUT = data[1];
            P4OUT &= ~BIT1;
            P4OUT |= (BIT0 | BIT2 | BIT3);
            break;

        case 2:
            P3OUT = data[2];
            P4OUT &= ~BIT2;
            P4OUT |= (BIT0 | BIT1 | BIT3);
            break;

        case 3:
            P3OUT = data[3];
            P4OUT &= ~BIT3;
            P4OUT |= (BIT0 | BIT1 | BIT2);
            break;
        }
    }
    else {
        switch (cnt) {
        case 0:
            P3OUT = digits[key_data[0]];
            P4OUT &= ~BIT0;
            P4OUT |= (BIT1 | BIT2 | BIT3);
            break;

        case 1:
            P3OUT = digits[key_data[1]];
            P4OUT &= ~BIT1;
            P4OUT |= (BIT0 | BIT2 | BIT3);
            break;

        case 2:
            P3OUT = digits[key_data[2]];
            P4OUT &= ~BIT2;
            P4OUT |= (BIT0 | BIT1 | BIT3);
            break;

        case 3:
            P3OUT = 0x02;
            P4OUT &= ~BIT3;
            P4OUT |= (BIT0 | BIT1 | BIT2);
            break;
        }
    }

}
