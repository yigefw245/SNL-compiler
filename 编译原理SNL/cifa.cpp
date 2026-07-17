#include "cifa.h"
#include"adv.h"
FILE* listing2;
const char* typeall1[] =
{
    "ENDFILE1", "ERROR1",
    /* 保留字 */
    "PROGRAM", "PROCEDURE", "TYPE", "VAR", "IF",
    "THEN", "ELSE", "FI", "WHILE", "DO",
    "ENDWH", "BEGIN", "END", "READ", "WRITE",
    "ARRAY", "OF", "RECORD", "RETURN",
    //类型
    "INTEGER", "CHAR1",
    /* 多字符单词符号 */
    "ID", "INTC", "CHARC",
    /*特殊符号 */
    "ASSIGN", "EQ", "LT", "PLUS", "MINUS",
    "TIMES", "OVER", "LPAREN", "RPAREN", "DOT",
    "COLON", "SEMI", "COMMA", "LMIDPAREN", "RMIDPAREN",
    "UNDERANGE"
};
/* 从输入缓冲区lineBuf中取得下一个非空字符, 如果lineBuf中的字串已经读完,则从源代码文件中读入一新行 */
int getNextChar(void)
{
    //当前代码输入行缓冲器lineBuf已经耗尽 
    if (!(linepos < bufsize))
    {
        //lineno1++; //行号+1
        if (fgets(lineBuf, BUFLEN - 1, source))
        {
            bufsize = strlen(lineBuf);  //取得当前输入源代码行的实际长度,送给变量bufsize
            linepos = 0;  //当前字符位置linepos指向lineBuf开始位置
            return lineBuf[linepos++];  //取得输入行缓冲区lineBuf中下一字符
        }
        else
        {
            EOF_flag = 1;   //未能成功读入新的代码行,fget函数返回值为NULL,已经到源代码文件末尾
            return EOF;
        }
    }
    //行输入缓冲区lineBuf中字符还未读完
    else return lineBuf[linepos++];
}

/* 用于超前读字符后不匹配时候的回退 */
void ungetNextChar(void)
{
    if (!EOF_flag)   //不处于文件末尾
        linepos--;
}

/* 将链表中的Token结点依次存入文件中 */
void ChainToFile(ChainNodeType* Chainhead)
{
    int num = 1;
    FILE* fp;
    FILE* fp2;
    ChainNodeType* currentP = Chainhead;
    fp = fopen("Tokenlist.txt", "ab");   //按追加方式打开文件
    fp2 = fopen("Tokenlist2.txt", "ab");
    if (fp == NULL)
    {
        printf("cannot open file Tokenlist!\n");
        exit(0);
    }
    if (fp2 == NULL)
    {
        printf("cannot open file Tokenlist2!\n");
        exit(0);
    }
    //依次将所有的Token写入文件
    do
    {
        fwrite(currentP, TOKENLEN, 1, fp);//将二进制数据写入文件
        fprintf(fp2, "%d", currentP->Token.lineshow);
        fprintf(fp2, " %s", typeall1[currentP->Token.Lex]);
        fprintf(fp2, " %s", currentP->Token.Sem);
        fputs("\n", fp2);
        currentP = currentP->nextToken;
        num++;
    } while (currentP != NULL);

    fclose(fp);  //关闭文件
    fclose(fp2);
}

/*保留字查找函数:标识符如果在保留字表中则返回相应单词,否则返回单词ID*/
LexType  reservedLookup(char* s)
{
    int i;
    for (i = 0; i < MAXRESERVED; i++)
        if (!strcmp(s, reservedWords[i].str))
            return(reservedWords[i].tok);//字符串s与保留字表中某一表项匹配,函数返回对应保留字单词
    return ID;//字符串s未在保留字表中找到,函数返回标识符单词ID
}


