/* instruction-level simulator */
#include "Method.h"
#include <stdlib.h>
#include <stdio.h>
using namespace std;
#define NUMMEMORY 65536 /* maximum number of words in memory 16-bit instruction */
#define NUMREGS 8       /* number of machine registers มี register 8 ตัว */
#define MAXLINELENGTH 1000

typedef struct stateStruct
{ //ห้ามแก้ส่วนนี้
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory; //ตัวนับจำนวนบรรทัดในไฟล์ machine code
} stateType;

void printState(stateType *);
void setInitialize(stateStruct *);
int convertNum(int num);
string convertDec_Bi(int decimal);
string convert_negative(string str);
void action(string str, stateType *state);
int convert_dec(string str);
int offset_dec(string str);
string halt;
int main(int argc, char *argv[])
{
    char line[MAXLINELENGTH]; //แถวในชุดคำสั่ง
    stateType state;
    stateType *stateX;
    //บรรทัดนี้ allocate memory สำหรับpointer ถ้าไม่ทำจะเกิด Segmentation fault (core dumped)
    stateX = (stateType *)(malloc(sizeof(stateType)));
    FILE *filePtr;
    // ใช้ 2 ตัว --> ตัวมัน กับ ตัวไฟล์ที่จะอ่านเข้ามา(Machine code จากข้อแรก )
    if (argc != 2)
    {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }

    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL)
    {
        printf("error: can't open file %s", argv[1]);
        perror("fopen");
        exit(1);
    }

    //Initialize reg first
    setInitialize(stateX);

    /* read in the entire machine-code file into memory */
    for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL;
         state.numMemory++)
    {
        if (sscanf(line, "%d", state.mem + state.numMemory) != 1)
        {
            printf("error in reading address %d\n", state.numMemory);
            exit(1);
        }
        printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]);
        //เช็คค่าใน memory ของ stateX
        stateX->mem[state.numMemory] = state.mem[state.numMemory];
        stateX->numMemory++;
    }
    string str;
    stateX->pc = 0;
    int count = 1;
    while (stateX->pc < stateX->numMemory)
    {
        //Call printState 1 ครั้ง ก่อน instruction executes
        printState(stateX);
        str = convertDec_Bi(state.mem[stateX->pc]);
        //    substr ตัวแรกคือเลือกตำแหน่งแรกที่จะเอา ตัวสองคือความยาวที่ต้องการจะใช้
        action(str, stateX);
        //ถ้าเครื่องมีการ halt เกิดขึ้นให้ printState แล้วก็จบการทำงาน
        if (halt == "halted")
        {
            //Call printState 1 ครั้ง ก่อน simulator exits.
            stateX->pc++;
            printf("total of %d instructions executed\n", count);
            printf("Finish!!!!!!!!!!\n");
            printState(stateX);
            exit(0);
        }
        //Go to next instruction
        stateX->pc++;
        count++;
    }
    return (0);
}

void action(string str, stateType *stateX)
{
    //add instruction
    string opcode = str.substr(7, 3);
    int a = convert_dec(str.substr(10, 3));
    int b = convert_dec(str.substr(13, 3));
    int dest = convert_dec(str.substr(29, 3));
    int offset = offset_dec(str.substr(16, 16));
    //printf("a= %d b = %d\n dest = %d offset = %d", a, b, dest, offset);
    if (opcode == "000")
    {
        //     destReg   =  regA + regB
        stateX->reg[dest] = stateX->reg[a] + stateX->reg[b];
    }
    //nand instruction
    else if (opcode == "001")
    {
        // destReg = regA (nand) regB
        stateX->reg[dest] = ~(stateX->reg[a] & stateX->reg[b]);
    }
    else if (opcode == "010")
    {
        // regB = Memory[regA + offsetField]

        stateX->reg[b] = stateX->mem[stateX->reg[a] + offset];
    }
    else if (opcode == "011")
    {
        //Memory[regA + offsetField] = regB
        stateX->mem[stateX->reg[a] + offset] = stateX->reg[b];
        //ถ้าเกิดมีการนำค่ามาเพิ่มในmemด้วยที่ตำแหน่งนั้นยังไม่เคยใช้มาก่อน ให้เพิ่มnumMemory เพื่อให้printStateทราบว่ามีการใช้memตัวใหม่

        if ((stateX->numMemory - 1) < (stateX->reg[a] + offset))
        {
            stateX->numMemory++;
        }
    }
    else if (opcode == "100")
    {
        // regA == regB
        if (stateX->reg[a] == stateX->reg[b])
        {
            stateX->pc = stateX->pc + offset;
        }
        else
        {
            //Do nothing
        }
    }
    else if (opcode == "101")
    {
        if (a == b)
        {
            // regA และ regB คือregister ตัวเดียวกัน ให้เก็บ PC+1 ก่อน
            stateX->reg[a] = stateX->pc + 1;
            //jump to pc+1
            stateX->pc = stateX->pc + 1;
        }

        else
        {
            //เก็บค่า pc+1 ใน regB
            stateX->reg[b] = stateX->pc + 1;
            //กระโดดไปที่ address ที่ถูกเก็บไว้ในregA
            stateX->pc = stateX->reg[a] - 1;
        }
    }
    else if (opcode == "110")
    {
        halt = "halted";
        printf("machine halted\n");
    }
    else if (opcode == "111")
    {
        //Do nothing
    }
}

int offset_dec(string str)
{
    int sum = 0;
    int check;
    //if input is negative integer
    if (str[0] == '1')
    {
        str = twoComplement(str, 16);
        check = 1;
    }
    else
    {
        //Do nothing
    }
    for (int i = 0; i < 16; i++)
    {
        //แปลง char เป็น int
        sum += (str[i] - '0') * pow(2, 15 - i);
    }
    //return negative integer
    if (check == 1)
    {
        return 0 - sum;
    }
    //return positive integer
    else
    {
        return sum;
    }
}

int convert_dec(string str)
{
    if (str == "001")
    {
        return 1;
    }
    else if (str == "010")
    {
        return 2;
    }
    else if (str == "011")
    {
        return 3;
    }
    else if (str == "100")
    {
        return 4;
    }
    else if (str == "101")
    {
        return 5;
    }
    else if (str == "110")
    {
        return 6;
    }
    else if (str == "111")
    {
        return 7;
    }
    else
    {
        return 0;
    }
}

string convertDec_Bi(int dec)
{
    int tmp[32];
    string s;
    if (dec > 0)
    {
        for (int i = 32; i > 0; i--)
        {
            tmp[i] = dec % 2;
            dec /= 2;
            s = to_string(tmp[i]) + s;
        }
        return s;
    }
    else
    {
        string tmp = to_string(dec);
        string t = tmp.substr(1, tmp.length());
        t = convertDec_Bi(atoi(t.c_str()));
        t = twoComplement(t, 32);
        return t;
    }
}

void setInitialize(stateStruct *state)
{
    int i = 0;
    while (i < 8)
    {
        state->reg[i] = 0;
        i++;
    }
}

void printState(stateType *statePtr) //ห้ามแก้ส่วนนี้
{
    int i;
    printf("\n@@@\nstate:\n");
    printf("\tpc %d\n", statePtr->pc);
    printf("\tmemory:\n");
    for (i = 0; i < statePtr->numMemory; i++)
    {
        printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]);
    }
    printf("\tregisters:\n");
    for (i = 0; i < NUMREGS; i++)
    {
        printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
    }
    printf("end state\n");
}
