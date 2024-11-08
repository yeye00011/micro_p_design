#include <msp430.h>
#include <stdio.h>
#include <time.h>

// reg for 1.1
unsigned int state = 0;
unsigned int set = 0;

// reg for 1.2
unsigned int digits[10] = { 0xDB, 0x50, 0x1F, 0x5D, 0xD4, 0xCD, 0xCF, 0xD8, 0xDF, 0xDD };
unsigned int dot_digits[10] = { 0xFB, 0x70, 0x3F, 0x7D, 0xF4, 0xED, 0xEF, 0xF8, 0xFF, 0xFD };
unsigned int cnt = 0;
unsigned int data = 0;
unsigned long end_time = 0;
int keyout = 0;
int keyout_before = 0;
unsigned int seg_state = 0;


void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

    P1DIR |= BIT0;  // PORT DIR
    P1DIR &= ~BIT1;
    P4DIR |= BIT7;

    P1OUT &= ~BIT0; // LED OFF
    P4OUT &= ~BIT7;

    P1OUT |= BIT1;  // SW PULL UP
    P1REN |= BIT1;

    P1IE |= BIT1;
    P1IES |= BIT1;
    P1IFG &= ~BIT1;

    TA0CTL = TASSEL_1 + MC_2 + TACLR; // timer for random number gen
    TA0CCTL0 = CCIE;

    TA1CTL = TASSEL_2 + MC_1 + TACLR; // timer for dynamic segment
    TA1CCTL0 = CCIE;
    TA1CCR0 = 6000;

    P3OUT &= 0x0000;
    P3DIR |= 0xffff;
    P4OUT &= ~0x0001;
    P4DIR |= 0x000f;




    __bis_SR_register(GIE);


    while (1) {
        switch (state) {
        case 0: // OFF
            P1OUT &= ~BIT0;
            set = 0;

            break;

        case 1:
            break;

        case 2:
            break;

        case 3:
            data = (end_time * 1000) / 32768;
            P1IE |= BIT3;
            P1IES |= BIT3; //falling edge select
            P1IFG &= ~BIT3;
            P1IE |= BIT2;
            P1IES |= BIT2; //falling edge select
            P1IFG &= ~BIT2;

            TA2CTL = TACLR;
            __delay_cycles(1000000);
            P4OUT &= ~BIT7;
            state = 4;

            break;

        case 4:

            if (keyout < 1000) {
                seg_state = 0;
            }
            else if (keyout < 2000) {
                seg_state = 1;
            }
            else if (keyout < 3000) {
                seg_state = 2;
            }
            else if (keyout < 4000) {
                seg_state = 3;
            }

            break;

        case 5:
            keyout = data * 670;
            keyout_before = keyout;

            P2DIR |= (BIT5 | BIT4); // PWM
            P2SEL |= (BIT5 | BIT4);

            TA2CTL = TASSEL_2 + MC_1; // timer for PWM speed
            TA2CCR0 = 1000;
            TA2CCTL2 = OUTMOD_6;
            TA2CCR2 = 0;
            TA2CCTL1 = OUTMOD_6;
            TA2CCR1 = 500;


            state = 6;
            break;
        case 6:
            if (keyout_before - keyout <= 670 && keyout_before - keyout > 650) {
                keyout_before = keyout;
                data--;
            }
            if (data == 0) {
                TA2CCR1 = 0;
            }
            break;

        case 7:

            set = 0;
            cnt = 0;
            data = 0;
            end_time = 0;
            keyout = 0;
            keyout_before = 0;
            seg_state = 0;

            state = 0;
            break;
        }
    }

}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void) {
    while (P1IN & BIT1 == 0) { // chattering avoidance
    }
    if (P1IFG & BIT1) {
        if (set == 0) {
            P1OUT &= ~BIT0;
            TA0CTL |= MC_0;
            if (TA0R > 0x8000) {
                TA0R -= 0x8000;
            }
            TA0CTL &= ~BIT0;
            TA0CTL |= MC_2;
            set = 1;
            state = 1;
        }
        if (state == 2) {
            end_time = TA2R;
            P4OUT |= BIT7;
            state = 3;
        }
        else if (state == 4) {
            state = 5;
        }
        else if (state == 6) {
            state = 7;
        }

        P1IFG &= ~BIT1;
    }

    if (state == 4) {
        if (P1IFG & BIT3) { //A interrupt
            if ((P1IN & BIT2) != 0) {
                keyout++;
            }
            else {
                keyout--;
            }
            P1IFG &= ~BIT3;
        }
        if (P1IFG & BIT2) { //B interrupt
            if ((P1IN & BIT3) == 0) {
                keyout++;
            }
            else {
                keyout--;
            }
            P1IFG &= ~BIT2;
        }
        if (keyout > 4000) {
            keyout = 0;
        }
        else if (keyout < 0) {
            keyout = 4000;
        }
    }
    else {
        if (P1IFG & BIT3) { //A interrupt
            if ((P1IN & BIT2) != 0) {
                keyout--;
            }
            else {
                keyout++;
            }
            P1IFG &= ~BIT3;
        }
        if (P1IFG & BIT2) { //B interrupt
            if ((P1IN & BIT3) == 0) {
                keyout--;
            }
            else {
                keyout++;
            }
            P1IFG &= ~BIT2;
        }

    }

}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
    if (set == 1) {
        state = 2;
        P1OUT |= BIT0;
        set = 2;
        TA0CCR0 = 32768;

        TA2CTL = TASSEL_1 + MC_2 + TACLR;
    }
    else if (set == 2) {
        P1OUT &= ~BIT0;
        TA0CCR0 = 65535; // 65536이 맞지만 에러로 인해 1감소시킴
    }
}


