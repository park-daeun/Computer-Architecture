
/*

  32171550 박다은

  Mobile Processor
  Simple Calculation
  project #1 

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int Reg[10];//레지스터의 값을 저장하는 데 사용된다.

FILE *fp;

//getline()
ssize_t len;
char *line = 0;//getline()사용하려면 NULL로 초기화해야함.
size_t n;

int p_index = 0;//줄번호로 print되는 index.

int gcd(int a, int b);//최대공약수 찾기
void jump(int num);//Jump와 Branch에서 사용됨
int dis_operand(char *opr, int a);//operand 구별

struct operand{
	char *opr;
	int n;
	int index;
}opr1, opr2;//operand1과 operand2

int  main(int argc, char* argv[]){
	
        char *op;//opcode

        int result;
	
	//dis_operand함수를 호출할 때, opr1인지 opr2인지 구분위해 사용
	int first = 1;
	int second = 2;

	printf("\n\n\n>>>>>>>>>>>>>>>>>Hello World!<<<<<<<<<<<<<<<<<\n\n");

        if(argc == 2)//./hello input.txt
                fp = fopen(argv[1], "r");//"r" : 읽기모드
        else
                fp = fopen("input.txt", "r");

	 while(1) {
                len = getline(&line, &n, fp);
                if(len == -1)   break; //getline() line읽기 실패하면 -1리턴.
		
		//print the line
                p_index++;
                printf("\n[%d] %s", p_index, line);

		//parse the line
		//operator
                op = strtok(line, " \t\n");//분리자:space,tap,newline

		//Halt
                if(*op == 'H') {
			printf("\n\n>>>>>>>>>>>>>>>>>Halt Execution<<<<<<<<<<<<<<<<<\n\n\n");
			return 0;
		}

                printf("opcode: %s | ", op);

		//operand1
		opr1.opr = strtok(NULL, " \t\n");//두번째부터는 str->NULL
		opr1.n = dis_operand(opr1.opr,first);

		//operand2
                if(*op != 'J' && *op != 'B'){ 
                        opr2.opr = strtok(NULL, " \t\n");
			opr2.n = dis_operand(opr2.opr,second);
             	}

		//execution
		switch(*op){
                        case '+' :
                                result = opr1.n + opr2.n;
                                printf("Reslut) %x := %x + %x\n",
                                        result, opr1.n, opr2.n);
                                Reg[0] = result;
                                break;
                        case '-' :
                                result = opr1.n - opr2.n;
                                printf("Result) %x := %x - %x\n",
                                        result, opr1.n, opr2.n);
                                Reg[0] = result;
                                break;
                        case '*' :
                                result = opr1.n * opr2.n;
                                printf("Result) %x := %x * %x\n",
                                        result, opr1.n, opr2.n);
                                Reg[0] = result;
                                break;
                        case '/' :
                                result = opr1.n / opr2.n;
                                printf("Result) %x := %x / %x\n",
                                        result, opr1.n, opr2.n);
                                Reg[0] = result;
                                break;
			case 'G' :
                                result = gcd(opr1.n, opr2.n);
                                printf("Result) %x = gcd(%x, %x)\n"
                                        , result, opr1.n, opr2.n);
                                Reg[0] = result;
                                break;
                        case 'C' :
                                if(opr1.n >= opr2.n){
                                        result = 0;
                                        printf("Result(R0)) %d\n", result);
                                }else{
                                        result = 1;
                                        printf("Result(R0)) %d\n", result);
                                }
                                Reg[0] = result;
                                break;
			//결과값을 R0에 저장한다.

                        case 'M' :
                                Reg[opr1.index] = Reg[opr2.index];
                                printf("Result) R%d(%x) = R%d\n",
                                opr1.index, Reg[opr2.index], opr2.index);
                                break;
                        case 'J' :
                                jump(opr1.n);
                                break;
                        case 'B' :
                                if(Reg[0] == 0) printf("\n->Continue\n");
                                if(Reg[0] == 1) jump(opr1.n);
                                break;
		}
	}
        free(line);//동적으로 할당되었던 메모리 시스템에 반납
        fclose(fp);//파일(input.txt) 닫기

        return 0;
}

int gcd(int a, int b){
        while( a != b){
                if(a> b)
                        a -= b;
                else
                        b -= a;
                }
        return a;
}

void jump(int num){
        printf("\n->Jump to line%d\n", num);
        if(num > p_index){
                for(; num >  p_index+1 ; p_index++)
                getline(&line, &n, fp);
        }
        else{
                free(line);
                line = NULL;//getline()두번째사용부터는 str(line)->NULL이므로
                fclose(fp);

                fp = fopen("input.txt", "r");
                for(int i = 1; i < num; i++){
                        getline(&line, &n, fp);
                }
        }
}

int dis_operand(char *oprnd, int a){
	int num;
		//J or B
                if(*oprnd >= '1' && *oprnd <= '9'){
                        num = atoi(oprnd);
			if(a == 1)  printf("n_opr1: %d ", num);
			//else if(a == 2) printf("n_opr2: %d\n", num);
			return num;
                } 
		//operand의 prefix가 0x인 16진수일 때
		else if(*(oprnd+1) == 'x' || *(oprnd+1) == 'X'){
			num = (int)strtol(oprnd+2,NULL,16);
			if(a == 1)  printf("n_opr1: 0x%x | ", num);
                        else if(a == 2) printf("n_opr2: 0x%x\n", num);
                        return num;
		} 
		//operand가 Register일 때
		else if(*oprnd == 'R' || *oprnd == 'r'){
				 if(a == 1){
					opr1.index = atoi(oprnd+1);
					num = Reg[opr1.index];
					 printf("R_opr1: R%d (val: %x) | " , opr1.index, num);
					return Reg[opr1.index];
				} else if(a == 2){
					opr2.index = atoi(oprnd+1);
					num = Reg[opr2.index];
					printf("R_opr2: R%d (val: %x)\n", opr2.index, num);
					return Reg[opr2.index];
	       			}
                        } else return 0; //error mal-formatted number
}
