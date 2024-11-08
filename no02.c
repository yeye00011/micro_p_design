#include <msp430.h>


/*
    DDDDD
   P     E
   P     E
    CCCCC
   B     G
   B     G
    AAAAA  F

0: 0xDB
1: 0X50
2: 0X1F
3: 0X5D
4: 0XD4
5: 0XCD
6: 0XCF
7: 0XD8
8: 0XDF
9: 0XDD
dot : 0b 0010 0000, 0x20

 */

unsigned int digits[10] = { 0xDB, 0x50, 0x1F, 0x5D, 0xD4, 0xCD, 0xCF, 0xD8, 0xDF, 0xDD };
unsigned int dot_digits[10] = { 0xFB, 0x70, 0x3F, 0x8D, 0xF4, 0xED, 0xEF, 0xF8, 0xFF, 0xFD };
int cnt = 0;
int segout1 = 0;
int segout2 = 0;
int state = 0;
int reset = 0;
unsigned int tgap = 0; //time gap
unsigned int sonic_out = 0;
int sonic_state = 0;
int timer_state = 0;
int timer1_refresh = 1;
unsigned int pwm_val1 = 0;
unsigned int pwm_val2 = 0;
unsigned int encoder_data = 0;
unsigned int encoder_data_comp = 0;
unsigned int encoder_data_comp1 = 0;