#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void) {
    cnt++;
    if (cnt > 3) {
        cnt = 0;
    }

    if (seg_state == 0) {
        switch (cnt) {
        case 0:
            P3OUT = digits[(data % 10)];
            P4OUT &= ~BIT0;
            P4OUT |= (BIT1 | BIT2 | BIT3);
            break;

        case 1:
            P3OUT = digits[(data / 10) % 10];
            P4OUT &= ~BIT1;
            P4OUT |= (BIT0 | BIT2 | BIT3);
            break;

        case 2:
            P3OUT = digits[(data / 100) % 10];
            P4OUT &= ~BIT2;
            P4OUT |= (BIT0 | BIT1 | BIT3);
            break;

        case 3:
            P3OUT = dot_digits[(data / 1000) % 10];
            P4OUT &= ~BIT3;
            P4OUT |= (BIT0 | BIT1 | BIT2);
            break;
        }
    }
    else if (seg_state == 1) {
        switch (cnt) {
        case 0:
            P3OUT = digits[(data % 10)];
            P4OUT &= ~BIT0;
            P4OUT |= (BIT1 | BIT2 | BIT3);
            break;

        case 1:
            P3OUT = digits[(data / 10) % 10];
            P4OUT &= ~BIT1;
            P4OUT |= (BIT0 | BIT2 | BIT3);
            break;

        case 2:
            P3OUT = dot_digits[(data / 100) % 10];
            P4OUT &= ~BIT2;
            P4OUT |= (BIT0 | BIT1 | BIT3);
            break;

        case 3:
            P3OUT = digits[(data / 1000) % 10];
            P4OUT &= ~BIT3;
            P4OUT |= (BIT0 | BIT1 | BIT2);
            break;
        }
    }
    else if (seg_state == 2) {
        switch (cnt) {
        case 0:
            P3OUT = digits[(data % 10)];
            P4OUT &= ~BIT0;
            P4OUT |= (BIT1 | BIT2 | BIT3);
            break;

        case 1:
            P3OUT = dot_digits[(data / 10) % 10];
            P4OUT &= ~BIT1;
            P4OUT |= (BIT0 | BIT2 | BIT3);
            break;

        case 2:
            P3OUT = digits[(data / 100) % 10];
            P4OUT &= ~BIT2;
            P4OUT |= (BIT0 | BIT1 | BIT3);
            break;

        case 3:
            P3OUT = digits[(data / 1000) % 10];
            P4OUT &= ~BIT3;
            P4OUT |= (BIT0 | BIT1 | BIT2);
            break;
        }
    }
    else if (seg_state == 3) {
        switch (cnt) {
        case 0:
            P3OUT = dot_digits[(data % 10)];
            P4OUT &= ~BIT0;
            P4OUT |= (BIT1 | BIT2 | BIT3);
            break;

        case 1:
            P3OUT = digits[(data / 10) % 10];
            P4OUT &= ~BIT1;
            P4OUT |= (BIT0 | BIT2 | BIT3);
            break;

        case 2:
            P3OUT = digits[(data / 100) % 10];
            P4OUT &= ~BIT2;
            P4OUT |= (BIT0 | BIT1 | BIT3);
            break;

        case 3:
            P3OUT = digits[(data / 1000) % 10];
            P4OUT &= ~BIT3;
            P4OUT |= (BIT0 | BIT1 | BIT2);
            break;
        }
    }
}

