grammar Basic;

body:(globalModule=moduleMember)* EOF;

classDecl: Class name=ID genericDecl? (Extend (implements+=typeLocation(','implements+=typeLocation)*))?
            (classMember)* End Class;
classMember: controlFlag=(Public|Private)? Static?
                (importDecl|aliasDecl|functionDecl|propertyDecl|varDecl|typeDecl|enumDecl|factoryDecl|operatorOverride);

moduleDecl: ModuleInfo name=ID genericDecl? genericDecl? (moduleMember)* End ModuleInfo;
moduleMember: controlFlag=(Public|Private)? Static?
                (importDecl|aliasDecl|functionDecl|varDecl|typeDecl|enumDecl|moduleDecl|classDecl);

aliasDecl: Alias name=ID genericDecl? '=' typeLocation;

enumDecl: Enum name=ID (enumPair)* End Enum;

operatorOverride:Function Operator op=('+'|'-'|'*'|'\\'|'/'|Clone) parameterList As returnType=typeLocation (Implements implements=typeLocation)?
            block+=line* End Function  ;

importDecl: Import typeLocation;

enumPair: name=ID ('=' value=constExp)?;

factoryDecl:Factory name=ID parameterList block+=line* End Factory;

typeDecl:Type name=ID genericDecl? (nameTypePair)* End Type  ;

varDecl: varFlag=(Dim|Static) variable (','variable)*  ;

variable: nameTypePair ('=' initial=exp)?;

functionDecl: Function name=ID genericDecl? parameterList As returnType=typeLocation (Override override=typeLocation?)? block+=line* End Function
            | Sub name=ID genericDecl? parameterList (Override override=typeLocation?)? block+=line* End Sub
            | Declare Function name=ID (Lib libPath=String)? (Alias aliasName=String)? parameterList As returnType=typeLocation
            | Declare Sub name=ID (Lib libPath=String)? (Alias aliasName=String)? parameterList
            ;

propertyDecl:Property name=ID As propertyType=typeLocation (getter? setter?|setter? getter?) End Property;
getter: Get (block+=line*) End Get;
setter: Set (block+=line*) End Set;

parameterList:'(' (necessaryParameter (','necessaryParameter)*?)? (','optionalParameter)*? (','paramArrayParameter)? ')';
necessaryParameter: passFlag=(Byref|Byval)? nameTypePair ;
optionalParameter: Optional passFlag=(Byref|Byval)? nameTypePair ('=' initial=constExp)?;
paramArrayParameter: ParamArray nameTypePair;

nameTypePair: name=ID (As typeLocation)?                         #NormalNameTypePair
            | name=ID '['(size=constExp)?']' (As typeLocation)?       #ArrayNameTypePair
            ;

typeLocation: (path+=type'.')*?target=type
            | lambdaType
            ;
type: ID generic?;

lambdaType : Function '(' nameTypePair(','nameTypePair)* ')' As typeLocation
           | Sub'('nameTypePair(','nameTypePair)*')'
           ;

line:statement;

statement:forStmt
        |foreachStmt
        |loopStmt
        |ifStmt
        |exitStmt
        |returnStmt
        |assignStmt
        |varDecl
        |select
        |exceptionHandling
        |exp
        ;

select:Select exp matchCase* (Case Else elseBlock+=line*)? End Select;
matchCase:Case matchExp block+=line*;
matchExp: exp '..' exp     #RangeMatchExp
        | matchTerminal(':'matchExp)+ #SliceMatchExp
        | '['(matchTerminal(','matchExp)*)?']'  #ArrayMatchExp
        | '('matchTerminal(','matchExp)*')'  #TrupleMatchExp
        | matchTerminal                      #TerminalMatchExp
        ;
matchTerminal: name=ID (As typeLocation)? #DefineInMatch
             | constExp                   #MatchFactor
             ;

exceptionHandling: Try block+=line* catchHandling+ End Try;
catchHandling: Catch nameTypePair block+=line*;

passArg:value=exp                #ArgPassValue
       |option=ID ':' value=exp  #ArgOptional
       ;


assignStmt: left=ID '=' right=exp  ;

exitStmt:Exit exitFlag=(For|Do|Sub|Function|Property)  ;

returnStmt:Return exp  ;

constExp: exp;

exp: '-' right=exp                                                  #NegExp
    | left=exp As right=typeLocation                                #CastExp
    | (path+=terminateNode '.')+ target=exp                         #RefExp
    | left=exp (leftShift|rightShift|andBit|orBit|xorBit) right=exp #BitExp
    | '~' right=exp                                                 #BitNotExp
    | left=exp '%' right=exp                                        #PowModExp
    | left=exp op=('*'|'/'|'\\')    right=exp                       #MulExp //   /普通除法  \整除
    | left=exp op=('+'|'-')         right=exp                       #PluExp
    | left=exp (eqCmp|ltCmp|gtCmp|leCmp|geCmp|neCmp) right=exp additionCmp*  #CmpExp
    | op='not' right=exp                                            #LogicNotExp
    | left=exp op=('and'|'or'|'xor') right=exp                      #LogicExp
    | '('exp')'                                                     #BucketExp
    | terminateNode                                                 #TerminateNodeExp
    ;

