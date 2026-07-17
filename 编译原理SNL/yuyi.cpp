#include"yuyi.h"
int initOff = 7;//第二层永远都是从7开始，干脆直接定义成7
int Off;
int Level = -1;
TypeIR* intPtr = nullptr;			//该指针一直指向整数类型的内部表示
TypeIR* charPtr = nullptr;		//该指针一直指向字符类型的内部表示
TypeIR* boolPtr = nullptr;		//该指针一直指向布尔类型的内部表示
SymbTable* Scope[1000];
bool ERROR_sematicas = false;
string NodeKind_string[] = { "ProK", "PheadK", "TypeK", "VarK", "ProcDecK", "StmLK", "DecK", "StmtK", "ExpK" }; //记录语法树节点类型
void CreateTable()//创建一个符号表
{
	Off = initOff;
	Level++;//level就对应符号表的个数
	Scope[Level] = nullptr;//遇到第一个元素的时候，指向它
}
SymbTable* NewTable()
{
	SymbTable* p = new SymbTable;
	p->attrIR.kind = typeKind;
	p->attrIR.idtype = nullptr;
	p->next = nullptr;
	p->attrIR.More.VarAttr.isParam = false;
	return p;
}
void DestroyTable()//撤销一个符号表
{
	Level--;
}
bool Enter(char* Id, AttributeIR* AttribP, SymbTable** Entry)//登记标记符和属性到符号表
{
	bool present = false;
	SymbTable* p = NewTable();
	p->next = nullptr;
	SymbTable* p1 = Scope[Level];
	SymbTable* p2 = p1;
	while (p1 != nullptr)
	{
		p2 = p1;
		if (strcmp(p1->idname, Id) == 0)//找到了
		{
			printf("%s 重复声明错误\n", Id);
			ERROR_sematicas = true;
			present = true;
			return present;
		}
		else
			p1 = p1->next;//全部搜一遍当前符号表，检查有没有声明
	}
	if (p1 == nullptr)//没找到
	{
		if (p2 == nullptr)Scope[Level] = p;
		else p2->next = p;
	}
	else
	{
		printf("%s 重复声明错误\n", Id);
		ERROR_sematicas = true;
		present = true;
		return present;
	}
	//登记属性
	strcpy(p->idname, Id);
	p->attrIR.idtype = AttribP->idtype;
	p->attrIR.kind = AttribP->kind;
	switch (AttribP->kind)
	{
	case  typeKind: break;
	case  varKind:
		p->attrIR.More.VarAttr.level = AttribP->More.VarAttr.level;
		p->attrIR.More.VarAttr.off = AttribP->More.VarAttr.off;
		p->attrIR.More.VarAttr.access = AttribP->More.VarAttr.access;
		break;
	case  procKind:
		p->attrIR.More.ProcAttr.level = AttribP->More.ProcAttr.level;
		p->attrIR.More.ProcAttr.param = AttribP->More.ProcAttr.param;
		break;
	default:break;
	}
	(*Entry) = p;
	return present;
}
bool FindEntry(char* Id, SymbTable** Entry)
{
	bool present = false;
	int lev = Level;
	SymbTable* findentry = Scope[lev];
	for (int i = Level; i >= 0; i--)
	{
		SymbTable* p = Scope[i];
		while (p != nullptr)
		{
			if (strcmp(p->idname, Id) == 0)//找到了
			{
				(*Entry) = p;
				return true;
			}
			else p = p->next;
		}
	}
	(*Entry) = nullptr;
	return false;
}
bool FindField(char* Id, FieldChain* head, FieldChain** Entry)
{
	FieldChain* p = head;
	while (p != nullptr)
	{
		if (strcmp(p->idname, Id) == 0)//找到了
		{
			if (Entry != nullptr)(*Entry) = p;
			return true;
		}
		else
			p = p->next;
	}
	if (p == nullptr)//没找到
	{
		return false;
	}
}
void PrintSymbTable()
{
	int level = 0;
	while (Scope[level] != nullptr)
	{
		SymbTable* t = Scope[level];
		cout << endl;
		for (int ii = 0; ii < level; ii++)
			cout << "--";
		cout << "-->第 " << level << " 层符号表:\n";
		while (t != nullptr)
		{
			for (int ii = 0; ii < level; ii++)
				cout << "----";
			cout << "| " << "标识符：" << setw(4) << t->idname << "|"; // 控制标识符的位宽

			AttributeIR* Attrib = &(t->attrIR);

			// 输出标识符的类型
			if (Attrib->idtype != nullptr)
			{
				switch (Attrib->idtype->kind)
				{
				case intTy: cout << " int     |"; break;
				case charTy: cout << " char    |"; break;
				case arrayTy: cout << " array   |"; break;
				case recordTy: cout << " record  |"; break;
				default: cout << " unknown |";
				}
			}
			else
			{
				cout << "         |";
			}
			// 输出标识符的类别和其他属性
			switch (Attrib->kind)
			{
			case typeKind:
				cout << " (Type)        |"; break;
			case varKind:
				cout << " (Variable)    |";
				cout << " 层级：" << setw(5) << Attrib->More.VarAttr.level << " |";
				cout << " 偏移：" << setw(5) << Attrib->More.VarAttr.off << " |";
				cout << " 访问方式：" << setw(10) << (Attrib->More.VarAttr.access == dir ? "direct" : "indirect") << " |";
				break;
			case procKind:
				cout << " (Function)    |";
				cout << " 层级：" << setw(5) << Attrib->More.ProcAttr.level << " |";
				break;
			default:
				cout << " (unknown kind)|";
			}

			cout << "\n";
			t = t->next;
		}
		level++;
	}
}

