
 /*

   32171550 박다은

   Mobile Processor
   Single-cycle MIPS
   project #2

 */

#include<stdio.h>
#include<errno.h>

//Declare clare 10MB memory structure
//memory has code and data
int Memory[0x1000000/4];

//program counter(or instruction pointer)
int PC = 0; // PC값은 0에서부터 시작

//cpu registers
int Regs[32];

//count
int inst_count = 1;
int r_count = 0;
int i_count = 0;
int j_count = 0;
int m_count = 0;
int b_count = 0;

//control (instruction [31-26] -> opcode)
int RegDst, Jump, Branch, MemRead, MemtoReg, ALUOp, MemWrite, ALUSrc, RegWrite;


void control(int op, int fun, int c_rs, int c_rt); //contol type별 구분
void count(int op); //type별 instruction 개수 count
void reverse(FILE* fp); //1byte씩 뒤집혀있는 instruction 원래대로 뒤집기
void PrintfE(int op, int fun, int p_rs, int p_rt, int p_v1, int p_v2, int p_v3);//execute후 결과 출력함수


int main(int argc, char* argv[]){
        FILE *fp = NULL;
        int inst = 0;
        //int inst_count = 1;

        Regs[31] = 0xffffffff; //RA 초기화
        Regs[29] = 0x100000; //SP 초기화
	Regs[15] = 0x40;//fib.bin에서 jal->jalr과정 때문에
	
        if(argc == 2) //2번째 자리에 파일명 입력하면 그 파일을 읽음
                fp = fopen(argv[1], "rb");// "rb" : 읽기모드 + 이진파일모드
        if(fp == NULL){ //입력하지 않았거나 잘못입력했으면 오류표시
                perror("no such input file"); //오류메세지 출력함수
                return 0; //프로그램 종료
        }

        reverse(fp); //뒤집혀있는 instruction 원래대로
	fclose(fp); //파일 닫기

        //We've read all the input program!
        while(1){

		//PC가 0xffff:ffff가 되면 cycle 종료
                if(PC == 0xffffffff) {
                        printf("\n PC : 0x%8x\n", PC);
                        printf(" <Halt program>\n\n");
                        break;
                }
		printf("\n\ninstruction %d-----------PC : 0x%x-------------\n",
                                 inst_count, PC);

		//1.fetch -> 2.decode -> 3.execution / ALU -> 4.Mem -> 5.WB


                //1.fetch----------------------------------------------------
                //getting an instruction from memory
                //divide by 4 to get the index into the array
                inst = Memory[PC/4];

                printf("[F] 0x%08x\n", inst);

		//instruntion이 0x00000000이면 남은 while문 실행X.
                if(inst == 0) {
                        printf(" none\n");
                        PC = PC + 4;
                        inst_count++;
                        continue;
                }

                //2. decode--------------------------------------------------
                //identify the instruction to execute
                int opcode = (inst & 0xfc000000) >> 26;
                int rs     = (inst & 0x03e00000) >> 21;
                int rt     = (inst & 0x001f0000) >> 16;
                int rd     = (inst & 0x0000f800) >> 11;
                int shamt  = (inst & 0x000007c0) >> 6;
                int func   = (inst & 0x0000003f) >> 0;
                int imm    = (inst & 0x0000ffff) >> 0;
                int addr   = (inst & 0x03ffffff) >> 0;
                int simm   = (imm >> 15) ?
                                (0xffff0000 | imm) : imm;
                int zimm   = imm;
                int Br_Addr= simm << 2;/*(imm >> 15) ?
                                (0xfffc0000 | imm << 2) : (imm << 2);*/
                int J_Addr = ((PC+4 & 0xf0000000) | addr) << 2;

                printf("[D] opcode: 0x%x, rs: %d, rt: %d, rd: %d, shamt:%d, func: 0x%x\n"
                        , opcode, rs, rt, rd, shamt, func);
                printf("        simm 0x%08x (%d)\n", simm, simm);

                count(opcode); //opcode를 구분해 instruction type별 개수 세기
		control(opcode, func, rs, rt); //control type별 구분

		//3. execute-------------------------------------------------
                //run the ALU
                int v1, v2, ALU_result;
                int bcond = 0; //branch taken or not taken 구분

		//ALU에 사용할 v1, v2초기화
                v1 = Regs[rs];

                if(ALUSrc) v2 = simm; //ALUSrc = 1이면 v2 = simm
                else v2 = Regs[rt]; //0이면 v2 = R[rt]

		//zimm or imm or shamt 사용하는 것들 예외처리
                if(opcode == 0xc || opcode == 0xd)
                        v2 = zimm;
                if(opcode == 0xf)
                        v2 = imm;
                if(opcode == 0 && (func == 0x0 || func == 0x2))
                        v1 = shamt;

                switch(ALUOp){
                        case 0x100: //j, jal, jalr, jr
                                break;
                        case 0x20: //add //+sw, lw
                        case 0x21: //addu
                        case 0x8: //addi
                        case 0x9: //addiu
                                ALU_result = v1 + v2;
                                break;
                        case 0x24: //and
                        case 0xc: //andi
                                ALU_result = v1 & v2;
                                break;
                        case 0xf: //lui
                                ALU_result = v2 << 16;
                                break;
                        case 0x27: //nor
                                ALU_result = ~(v1 | v2);
                                break;
                        case 0x25: //or
                        case 0xd: //ori
                                ALU_result = v1 | v2;
                                break;
                        case 0x2a: //slt
                        case 0x2b: //sltu
                        case 0xa: //slti
                                ALU_result = (v1 < v2) ? 1:0;
                                break;
                        case 0x2: //srl
                                ALU_result = v2 >> v1;
                                break;
                        case 0x22: //sub
                        case 0x23: //subu
                                ALU_result = v1 - v2;
                                break;
                        case 0x4: //beq
                                if(v1 == v2)
                                        bcond = 1;
                                break;
                        case 0x5: //bne
                                if(v1 != v2)
                                        bcond = 1;
                                break;
			case 0x0: //sll
                                ALU_result = v2 << v1;
                                break;
                        default: //잘못된 instruction 명령 시 프로그램 종료
                                printf("\twrong insrtuction\n");
                                return 0;
                }

		//Excute 단계 printf
                PrintfE(opcode, func, rs, rt, v1, v2, ALU_result);


		//4. memory--------------------------------------------------
                //access memory
                //addr
                //MemRead
                //value = Memory[(addr>>2)];

                //lw
                int Address;
                if(MemRead){ //메모리 가져오기
                        Address = Memory[ALU_result];
                        printf("[M] Read Memory\n");
                        printf("        0x%x <= M[0x%x]\n",
                                        Address, ALU_result);
                }

                //sw
                if(MemWrite){ //메모리 저장하기
                        Memory[ALU_result] = Regs[rt];
                        printf("[M] Write Memory\n");
                        printf("        M[0x%x] <= R[%d](0x%x)\n",
                                        ALU_result, rt, Regs[rt]);
                }


                //5. write back----------------------------------------------
                //update register values
		int WriteR;
		if(RegDst) WriteR = rd; //RegDst = 1이면 WriteR = instruction[15-11](rd)
                else WriteR = rt; //0이면 WriteR = instruction[20-16](rt)

                if(RegWrite){
                        if(MemtoReg){ //lw
                                Regs[WriteR] = Address;
                                printf("[W] R[%d] = 0x%x\n",
                                        WriteR, Address);
                        }
                        else{
                                Regs[WriteR] = ALU_result;
                                printf("[W] R[%d] = 0x%x\n",
                                        WriteR, ALU_result);
                        }
                }//----------------------------------------------------------


                //Update PC
                if(Jump){
                        if(opcode == 0 && func == 0x8) //jr
                                PC = Regs[rs];
			else if(opcode == 0 && func == 0x9){ //jalr
				Regs[rd] = PC + 8; //rd : 31
				PC = Regs[rs];
			}
                        else if(opcode == 0x2) //j
                                PC = J_Addr;
                        else if(opcode == 0x3){ //jal
                                Regs[31] = PC + 8;
                                PC = J_Addr;
                        }
                        printf("Jump to 0x%x\n", PC);
                }
                else if(Branch && bcond){ //branch taken
                        PC = PC + 4 + Br_Addr;
                        printf("Jump to 0x%x\n", PC);
			b_count++;
                }
                else PC = PC + 4;

		//changed architectural state
		//register, PC, memory가 변했을 시 출력
		printf("\n");
		printf("changed PC      : PC     = 0x%x\n", PC);
		if(RegWrite)
			printf("changed register: R[%d]  = 0x%x\n", WriteR, Regs[WriteR]);	
		if(MemWrite)
			printf("changed memory  : M[0x%x]= 0x%x\n", ALU_result, Regs[rt]);
        }

        printf("\nfinal return value(R[2])             : %d\n", Regs[2]);
        printf("number of excuted instructions       : %d\n", inst_count-1);
        printf("number of R-type instructions        : %d\n", r_count);
        printf("number of I-type instructions        : %d\n", i_count);
        printf("number of J-type instructions        : %d\n", j_count);
        printf("number of memory access instructions : %d\n", m_count);
        printf("number of taken branches             : %d\n\n\n", b_count);

        return 0;
}

