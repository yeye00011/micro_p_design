#include <msp430.h> 

//완
//sw 1.1를 누른 후 엔코더를 살짝 돌려주어야 한다.

unsigned int digits[10] = { 0xDB, 0x50, 0x1F, 0x5D, 0xD4, 0xCD, 0xCF, 0xD8, 0xDF, 0xDD };
unsigned int cnt = 0;
int keyout = 0;
unsigned int password = 0;
unsigned int in_pw = 0;
unsigned int seg1234 = 0; // 자릿수
unsigned int data[4] = { 0, 0, 0, 0 };
unsigned int i = 0;
int state = 0;
int set = 0;
int a = 0;

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    P1DIR &= ~BIT1;  // SW 1.1
    P1OUT |= BIT1;
    P1REN |= BIT1;

    P2DIR &= ~BIT1;  // SW 2.1
    P2OUT |= BIT1;
    P2REN |= BIT1;
    P2IE |= BIT1;
    P2IES |= BIT1;
    P2IFG &= ~BIT1;

    P1DIR |= BIT0;  // LED 1.0
    P1OUT &= ~BIT0;

    P4DIR |= BIT7;  // LED 1.0
    P4OUT &= ~BIT7;


    P3OUT &= 0x0000;
    P3DIR |= 0xffff;
    P4OUT &= ~0x0001;
    P4DIR |= 0x000f;


    //P1 interrupt P1.3 A, P1.2 B
    P1IE |= BIT3;
    P1IES |= BIT3; //falling edge select
    P1IFG &= ~BIT3;
    P1IE |= BIT2;
    P1IES |= BIT2; //falling edge select
    P1IFG &= ~BIT2;

    TA0CTL = TASSEL_2 + MC_1 + TACLR; // timer for int. segment
    TA0CCTL0 = CCIE;
    TA0CCR0 = 6000;

    //keypad
    P2DIR |= (BIT0 | BIT2 | BIT3); //output
    P2OUT |= (BIT0 | BIT2 | BIT3); //high
    P6REN |= (BIT3 | BIT4 | BIT5 | BIT6); //input
    P6OUT |= (BIT3 | BIT4 | BIT5 | BIT6); //pull up

    __bis_SR_register(GIE);

    while (1) {
        switch (state) {
        case 0:
            break;
        case 1:
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
            else if ((P6IN & BIT4) == 0) {   // shift
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
                seg1234 = 0;

                i = 0; // 4자리 overflow 방지
                while (i < 4) {
                    data[i] = 0;
                    i++;
                }
            }

            //logic 4자리 숫자를 받는 과정
            if (seg1234 == 0) {
                data[3] = keyout;
            }
            else if (seg1234 == 1) {
                data[2] = keyout;
            }
            else if (seg1234 == 2) {
                data[1] = keyout;
            }
            else if (seg1234 == 3) {
                data[0] = keyout;
            }
            else if (seg1234 == 4) { //4자리숫자로 변환하는 과정
                seg1234 = 0;
                in_pw = data[3] * 1000 + data[2] * 100 + data[1] * 10 + data[0] * 1;

                if (in_pw == password) {
                    state = 2;
                }

                i = 0;
                while (i < 4) { // 4자리 overflow 방지
                    data[i] = 0;
                    i++;
                }
            }
            break;

        case 2: // 대기상태

            break;
        }
    }
}
#pragma vector=PORT2_VECTOR
__interrupt void port2(void) {
    if (P2IFG & BIT1 == BIT1) {
        if (state == 2) {
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
            }
        }
        P2IFG &= ~BIT1;
    }
}
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void) {
    P1OUT |= BIT0;
    a = 0;
    P2IE &= ~BIT1;
}
#pragma vector=PORT1_VECTOR
__interrupt void port1(void) {
    if (P1IFG & BIT1) { // SW 1.1 interrupt
        P1IFG &= ~BIT2;
        P1IFG &= ~BIT3;

        P1IE &= ~BIT2;
        P1IE &= ~BIT3;
        password = keyout;

        state = 1;
        keyout = 0;
        set = 1;

        P1IFG &= ~BIT1;
    }

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
    if (keyout > 9999) {
        keyout = 0;
    }
    else if (keyout < 0) {
        keyout = 9999;
    }

}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void) {
    cnt++;
    if (cnt > 3) {
        cnt = 0;
    }

    if (set == 0) {
        switch (cnt) {
        case 0:
            P3OUT = digits[keyout % 10];
            P4OUT &= ~BIT0;
            P4OUT |= (BIT1 | BIT2 | BIT3);
            break;

        case 1:
            P3OUT = digits[(keyout / 10) % 10];
            P4OUT &= ~BIT1;
            P4OUT |= (BIT0 | BIT2 | BIT3);
            break;

        case 2:
            P3OUT = digits[(keyout / 100) % 10];
            P4OUT &= ~BIT2;
            P4OUT |= (BIT0 | BIT1 | BIT3);
            break;

        case 3:
            P3OUT = digits[(keyout / 1000) % 10];
            P4OUT &= ~BIT3;
            P4OUT |= (BIT0 | BIT1 | BIT2);
            break;
        }
    }
    else {
        switch (cnt) {
        case 0:
            P3OUT = digits[data[0]];
            P4OUT &= ~BIT0;
            P4OUT |= (BIT1 | BIT2 | BIT3);
            break;

        case 1:
            P3OUT = digits[data[1]];
            P4OUT &= ~BIT1;
            P4OUT |= (BIT0 | BIT2 | BIT3);
            break;

        case 2:
            P3OUT = digits[data[2]];
            P4OUT &= ~BIT2;
            P4OUT |= (BIT0 | BIT1 | BIT3);
            break;

        case 3:
            P3OUT = digits[data[3]];
            P4OUT &= ~BIT3;
            P4OUT |= (BIT0 | BIT1 | BIT2);
            break;
        }
    }

}