//语义分析--------------------------
void Analyze(TreeNode* t)
{
	if (t == nullptr)return;
	CreateTable();
	initialize();
	//循环处理主程序的声明部分
	TreeNode* p = t->child[1];
	while (p != nullptr)
	{
		switch (p->nodekind)
		{
		case TypeK:
			TypeDecPart(p->child[0]);
			break;
		case VarK:
			VarDecPart(p->child[0]);
			break;
		case ProcDecK:
			ProcDecPart(p);
			break;
		default:
			ERROR_sematicas = true;
			cout << t->lineno << " " << NodeKind_string[p->nodekind] << "在语法树中没有这种类型的节点" << endl;
			break;
		}
		p = p->Sibling;
	}

	//处理主程序体
	t = t->child[2];
	if (t->nodekind == StmLK)
	{
		TreeNode* p = t->child[0];
		while (p != nullptr)
		{
			statement(p);//循环处理语句
			p = p->Sibling;
		}
	}

	if (Level != -1)DestroyTable();
	if (ERROR_sematicas == false)
		printf("没有发生语义错误\n");
	else {
		exit(1);
	}
	printf("\n打印符号表：\n");
	PrintSymbTable();
}
void initialize()
{
	//初始化布尔、整数、字符类型
	intPtr = new TypeIR;
	intPtr->kind = intTy;
	intPtr->size = 1;
	charPtr = new TypeIR;
	charPtr->kind = charTy;
	charPtr->size = 1;
	boolPtr = new TypeIR;
	boolPtr->size = 1;
	boolPtr->kind = boolTy;
	for (int i = 0; i < 1000; i++)Scope[i] = nullptr;
}
TypeIR* TypeProcess(TreeNode* t, Dec deckind)
{
	TypeIR* Ptr = nullptr;
	switch (deckind)
	{
	case ArrayK:
		Ptr = arrayType(t);
		break;
	case CharK:
		Ptr = charPtr;
		break;
	case IntegerK:
		Ptr = intPtr;
		break;
	case RecordK:
		Ptr = recordType(t);
		break;
	case IdK:
		Ptr = nameType(t);
		break;
	default:
		break;
	}
	return Ptr;
}
TypeIR* nameType(TreeNode* t)//查找定义的类型名
{
	TypeIR* Ptr = nullptr;
	SymbTable* entry = nullptr;
	bool present = FindEntry(t->type_name, &entry);
	if (present == true)
	{
		if (entry->attrIR.kind == typeKind)
		{
			Ptr = entry->attrIR.idtype;
		}
		else
		{
			printf("%d %s非类型标识符错误\n", t->lineno, t->type_name);
			ERROR_sematicas = true;
		}
	}
	else
	{
		printf("%d %s 类型名没有声明\n", t->lineno, t->type_name);
		ERROR_sematicas = true;
	}
	return Ptr;
}
TypeIR* arrayType(TreeNode* t)//数组类型内部表示处理函数
{
	TypeIR* Ptr = nullptr;
	if (t->attr.ArrayAttr.low > t->attr.ArrayAttr.up)
	{
		printf("%d 数组越界错误\n", t->lineno);
		ERROR_sematicas = true;
	}
	else
	{
		;//调用类型分析函数处理下标类型和元素类型，创建新的类型信息表，计算类型长度
		TypeIR* Ptr0 = TypeProcess(t, IntegerK);
		TypeIR* Ptr1 = TypeProcess(t, t->attr.ArrayAttr.childType);
		Ptr = NewTy(arrayTy);
		Ptr->size = (t->attr.ArrayAttr.up - t->attr.ArrayAttr.low + 1) * (Ptr1->size);
		Ptr->More.ArrayAttr.indexTy = Ptr0;//下标类型
		Ptr->More.ArrayAttr.elemTy = Ptr1;
		Ptr->More.ArrayAttr.low = t->attr.ArrayAttr.low;
		Ptr->More.ArrayAttr.up = t->attr.ArrayAttr.up;
	}
	return Ptr;
}
TypeIR* recordType(TreeNode* t)//处理记录类型的内部表示函数
{
	TypeIR* Ptr = NewTy(recordTy);
	t = t->child[0];
	FieldChain* x2 = nullptr;
	FieldChain* x1 = nullptr;
	FieldChain* body = nullptr;
	int off = 0;
	while (t != nullptr)
	{
		for (int i = 0; i < t->idnum; i++)
		{
			//创建新的域类型单元结构指针后移
			x2 = new FieldChain;
			x2->next = nullptr;
			x2->offset = 0;
			x2->unitType = nullptr;
			if (body == nullptr)
				body = x1 = x2;
			strcpy(x2->idname, t->name[i]);
			x2->unitType = TypeProcess(t, t->kind.dec);
			if (x2 != x1)
			{
				x2->offset = x1->offset + x1->unitType->size;
				x1->next = x2;
				x1 = x2;
			}
		}
		t = t->Sibling;
	}
	Ptr->size = x2->offset + x2->unitType->size;
	//将域链链入记录类型的body部分
	Ptr->More.body = body;
	return Ptr;
}
void TypeDecPart(TreeNode* t)//类型声明部分分析处理函数
{
	bool present = false;
	AttributeIR attIr;
	attIr.kind = typeKind;
	//属性类型标记为typekind
	SymbTable* Entry = nullptr;
	while (t != nullptr)
	{
		present = Enter(t->name[0], &attIr, &Entry);//调用enter
		if (present != true)
		{
			Entry->attrIR.idtype = TypeProcess(t, t->kind.dec);
		}
		else
		{
			printf("%d %s 重复声明错误\n", t->lineno, t->name[0]);
			ERROR_sematicas = true;
			Entry = nullptr;
		}
		t = t->Sibling;
	}

}
void ProcDecPart(TreeNode* t)//过程声明部分分析处理函数
{
	SymbTable* Head = HeadProcess(t);//处理过程头
	TreeNode* temp = t;
	t = t->child[1];
	while (t != nullptr)
	{

		switch (t->nodekind)//类型声明，变量声明，过程声明
		{
		case  TypeK:     TypeDecPart(t->child[0]);  break;
		case  VarK:     VarDecPart(t->child[0]);   break;
		case  ProcDecK:  break;
		default:
			ERROR_sematicas = true;
			printf("%d %s 在语法树中没有这种类型的节点\n", t->lineno, t->nodekind);
			break;
		}
		if (t->nodekind == ProcDecK)break;
		else
			t = t->Sibling;

	}
	while (t != nullptr)//处理函数声明
	{
		ProcDecPart(t);
		t = t->Sibling;
	}
	;//处理过程体部分
	if (temp->child[2]->nodekind == StmLK)
	{
		TreeNode* p = temp->child[2]->child[0];
		while (p != nullptr)
		{
			statement(p);//处理语句
			p = p->Sibling;
		}
	}
	if (Level != -1)DestroyTable();
}
void VarDecPart(TreeNode* t)//处理变量声明的语义分析
{
	AttributeIR  attrIr;
	bool present = false;
	SymbTable* entry = nullptr;
	while (t != nullptr)
	{
		attrIr.kind = varKind;
		for (int i = 0; i < (t->idnum); i++)
		{
			attrIr.idtype = TypeProcess(t, t->kind.dec);

			//判断是值参还是变参
			if (t->attr.procAttr.paramt == varparamtype) //如果是变参
			{
				attrIr.More.VarAttr.access = indir;
				attrIr.More.VarAttr.level = Level;
				/*计算变参的偏移*/

				attrIr.More.VarAttr.off = Off;
				Off = Off + 1;
			}/*如果是变参，则偏移加1*/
			else  //如果是值参
			{
				attrIr.More.VarAttr.access = dir;
				attrIr.More.VarAttr.level = Level;
				/*计算值参的偏移*/
				if (attrIr.idtype != nullptr)
				{
					attrIr.More.VarAttr.off = Off;
					Off = Off + (attrIr.idtype->size);
				}
			}
			/*登记该变量的属性及名字,并返回其类型内部指针*/
			present = Enter(t->name[i], &attrIr, &entry);
			if (present != false)
				printf("%d %s 重复定义\n", t->lineno, t->name[i]), ERROR_sematicas = true;
			else
				t->table[i] = entry;
		}
		t = t->Sibling;
	}
}
SymbTable* HeadProcess(TreeNode* t)//处理函数头的语义分析
{
	SymbTable* entry = nullptr;
	AttributeIR attrIr;
	bool present = false;
	;//填写过程标识符属性，登记过程的符号表项，调用形参处理函数
	attrIr.kind = procKind;
	attrIr.idtype = nullptr;
	attrIr.More.ProcAttr.level = Level + 1;
	if (t != nullptr)
	{
		present = Enter(t->name[0], &attrIr, &entry);
		t->table[0] = entry;

	}
	entry->attrIR.More.ProcAttr.param = ParaDecList(t);//调用形参处理函数
	return entry;
}
ParamTable* ParaDecList(TreeNode* t)//过程形参处理函数
{
	TreeNode* p = nullptr;
	ParamTable* Ptr1 = nullptr;
	ParamTable* Ptr2 = nullptr;
	ParamTable* head = nullptr;

	if (t != nullptr)
	{
		if (t->child[0] != nullptr)
			p = t->child[0];   	//程序声明节点的第一个儿子节点
		CreateTable();			//进入新的局部化区
		Off = 7;                //子程序中的变量初始偏移设为7
		VarDecPart(p);			//变量声明部分
		SymbTable* Ptr0 = Scope[Level];

		while (Ptr0 != nullptr)         //只要不为空，就访问其兄弟节点
		{
			//构造形参符号表，并使其连接至符号表的param项
			Ptr2 = new paramTable;
			Ptr2->entry = nullptr;
			Ptr2->next = nullptr;
			if (head == nullptr)
				head = Ptr1 = Ptr2;
			Ptr2->entry = Ptr0;
			Ptr2->next = nullptr;

			if (Ptr2 != Ptr1)
			{
				Ptr1->next = Ptr2;
				Ptr1 = Ptr2;
			}
			Ptr0 = Ptr0->next;
		}
	}
	return head;   //返回形参符号表的头指针
}
void statement(TreeNode* t)//语句分析
{
	switch (t->kind.stmt)
	{
	case IfK:
		ifstatment(t);
		break;
	case ElseK:
		elsestatement(t);
		break;
	case FiK:
		fistatement(t);
		break;
	case WhileK:
		whilestatement(t);
		break;
	case EndwhK:
		endwhstatement(t);
		break;
	case  AssignK:
		assignstatemnet(t);
		break;
	case ReadK:
		readstatement(t);
		break;
	case WriteK:
		writestatement(t);
		break;
	case CallK:
		callstatement(t);
		break;
	case ReturnK:
		returnstatement(t);
		break;
	case EndK:
		endstatement(t);
		break;
	default:
		printf("%d 语句类型错误\n", t->lineno);
		break;
	}
}
TypeIR* Expr(TreeNode* t, AccessKind* Ekind)//表达式分析处理函数
{
	TypeIR* Ptr = nullptr;
	bool present = false;
	if (t->kind.exp == ConstK)
	{
		if (t->attr.ExpAttr.valc=="") {
			Ptr = TypeProcess(t, IntegerK);
			Ptr->kind = intTy;
		}
		else {
			Ptr = TypeProcess(t, CharK);
			Ptr->kind = charTy;
		}
		if (Ekind != nullptr)(*Ekind) = dir;
	}
	else if (t->kind.exp == OpK)
	{
		//递归处理孩子节点
		TypeIR* c0 = Expr(t->child[0], nullptr);
		if (c0 == nullptr)return nullptr;
		TypeIR* c1 = Expr(t->child[1], nullptr);
		if (c1 == nullptr)return nullptr;
		if (Compat(c0, c1) == true)//类型相容
		{
			switch (t->attr.ExpAttr.op)
			{
			case LT:
			case EQ:
				Ptr = boolPtr;
				break;  //条件表达式
			case PLUS:
			case MINUS:
			case TIMES:
			case OVER:
				Ptr = intPtr;
				break;  //算数表达式
			}
			if (Ekind != nullptr)
				(*Ekind) = dir; //直接变量
		}
		else
		{
			ERROR_sematicas = true;
			printf("%d 运算分量不匹配\n", t->lineno);
		}
	}
	else if (t->kind.exp == VariK)
	{
		if (t->child[0] == nullptr)
		{
			SymbTable* entry = nullptr;
			//在符号表中查找此标识符
			present = FindEntry(t->name[0], &entry);
			t->table[0] = entry;

			if (present != false) //在符号表中能找到
			{
				//id不是变量
				if (FindAttr(entry).kind != varKind)
				{
					ERROR_sematicas = true;
					printf("%d %s不是变量\n", t->lineno, t->name[0]);
					Ptr = nullptr;
				}
				else
				{
					Ptr = entry->attrIR.idtype;
					if (Ekind != nullptr)
						(*Ekind) = indir;  //间接变量
				}
			}
			else  //在符号表中找不到，标识符无声明
				printf("%d %s 无声明\n", t->lineno, t->name[0]), ERROR_sematicas = true;
		}
		else//Var = Var0[E]的情形
		{
			if (t->attr.ExpAttr.varkind == ArrayMembV)
				Ptr = arrayVar(t); //处理分析数组变量
			else //Var = Var0.id的情形
				if (t->attr.ExpAttr.varkind == FieldMembV)
					Ptr = recordVar(t); //处理分析域成员变量
		}
	}
	else
	{
		printf("%d 表达式异常\n", t->lineno);
	}
	return Ptr;
}
TypeIR* arrayVar(TreeNode* t)//处理数组变量的下标分析
{
	TypeIR* Ptr = nullptr;
	bool present = false;
	SymbTable* entry = nullptr;
	present = FindEntry(t->name[0], &entry);
	t->table[0] = entry;
	if (present == false)
	{
		ERROR_sematicas = true;
		printf("%d %s 无声明\n", t->lineno, t->name[0]);
	}
	else
	{
		if (FindAttr(entry).kind != varKind)
		{
			printf("%d %s 不是变量\n", t->lineno, t->name[0]);
			ERROR_sematicas = true;
			Ptr = nullptr;
		}
		else
		{
			if (FindAttr(entry).idtype != nullptr)
				if (FindAttr(entry).idtype->kind != arrayTy)
				{
					printf("%d %s 不是数组类型变量\n", t->lineno, t->name[0]);
					ERROR_sematicas = true;
					Ptr = nullptr;
				}
				else
				{
					;//如果下标类型不符报错
					TypeIR* Ptr0 = entry->attrIR.idtype->More.ArrayAttr.indexTy;
					if (Ptr0 == nullptr)
						return nullptr;
					typeIR* Ptr1 = Expr(t->child[0], nullptr);//intPtr;
					if (Ptr1 == nullptr)
						return nullptr;
					present = Compat(Ptr0, Ptr1);
					if (present != true)
					{
						printf("%d,类型与数组成员不匹配\n", t->lineno);
						ERROR_sematicas = true;
						Ptr = nullptr;
					}
					else
						Ptr = entry->attrIR.idtype->More.ArrayAttr.elemTy;
				}
		}
	}
	return Ptr;
}
TypeIR* recordVar(TreeNode* t)//记录变量域的分析
{
	TypeIR* Ptr = nullptr;
	bool present = false;
	SymbTable* entry = nullptr;
	present = FindEntry(t->name[0], &entry);
	t->table[0] = entry;
	FieldChain* currentP = nullptr;
	bool result = true;
	if (present == false)
	{
		printf("%d %s 未声明先使用错误\n", t->lineno, t->name[0]);
		ERROR_sematicas = true;
		Ptr = nullptr;
	}
	else
	{
		if (FindAttr(entry).kind != varKind)
		{
			printf("%d %s 不是变量\n", t->lineno, t->name[0]);
			ERROR_sematicas = true;
			Ptr = nullptr;
		}
		else
		{
			if (FindAttr(entry).idtype->kind != recordTy)
			{
				printf("%d %s 不是记录类型变量\n", t->lineno, t->name[0]);
				ERROR_sematicas = true;
			}
			else
			{
				;//如果id不是合法域名报错
				TypeIR* Ptr0 = entry->attrIR.idtype;
				currentP = Ptr0->More.body;
				while ((currentP != nullptr) && (result != false))
				{
					result = strcmp(t->child[0]->name[0], currentP->idname);
					//如果相等
					if (result == false)
						Ptr = currentP->unitType;
					else
						currentP = currentP->next;
				}
				if (currentP == nullptr)
					if (result == true)
					{
						ERROR_sematicas = true;
						printf("%d %s 不是域类型\n", t->child[0]->lineno, t->child[0]->name[0]);
						Ptr = nullptr;
					}
					else//如果id是数组变量
						if (t->child[0]->child[0] != nullptr)
							Ptr = arrayVar(t->child[0]);
			}
		}
	}
	return Ptr;
}
void assignstatemnet(TreeNode* t)
{
	//printf("\nassignstatement()\n");
	SymbTable* entry = nullptr;
	bool present = false;
	TypeIR* ptr = nullptr;
	TypeIR* Eptr = nullptr;//赋值号左端标识符类型
	TreeNode* child1 = t->child[0];
	TreeNode* child2 = t->child[1];
	if (child1->child[0] == nullptr)
	{
		//在符号表中查找此标识符
		present = FindEntry(child1->name[0], &entry);
		if (present != false)
		{   //id不是变量
			if (FindAttr(entry).kind != varKind)
			{
				ERROR_sematicas = true;
				printf("%d %s 不是变量\n", child1->lineno, child1->name[0]);
				Eptr = nullptr;
			}
			else
			{
				Eptr = entry->attrIR.idtype;
				child1->table[0] = entry;
			}
		}
		else //标识符无声明
			cout << child1->lineno << " " << child1->name[0] << " 无声明" << endl, ERROR_sematicas = true;
	}
	else
	{
		if (child1->attr.ExpAttr.varkind == ArrayMembV)
			Eptr = arrayVar(child1);
		else //Var0.id的情形
			if (child1->attr.ExpAttr.varkind == FieldMembV)
				Eptr = recordVar(child1);
	}//检查完了左侧
	if (Eptr != nullptr)
	{
		if ((t->nodekind == StmtK) && (t->kind.stmt == AssignK))
		{
			//检查是不是赋值号两侧 类型等价
			ptr = Expr(child2, nullptr);
			if (!Compat(ptr, Eptr))
			{
				printf("%d 赋值表达式错误\n", t->lineno);
				ERROR_sematicas = true;
			}
		}
	}
}
void callstatement(TreeNode* t)
{
	bool present = false;
	SymbTable* entry = nullptr;
	present = FindEntry(t->child[0]->name[0], &entry);
	t->child[0]->table[0] = entry;
	if (present == true)
	{
		;//判断ID
		;//处理形参与实参结合
		if (FindAttr(entry).kind != procKind)
			printf("%d %s 不是函数名\n", t->lineno, t->child[0]->name[0]), ERROR_sematicas = true;
		else
		{
			TreeNode* p = t->child[1];
			ParamTable* paramP = FindAttr(entry).More.ProcAttr.param;//paramP指向形参符号表的表头
			while ((p != nullptr) && (paramP != nullptr))
			{
				AccessKind Ekind;
				SymbTable* paraEntry = paramP->entry;
				TypeIR* Etp = Expr(p, &Ekind);//实参
				//参数类别不匹配
				if ((FindAttr(paraEntry).More.VarAttr.access == indir) && (Ekind == dir))
				{
					ERROR_sematicas = true;
					printf("%d 参数类别不匹配\n", t->lineno);
				}
				else
					//参数类型不匹配
					if ((FindAttr(paraEntry).idtype) != Etp)
					{
						ERROR_sematicas = true;
						printf("%d 参数类型不匹配\n", t->lineno);
					}
				p = p->Sibling;
				paramP = paramP->next;
			}
			//参数个数不匹配
			if (p != nullptr || paramP != nullptr)
			{
				ERROR_sematicas = true;
				printf("%d 参数个数不匹配\n", t->lineno);
			}
		}
	}
	else
	{
		ERROR_sematicas = true;
		printf("%d %s 函数无声明\n", t->lineno, t->child[0]->name[0]);
	}
}
void ifstatment(TreeNode* t)
{
	AccessKind* Ekind = nullptr;
	TypeIR* Etp = Expr(t->child[0], Ekind);
	if (Etp != nullptr)
		if (Etp->kind != boolTy)
		{
			ERROR_sematicas = true;
			printf("%d 条件表达式错误\n", t->lineno);
		}
		else
		{
			TreeNode* p = t->child[1];
			/*处理then语句序列部分*/
			while (p != nullptr)
			{
				statement(p);
				p = p->Sibling;
			}
			t = t->child[2]; //必有三儿子
			//处理else语句部分
			while (t != nullptr)
			{
				statement(t);
				t = t->Sibling;
			}
		}
}
void elsestatement(TreeNode*t) {

}
void fistatement(TreeNode* t) {

}
void whilestatement(TreeNode* t)
{
	TypeIR* Etp = Expr(t->child[0], nullptr);
	if (Etp != nullptr)
		if (Etp->kind != boolTy)
		{
			ERROR_sematicas = true;
			printf("%d 条件表达式错误\n", t->lineno);
		}
		else
		{
			t = t->child[1];
			while (t != nullptr) //处理循环部分
			{
				statement(t);
				t = t->Sibling;
			}
		}
}
void endwhstatement(TreeNode* t) {

}
void endstatement(TreeNode* t) {

}
void readstatement(TreeNode* t)
{
	bool present = false;
	SymbTable* entry = nullptr;
	present = FindEntry(t->name[0], &entry);
	if (present == true)
	{
		//判断id是不是变量标识符
		if (entry->attrIR.kind != varKind)
			printf("%d %s 不是变量标识符\n ", t->lineno, t->name[0]), ERROR_sematicas = true;
	}
	else
	{
		ERROR_sematicas = true;
		cout << t->lineno << " " << t->name[0] << " 无声明" << endl;
	}
}
void writestatement(TreeNode* t)
{
	TypeIR* Etp = Expr(t->child[0], nullptr);
	if (Etp != nullptr)
		//如果表达式类型为bool类型，报错
		if (Etp->kind == boolTy)
		{
			ERROR_sematicas = true;
			printf("%d 表达式类型错误\n", t->lineno);
		}
}
void returnstatement(TreeNode* t)
{
	if (Level == 0) //如果返回语句出现在主程序中，报错
	{
		ERROR_sematicas = true;
		printf("%d 返回语句错误\n", t->lineno);
	}
}
bool Compat(TypeIR* tp1, TypeIR* tp2)//判断类型是否相容
{
	if (tp1 != tp2)return false;
	else
		return true;
}
TypeIR* NewTy(TypeKind  kind)//创建当前空类型内部表示
{
	TypeIR* table = new TypeIR;
	switch (kind)
	{
	case intTy:
	case charTy:
	case boolTy:
		table->kind = kind;
		table->size = 1;
		break;
	case arrayTy:
		table->kind = arrayTy;
		table->More.ArrayAttr.indexTy = nullptr;  //指向数组下标类型
		table->More.ArrayAttr.elemTy = nullptr;   //指向数组元素类型
		break;
	case recordTy:
		table->kind = recordTy;
		table->More.body = nullptr;
		break;
	}
	return table;
}
AttributeIR FindAttr(SymbTable* entry)//属性查询
{
	AttributeIR attrIr = entry->attrIR;
	return attrIr;
}