void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    P3OUT &= 0x0000;
    P3DIR |= 0xffff;
    P4OUT &= ~0x0001;
    P4DIR |= 0x000f;

    P1DIR |= BIT0;  //PORT DIR
    P4DIR |= BIT7;

    P1OUT &= ~BIT0; //LED OFF
    P4OUT &= ~BIT7;

    P2OUT |= BIT1;
    P2REN |= BIT1;
    P2IE |= BIT1;
    P2IFG &= ~BIT1;

    TA0CTL = TASSEL_2 + MC_1 + TACLR; // for dynamic segment
    TA0CCTL0 = CCIE;
    TA0CCR0 = 6000;

    P2DIR |= (BIT5 | BIT4); // PWM
    P2SEL |= (BIT5 | BIT4);

    TA2CTL = TASSEL_2 + MC_1; // timer for PWM speed
    TA2CCR0 = 1000;
    TA2CCTL2 = OUTMOD_6;
    TA2CCR2 = 0;
    TA2CCTL1 = OUTMOD_6;
    TA2CCR1 = 0;

    //keypad
    P2DIR |= (BIT0 | BIT2 | BIT3); //output
    P2OUT |= (BIT0 | BIT2 | BIT3); //high
    P6REN |= (BIT3 | BIT4 | BIT5 | BIT6); //input
    P6OUT |= (BIT3 | BIT4 | BIT5 | BIT6); //pull up

    P6SEL |= BIT0; // 가변저항의 전압 범위 : 약 0.9 ~ 3.7 //SEL |= 1 : ADC사용
    ADC12CTL0 = ADC12SHT02 + ADC12MSC + ADC12ON; // Sample and Hold Time(4, 8, 16, 32, 64),
    ADC12CTL1 = ADC12SHP + ADC12CONSEQ_2;
    ADC12MCTL0 = ADC12INCH_0;
    ADC12CTL0 |= ADC12ENC;
    ADC12CTL0 |= ADC12SC;

    // Ultrasonic wave sensor
    P2OUT &= ~BIT7;
    P2DIR |= BIT7;
    P1IE |= BIT4;
    P1IFG &= ~BIT4;
    P1IES &= ~BIT4; //rising edge

    // Encoder
    P1IE |= BIT3;
    P1IES |= BIT3; //falling edge select
    P1IFG &= ~BIT3;
    P1IE |= BIT2;
    P1IES |= BIT2; //falling edge select
    P1IFG &= ~BIT2;




    __bis_SR_register(GIE);


    while (1) {
        switch (state) {
        case 0:

            cnt = 0;
            segout1 = 0;
            segout2 = 0;
            reset = 0;
            tgap = 0; //time gap
            sonic_out = 0;
            sonic_state = 0;
            timer_state = 0;
            timer1_refresh = 1;
            pwm_val1 = 0;
            pwm_val2 = 0;
            encoder_data = 0;
            encoder_data_comp = 0;
            encoder_data_comp1 = 0;

            P1IE |= BIT4;


            break;
        case 1:
            //data = ADC12MEM0/1.5 - 730; 이 방식은 정확하지 않다고 생각함
            //(4095 - 1100) / 21 = 143 씩 매핑
            if (ADC12MEM0 < 1243) {
                segout1 = 0; // 앞자리
                segout2 = 0; // 뒷자리
            }
            else if (ADC12MEM0 < 1386) {
                segout1 = 0; // 앞자리
                segout2 = 1; // 뒷자리
            }
            else if (ADC12MEM0 < 1536) {
                segout1 = 0; // 앞자리
                segout2 = 2; // 뒷자리
            }
            else if (ADC12MEM0 < 1679) {
                segout1 = 0; // 앞자리
                segout2 = 3; // 뒷자리
            }
            else if (ADC12MEM0 < 1822) {
                segout1 = 0; // 앞자리
                segout2 = 4; // 뒷자리
            }
            else if (ADC12MEM0 < 1965) {
                segout1 = 0; // 앞자리
                segout2 = 5; // 뒷자리
            }
            else if (ADC12MEM0 < 2108) {
                segout1 = 0; // 앞자리
                segout2 = 6; // 뒷자리
            }
            else if (ADC12MEM0 < 2251) {
                segout1 = 0; // 앞자리
                segout2 = 7; // 뒷자리
            }
            else if (ADC12MEM0 < 2394) {
                segout1 = 0; // 앞자리
                segout2 = 8; // 뒷자리
            }
            else if (ADC12MEM0 < 2537) {
                segout1 = 0; // 앞자리
                segout2 = 9; // 뒷자리
            }
            else if (ADC12MEM0 < 2680) {
                segout1 = 1; // 앞자리
                segout2 = 0; // 뒷자리
            }
            else if (ADC12MEM0 < 2823) {
                segout1 = 1; // 앞자리
                segout2 = 1; // 뒷자리
            }
            else if (ADC12MEM0 < 2966) {
                segout1 = 1; // 앞자리
                segout2 = 2; // 뒷자리
            }
            else if (ADC12MEM0 < 3109) {
                segout1 = 1; // 앞자리
                segout2 = 3; // 뒷자리
            }
            else if (ADC12MEM0 < 3252) {
                segout1 = 1; // 앞자리
                segout2 = 4; // 뒷자리
            }
            else if (ADC12MEM0 < 3395) {
                segout1 = 1; // 앞자리
                segout2 = 5; // 뒷자리
            }
            else if (ADC12MEM0 < 3538) {
                segout1 = 1; // 앞자리
                segout2 = 6; // 뒷자리
            }
            else if (ADC12MEM0 < 3681) {
                segout1 = 1; // 앞자리
                segout2 = 7; // 뒷자리
            }
            else if (ADC12MEM0 < 3824) {
                segout1 = 1; // 앞자리
                segout2 = 8; // 뒷자리
            }
            else if (ADC12MEM0 < 3967) {
                segout1 = 1; // 앞자리
                segout2 = 9; // 뒷자리
            }
            else {
                segout1 = 2; // 앞자리
                segout2 = 0; // 뒷자리
            }
            break;

        case 2:
            TA1CTL = TASSEL_1 + MC_1 + TACLR; // for blinking LED
            TA1CCTL0 = CCIE;
            TA1CCR0 = segout1 * 32767 + segout2 * 3277; // 32768로 하면 overflow 남
            state = 3;
            break;

        case 3:
            break;

        case 4:
            if (TA2CCR1 == 1000 && TA2CCR2 == 1000) {
                TA2CCR1 = 0;
                TA2CCR2 = 0;
            }
            P2OUT &= ~BIT2;           // *
            P2OUT |= (BIT0 | BIT3);
            if ((P6IN & BIT4) == 0) {
                timer_state = 1;
                TA1CTL = TASSEL_1 + MC_1; // for making 0.1sec
                TA1CCR0 = 3276;
            }
            else {
                TA1CTL = MC_0;
            }

            P2OUT &= ~BIT3;           // #
            P2OUT |= (BIT0 | BIT2);
            if ((P6IN & BIT4) == 0) {
                timer_state = 2;
                TA1CTL = TASSEL_1 + MC_1; // for making 0.1sec
                TA1CCR0 = 3276;
            }
            else {
                TA1CTL = MC_0;
            }

            break;

        case 5:
            timer_state = 3;
            if (timer1_refresh == 1) {
                TA1CTL = TASSEL_2 + MC_1 + TACLR; // timer for Ult sonic wave sensor
                TA1CCTL0 = CCIE;
                TA1CCR0 = 50; // 50us
                timer1_refresh = 0;
            }
            else {
                if (sonic_state == 0) {
                    P2OUT |= BIT7; //shoot start
                    __delay_cycles(10); // 10us
                    P2OUT &= ~BIT7; //shoot stop
                    sonic_state = 1;
                }
            }
            break;

        case 6:
            TA1CTL = MC_0;
            TA2CCR1 = pwm_val1;
            TA2CCR2 = pwm_val2;

            state = 7;
            break;

        case 7:
            encoder_data_comp1 = encoder_data;
            __delay_cycles(500000);  //0.5sec
            encoder_data_comp = encoder_data - encoder_data_comp1;
            if (encoder_data_comp <= 2680) { // 0.5s 이내에 4바퀴이상 못돌면 방해라고 판단 -> 670 * 4 = 2680
                TA2CCR1 = 0;
                TA2CCR2 = 0;
            }
            break;
        }
    }
}