/*取得单词函数: 函数从源文件字符串序列中获取下一个单词符号.使用确定性有限自动机DFA,采用直接转向法 */
void  getTokenList1()
{

    source = fopen("text.txt", "r");//调试用的

    ChainNodeType* chainHead;    //链表的表头指针
    ChainNodeType* currentNode;  //指向处理当前Token的当前结点
    ChainNodeType* preNode;      //指向当前结点的前驱结点
    ChainNodeType* p1;           //临时指针，用于释放链表部分
    Token1 currentToken;      //存放当前的Token

    chainHead = preNode = currentNode = (ChainNodeType*)malloc(CHAINNODELEN);  //链表的第一个结点
    currentNode->nextToken = NULL;  //初始化当前节点

    do
    {
        int tokenStringIndex = 0;
        StateType state = START;  //当前状态标志state,以START开始

        int save;  //决定当前识别字符是否存入当前识别单词词元存储区tokenString

        //DFA
        while (state != DONE)
        {
            int c = getNextChar();
            save = 1;

            switch (state)
            {
                /* 当前DFA状态state为开始状态START,DFA处于当前单词开始位置 */
            case START:
                if (c >= 48 && c <= 57)   //数字
                    state = INNUM;
                else if ((c >= 65 && c <= 90) || (c >= 97 && c <= 122))  //字母
                    state = INID;
                else if (c == ':')
                {
                    save = 0;
                    state = INASSIGN;
                }
                else if (c == '.')
                {
                    save = 0;
                    state = INRANGE;
                }

                else if (c == '\'')
                {
                    save = 0;
                    state = INCHAR;
                }
                else if ((c == ' ' || c == '\r' || c == '\t'))  //空白字符
                {
                    save = 0;
                    state = START;
                }
                else if (c == '\n')
                {
                    save = 0;
                    lineno1++;
                }

                else if (c == '{')  //注释
                {
                    save = 0;
                    state = INCOMMENT;
                }

                else  // 当前字符c为其它字符,当前DFA状态state设置为完成状态DONE  
                {
                    state = DONE;
                    save = 0;
                    switch (c)
                    {
                        //文件结束EOF
                    case EOF:
                        currentToken.Lex = ENDFILE1;
                        break;
                        //单目运算符
                    case '=':
                        currentToken.Lex = EQ;
                        break;

                    case '<':
                        currentToken.Lex = LT;
                        break;
                    case '+':
                        currentToken.Lex = PLUS;
                        break;

                    case '-':
                        currentToken.Lex = MINUS;
                        break;

                    case '*':
                        currentToken.Lex = TIMES;
                        break;

                    case '/':
                        currentToken.Lex = OVER;
                        break;

                    case '(':
                        currentToken.Lex = LPAREN;
                        break;

                    case ')':
                        currentToken.Lex = RPAREN;
                        break;

                    case ';':
                        currentToken.Lex = SEMI;
                        break;

                    case ',':
                        currentToken.Lex = COMMA;
                        break;

                    case '[':
                        currentToken.Lex = LMIDPAREN;
                        break;

                    case ']':
                        currentToken.Lex = RMIDPAREN;
                        break;

                        // 当前字符c为其它字符,当前识别单词返回值currentToken设置为错误单词ERROR1 
                    default:
                        currentToken.Lex = ERROR1;
                        Error = true;
                        printf("%d行出错", lineno1);
                        exit(1);
                        break;
                    }
                }
                break;
                /********** 当前状态为开始状态START的处理结束 **********/

                /* 当前DFA状态state为注释状态INCOMMENT,确定性有限自动机DFA处于注释位置 */
            case INCOMMENT:
                save = 0;  //注释中内容不生成单词,无需存储
                //当前字符c为EOF
                if (c == EOF)
                {
                    state = DONE;
                    currentToken.Lex = ENDFILE1;
                }
                // 当前字符c为"}",注释结束.当前DFA状态state设置为开始状态START 
                else if (c == '}')
                    state = START;
                break;

                /* 当前DFA状态state为赋值状态INASSIGN,确定性有限自动机DFA处于赋值单词位置 */
            case INASSIGN:
                state = DONE;
                save = 0;
                //双字符分节符":="
                if (c == '=')
                {
                    currentToken.Lex = ASSIGN;
                    state = DONE; //调试 
                }

                // ":"后不是"=",识别为":"COLON，回退一个字符  
                else
                {
                    ungetNextChar();
                    currentToken.Lex = COLON;
                }
                break;

                /* 当前DFA状态state为数组下标界限状态INRANGE */
            case INRANGE:
                state = DONE;
                save = 0;
                // ".."下标界UNDERANGE 
                if (c == '.')
                    currentToken.Lex = UNDERANGE;

                // "."程序结束标志DOT 
                else
                {
                    ungetNextChar();
                    currentToken.Lex = DOT;
                }
                break;

                /* 当前DFA状态state为数字状态INNUM,确定性有限自动机处于数字单词位置 */
            case INNUM:
                //不是数字,回退一个字符，state设置为DONE,数字单词识别完成
                if (!(c >= 48 && c <= 57))
                {
                    ungetNextChar();
                    save = 0;
                    state = DONE;
                    currentToken.Lex = INTC;
                }
                break;

                /* 当前DFA状态state为字符标志状态INCHAR */
            case INCHAR:
                // 当前为字母或数字
                if ((c >= 48 && c <= 57) || (c >= 65 && c <= 90) || (c >= 97 && c <= 122))
                {
                    int c1 = getNextChar();
                    if (c1 == '\'')  //‘字符’
                    {
                        save = 1;
                        state = DONE;
                        currentToken.Lex = CHARC;
                    }
                    else  //‘后无’ ，出错
                    {
                        ungetNextChar();
                        ungetNextChar();

                        state = DONE;
                        currentToken.Lex = ERROR1;
                        printf("%d行出错",lineno1);
                        save = 0;
                        Error = true;
                    }
                }
                else //其他符号（'后不是字母数字） 出错
                {
                    ungetNextChar();

                    state = DONE;
                    currentToken.Lex = ERROR1;
                    printf("%d行出错", lineno1);
                    save = 0; //调试
                    Error = true;
                }
                break;

                /* 当前DFA状态state为标识符状态INID,确定性有限自动机DFA处于标识符单词位置 */
            case INID:
                //不是数字字母 
                if (!((c >= 48 && c <= 57) || (c >= 65 && c <= 90) || (c >= 97 && c <= 122)))
                {
                    ungetNextChar();
                    save = 0;
                    state = DONE;
                    currentToken.Lex = ID;
                }
                break;

                /* 当前DFA状态state为完成状态DONE,确定性有限自动机DFA处于单词结束位置 */
            case DONE:	break;

                /* 当前DFA状态state为其它状态,此种情况不应发生 */
            default:
                state = DONE;
                currentToken.Lex = ERROR1;
                printf("%d行出错", lineno1);
                save = 0; //调试
                Error = true;
                break;

            }
            /*************** 分类判断处理结束 *******************/

            // 当前字符存储状态save为1,且当前正识别单词已经识别部分未超过单词最大长度,将当前字符c写入tokenString	
            if ((save) && (tokenStringIndex <= MAXTOKENLEN))
                tokenString[tokenStringIndex++] = (char)c;

            if (state == DONE)
            {
                tokenString[tokenStringIndex] = '\0';  //DONE 单词识别完，tokenString加上结束标志
                if (currentToken.Lex == ID)
                {
                    currentToken.Lex = reservedLookup(tokenString);    //标识符进一步判断是标识符or保留字
                }
            }
        }

        currentToken.lineshow = lineno1;
        strcpy(currentToken.Sem, tokenString);
        //currentToken存入链表
        (currentNode->Token).lineshow = currentToken.lineshow;
        (currentNode->Token).Lex = currentToken.Lex;
        strcpy((currentNode->Token).Sem, currentToken.Sem);

        //若不是第一个结点，则将当前结点连入链表
        if (preNode != currentNode)
        {
            preNode->nextToken = currentNode;
            preNode = currentNode;
        }
        //申请一个新的结点
        currentNode = (ChainNodeType*)malloc(CHAINNODELEN);
        currentNode->nextToken = NULL;  //初始化
    } while ((currentToken.Lex) != ENDFILE1);  //处理完所有的Token

    ChainToFile(chainHead);  //将Token链表存入文件中

    //释放链表
    while (chainHead != NULL)
    {
        p1 = chainHead->nextToken;
        free(chainHead);
        chainHead = p1;
    }

    /* 词法分析函数结束 */
}

int CIFA()
{
    lineno1 = 1;
    linepos = 0;
    bufsize = 0;
    EOF_flag = 0;
    Error = false;

    listing = fopen("Tokenlist.txt", "wb+");  //创建tokenlist文件
    listing2 = fopen("Tokenlist2.txt", "wb+");
    if (listing == NULL || listing2 == NULL)
    {
        printf("文件创建失败\n");
        exit(0);
    }

    getTokenList1();  //词法分析
    if (Error == true)
        return 1;
    return 0;
}