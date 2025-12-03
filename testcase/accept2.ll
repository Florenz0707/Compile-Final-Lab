declare i32 @getint()

declare i32 @getch()

declare i32 @getarray(i32*)

declare void @putint(i32)

declare void @putch(i32)

declare void @putarray(i32, i32*)

declare void @starttime()

declare void @stoptime()

define i32 @main() {
label_main_ENTRY:
  %op0 = alloca i32
  %op1 = alloca i32
  %op2 = alloca i32
  store i32 15, i32* %op0
  store i32 4, i32* %op1
  %op3 = load i32, i32* %op0
  %op4 = load i32, i32* %op1
  %op5 = mul i32 %op4, 2
  %op6 = add i32 %op3, %op5
  %op7 = sdiv i32 10, 5
  %op8 = srem i32 %op7, 3
  %op9 = sub i32 %op6, %op8
  store i32 %op9, i32* %op2
  %op10 = load i32, i32* %op2
  %op11 = icmp sgt i32 %op10, 20
  br i1 %op11, label %label12, label %label14
label12:                                                ; preds = %label_main_ENTRY
  %op13 = load i32, i32* %op2
  ret i32 %op13
label14:                                                ; preds = %label_main_ENTRY
  ret i32 0
}