additionCmp: op=('and'|'or')(eqCmp|ltCmp|gtCmp|leCmp|geCmp|neCmp)right=exp;

leftShift:'<''<';
rightShift:'>''>';
andBit:'&';
orBit:'|';
xorBit:'^';
eqCmp:'=''=';
ltCmp:'<';
gtCmp:'>';
leCmp:'=''<'|'<''=';
geCmp:'>''=';
neCmp:'<''>';


keyValuePair: nameTypePair ':' value=exp;

terminateNode: Integer                                                      #Integer
            | Decimal                                                       #Decimal
            | string                                                        #Strings
            | Character                                                     #Character
            | Boolean                                                       #Boolean
            | target=ID generic? '(' (passArg (','passArg?)*)? ')'          #FunctionCall
            | ID generic?                                                   #TargetExp
            | '{'keyValuePair(','keyValuePair)*'}'                          #MapExp
            | '['exp(','exp)*']'                                            #ArrayExp
            | '('exp(','exp)*')'                                            #TupleExp
            | array=ID '['index=exp']'                                      #ArrayOffset
            | (nameTypePair|'('nameTypePair(','nameTypePair)* ')') '=>' statement #Lambda
            ;

genericDecl: '<' terminateNode (','terminateNode)* '>';
generic: '<'exp(','exp)*'>';


foreachStmt: For Each (iterator=ID|Dim nameTypePair) In group=exp block+=line* Next nextFlag=ID? ;
forStmt: For (iterator=ID|Dim nameTypePair) '=' begin=exp To end=exp (Step step=exp)? block+=line* Next nextFlag=ID?;

ifStmt: If condition=exp Then statement (Else elseStatement=statement)?           #SingleLineIf
        | If ifBlock (ElseIf ifBlock)* (Else elseBlock+=line*)? End If    #MutiLineIf
        ;

ifBlock: condition=exp Then block+=line* ;

loopStmt: While exp block+=line* Loop    #DoWhile
        | Until exp block+=line* Loop    #DoUntil
        | Do block+=line* Loop Until exp #LoopUntil
        | Do block+=line* Loop While exp #LoopWhile
        | While exp block+=line* Wend    #LoopWhile
        ;

string: String|RowText;

String:'"' ~('"'|'\r'|'\n')* '"';
RowText:'"""' .* '"""';
Character:'\'' .* '\'';

Integer: [0-9]+; //i32
Decimal: [0-9]+'.'[0-9]+ | [0-9]+('E'|'e')'-'?[0-9]+;//f64
Boolean: T R U E | F A L S E;

Comment: '\'' ~('\r'|'\n')*  -> skip;
BlockComment: '\'*' .*? '*\'' -> skip;
LineEnd: [\n\r]->skip;
WS: [ \t]->skip;

Try: T R Y;
Catch:C A T C H;
Override:O V E R R I D E;
Operator:O P E R A T O R;
Clone: C L O N E;
Factory:F A C T O R Y;
Implements:I M P L E M E N T S;
Import:I M P O R T;
Class:C L A S S;
Preserve:P R E S E R V E;
Redim:R E D I M;
ParamArray:P A R A M A R R A Y;
Declare:D E C L A R E;
Lib:L I B;
Enum:E N U M;
If:I F;
ElseIf:E L S E I F;
Wend:W E N D;
From:F O R M;
Namespace:N A M E S P A C E;
Implement:I M P L E M E N T;
Type: T Y P E;
Alias:A L I A S;
Self:S E L F;
Static:S T A T I C;
ModuleInfo:M O D U L E;
Public:P U B L I C;
Private:P R I V A T E;
Protected:P R O T E C T E D;
Get:G E T;
Set:S E T;
Property:P R O P E R T Y;
Var:V A R;
Dim:D I M;
Let:L E T;
Return: R E T U R N;
Function:F U N C T I O N;
Difference: D I F F E R E N C E;
Union: U N I O N;
Case:C A S E;
Select:S E L E C T;
End:E N D;
Until:U N T I L;
Loop:L O O P;
Exit:E X I T;
While: W H I L E;
Do: D O;
Each: E A C H;
To: T O;
Step:S T E P;
Next: N E X T;
In: I N;
For: F O R;
Optional: O P T I O N A L;
Byval:B Y V A L;
Byref:B Y R E F;
Then:T H E N;
Else:E L S E;
Call:C A L L;
Sub:S U B;
As: A S;
ID: [a-zA-Z_][a-zA-Z0-9_]*;

fragment A:('a'|'A');
fragment B:('b'|'B');
fragment C:('c'|'C');
fragment D:('d'|'D');
fragment E:('e'|'E');
fragment F:('f'|'F');
fragment G:('g'|'G');
fragment H:('h'|'H');
fragment I:('i'|'I');
fragment J:('j'|'J');
fragment K:('k'|'K');
fragment L:('l'|'L');
fragment M:('m'|'M');
fragment N:('n'|'N');
fragment O:('o'|'O');
fragment P:('p'|'P');
fragment Q:('q'|'Q');
fragment R:('r'|'R');
fragment S:('s'|'S');
fragment T:('t'|'T');
fragment U:('u'|'U');
fragment V:('v'|'V');
fragment W:('w'|'W');
fragment X:('x'|'X');
fragment Y:('y'|'Y');
fragment Z:('z'|'Z');