#pragma vector=PORT1_VECTOR
__interrupt void port1(void) {
    if (P1IFG & BIT3) {
        encoder_data++;
        P1IFG &= ~BIT3;
    }
    if (P1IFG & BIT2) {
        encoder_data++;
        P1IFG &= ~BIT2;
    }

    if (P1IFG & BIT4) {
        if ((P1IES & BIT4) == 0) { // if rising edge
            tgap = 0;
            P1IES |= BIT4;  // falling edge
        }
        else if (P1IES & BIT4) { // if falling edge
            if ((tgap > 3) && (tgap < 500)) { // 150us ~ 25ms (timer1 isr이 50us마다 실행되기 때문)
                sonic_out = (tgap * 50) / 58;
                if (sonic_out < 10) {
                    TA2CCR1 = 0;
                    TA2CCR2 = 0;
                }
                else {
                    TA2CCR1 = pwm_val1;
                    TA2CCR2 = pwm_val2;
                }
            }
            else if (tgap >= 760) { //38ms에 50을 나누면 760us 즉 38ms 넘어가면 측정할수없는 먼 값이니 9999출력
                sonic_out = 9999;
            }
            else {
                sonic_out = 0;
            }
            P1IES &= ~BIT4;  // rising edge
        }
        P1IFG &= ~BIT4;
    }
}


#pragma vector=PORT2_VECTOR
__interrupt void port2(void) {
    while (P2IN & BIT1 == 0) { // chattering avoidance
    }
    while (P2IN & BIT1 == 0) { // chattering avoidance
    }
    while (P2IN & BIT1 == 0) { // chattering avoidance
    }
    if (state == 0) {
        state = 1;
    }
    else if (state == 1) {
        state = 2;
    }
    else if (state == 3) {
        P1OUT &= ~BIT0;
        P4OUT &= ~BIT7;
        TA1CTL = MC_0;
        state = 4;
    }
    else if (state == 4) {
        pwm_val1 = TA2CCR1;
        pwm_val2 = TA2CCR2;
        state = 5;
    }
    else if (state == 5) {
        state = 6;
    }
    else if (state == 7) {
        state = 0;
    }
    P2IFG &= ~BIT1;
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void) {
    if (timer_state == 0) {
        if (reset == 0)
        {
            P4OUT |= BIT7;
            P1OUT &= ~BIT0;
            reset = 1;
        }
        else
        {
            P1OUT |= BIT0;
            P4OUT &= ~BIT7;
            reset = 0;
        }
    }
    else if (timer_state == 1) {
        if (TA2CCR1 == 0) {
            TA2CCR1 += 300;
        }
        else if (TA2CCR1 < 1000) {
            TA2CCR1 += 100;
        }
        else if (TA2CCR1 >= 1000) {
            TA2CCR1 = 1000;
        }
    }
    else if (timer_state == 2) {
        if (TA2CCR2 == 0) {
            TA2CCR2 += 300;
        }
        else if (TA2CCR2 < 1000) {
            TA2CCR2 += 100;
        }
        else if (TA2CCR2 >= 1000) {
            TA2CCR2 = 1000;
        }
    }
    else if (timer_state == 3) {
        tgap++;
        if ((sonic_state == 1) && (tgap > 1000)) { // 전체 cycle이 50ms이어야 하므로 50ms/50us = 1000
            sonic_state = 0;
        }
    }
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void) {

    cnt++;
    if (cnt > 3) {
        cnt = 0;
    }

    if (state >= 1 && state < 5) {
        switch (cnt) {
        case 0:
            P3OUT = digits[segout2];
            P4OUT &= ~BIT0;
            P4OUT |= (BIT1 | BIT2 | BIT3);
            break;

        case 1:
            P3OUT = dot_digits[segout1];
            P4OUT &= ~BIT1;
            P4OUT |= (BIT0 | BIT2 | BIT3);
            break;

        case 2:
            P3OUT = 0;
            P4OUT &= ~BIT2;
            P4OUT |= (BIT0 | BIT1 | BIT3);
            break;

        case 3:
            P3OUT = 0;
            P4OUT &= ~BIT3;
            P4OUT |= (BIT0 | BIT1 | BIT2);
            break;
        }
    }
    else if (state >= 5) {
        switch (cnt) {
        case 0:
            P3OUT = digits[sonic_out % 10];
            P4OUT &= ~BIT0;
            P4OUT |= (BIT1 | BIT2 | BIT3);
            break;

        case 1:
            P3OUT = digits[(sonic_out / 10) % 10];
            P4OUT &= ~BIT1;
            P4OUT |= (BIT0 | BIT2 | BIT3);
            break;

        case 2:
            P3OUT = digits[(sonic_out / 100) % 10];
            P4OUT &= ~BIT2;
            P4OUT |= (BIT0 | BIT1 | BIT3);
            break;

        case 3:
            P3OUT = digits[(sonic_out / 1000) % 10];
            P4OUT &= ~BIT3;
            P4OUT |= (BIT0 | BIT1 | BIT2);
            break;
        }
    }
}