//type 별 control 구분
void control(int op, int fun, int c_rs, int c_rt){

	RegDst = 0;
        Jump = 0;
        Branch = 0;
        MemRead = 0;
        MemtoReg = 0;
        ALUOp = 0x100;
        MemWrite = 0;
        ALUSrc = 0;
        RegWrite = 0;

	if(op == 0){
		if(fun == 0x8 || fun == 0x9) //jr, jalr
			Jump = 1;
		else{ //R_type
			RegDst = 1;
			ALUOp = fun;
			RegWrite = 1;
                }
	}
	else if(op == 0x2 || op == 0x3) //jal, jr
		Jump = 1;
       	else if(op == 0x23){ //lw
                MemRead = 1;
                MemtoReg = 1;
                ALUOp = 0x20;
                ALUSrc = 1;
                RegWrite = 1;
	}
       	else if(op == 0x2b){ //sw
                ALUOp = 0x20;
                MemWrite = 1;
                ALUSrc = 1;
        }
        else if(op == 0x4 || op == 0x5){ //beq, bne
		Branch = 1;
		ALUOp = op;
        }
        else { //I_type
                ALUOp = op;
                ALUSrc = 1;
		RegWrite = 1;
	}
	return;
}

//1byte씩 뒤집혀있는 instruction 원래대로 뒤집기
void reverse(FILE *f){
        int rev_inst = 0;
        int index = 0;
        int data = 0;
        size_t ret = 0;
        while(1){
                int h1 = 0;
                int h2 = 0;
                int h3 = 0;
                int h4 = 0;
                int rev_inst = 0;

                //reading file to eof
                ret = fread(&data, sizeof(data), 1, f);
                if(ret == 0) break;

                //printout
                printf("orig 0x%08x    ", data);

                h1 = ((data & 0xff) << 24);
                h2 = ((data & 0xff00) << 8);
                h3 = ((data & 0xff0000) >> 8) & 0xff00;
                h4 = ((data & 0xff000000) >> 24) & 0xff;

                rev_inst = h1 | h2 | h3 | h4;

                Memory[index/4] = rev_inst;
                printf("[0x%08x] 0x%08x\n",
                                index, Memory[index/4]);
                index = index + 4;
        }
        return;
}

