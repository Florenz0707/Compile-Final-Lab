declare i32 @getint()

declare i32 @getch()

declare i32 @getarray(i32*)

declare void @putint(i32)

declare void @putch(i32)

declare void @putarray(i32, i32*)

declare void @starttime()

declare void @stoptime()

define i32 @add_numbers(i32 %arg0, i32 %arg1) {
label_add_numbers_ENTRY:
  %op2 = alloca i32
  store i32 %arg0, i32* %op2
  %op3 = alloca i32
  store i32 %arg1, i32* %op3
  %op4 = alloca i32
  %op5 = load i32, i32* %op2
  %op6 = load i32, i32* %op3
  %op7 = add i32 %op5, %op6
  store i32 %op7, i32* %op4
  %op8 = load i32, i32* %op4
  ret i32 %op8
}
define void @main() {
label_main_ENTRY:
  %op0 = alloca i32
  store i32 5, i32* %op0
  %op1 = alloca i32
  store i32 7, i32* %op1
  %op2 = alloca i32
  %op3 = load i32, i32* %op0
  %op4 = load i32, i32* %op1
  %op5 = call i32 @add_numbers(i32 %op3, i32 %op4)
  store i32 %op5, i32* %op2
  %op6 = load i32, i32* %op2
  %op7 = icmp eq i32 %op6, 12
  br i1 %op7, label %label8, label %label9
label8:                                                ; preds = %label_main_ENTRY
  ret void
label9:                                                ; preds = %label_main_ENTRY
  ret void
}