//type별 instruction 개수 count
void count(int op){
	inst_count++;
        if(op == 0) r_count++; //r_type
        else if(op == 0x2 || op == 0x3) j_count++; //j_type
        else if(op == 0x23 || op ==0x2b){ //lw. sw(i_type)
                i_count++;
                m_count++;
        }
        else i_count++; //i_type

        return ;
}

//execute 후 결과 출력함수
void PrintfE(int op, int fun, int p_rs, int p_rt, int p_v1, int p_v2, int p_v3){

        if(op == 0){ //R_type
                if(fun == 0x20 || fun == 0x21) //add, addu
                        printf("[E] result(0x%x) = R[%d](0x%x) + R[%d](0x%x)\n",
                                p_v3, p_rs, p_v1, p_rt, p_v2);

                else if(fun == 0x24) //and
                        printf("[E] result(0x%x)(%d) = R[%d](0x%x) & R[%d](0x%x)\n",
				p_v3, p_v3, p_rs, p_v1, p_rt, p_v2);
                else if(fun == 0x27) //nor
                        printf("[E] result(0x%x)(%d) = ~(R[%d](0x%x) | R[%d](0x%x))\n",
                                p_v3, p_v3, p_rs, p_v1, p_rt, p_v2);
                else if(fun == 0x25) //or
                        printf("[E] result(0x%x)(%d) = (R[%d](0x%x) | R[%d](0x%x))\n",
                                 p_v3, p_v3, p_rs, p_v1, p_rt, p_v2);
                else if(fun == 0x2a || fun == 0x2b) //slt, sltu
                        printf("[E] result(0x%x)(%d) = (R[%d](0x%x) < R[%d](0x%x)) ? 1 : 0\n",
                                p_v3, p_v3, p_rs, p_v1, p_rt, p_v2);
                else if(fun == 0x0) //sll
                        printf("[E] result(0x%x)(%d) = R[%d](0x%x) << 0x%x(%d)\n",
                                p_v3, p_v3, p_rt, p_v2, p_v1, p_v1);
                else if(fun == 0x2) //srl
                        printf("[E] result(0x%x)(%d) = R[%d](0x%x) >> 0x%x(%d)\n",
                                p_v3, p_v3, p_rt, p_v2, p_v1, p_v1);
                else if(fun == 0x22 || fun == 0x23) //sub, subu
                        printf("[E] result(0x%x)(%d) = R[%d](0x%x) - R[%d](0x%x)\n",
                                p_v3, p_v3, p_rs, p_v1, p_rt, p_v2);
        }
        else if(op == 0x8 || op == 0x9 || op == 0x23 || op == 0x2b)//addi, addiu, lw, sw
                printf("[E] result(0x%x) = R[%d](0x%x) + simm(0x%x)\n",
                                p_v3, p_rs, p_v1, p_v2);
        else if(op == 0xc) //andi
                printf("[E] result(0x%x) = R[%d](0x%x) & zimm(0x%x)\n",
                                p_v3, p_rs, p_v1, p_v2);
        else if(op == 0xf) //lui
                printf("[E] result(0x%x) = {imm(0x%x), 16b'0}\n",
                                p_v3, p_v2);
        else if(op == 0xd) //ori
                printf("[E] result(0x%x) = R[%d](0x%x) | zimm(0x%x)\n",
                                p_v3, p_rs, p_v1, p_v2);
	else if(op == 0xa) //slti
                printf("[E] result(0x%x)(%d) = R[%d](0x%x) < simm(0x%x)? 1 : 0\n",
                                p_v3, p_v3, p_rs, p_v1, p_v2);

        return;
}
