declare i32 @getint()

declare i32 @getch()

declare i32 @getarray(i32*)

declare void @putint(i32)

declare void @putch(i32)

declare void @putarray(i32, i32*)

declare void @starttime()

declare void @stoptime()

define void @main() {
label_main_ENTRY:
  %op0 = alloca i32
  %op1 = alloca i32
  %op2 = alloca float
  store float 3.140000, float* %op2
  store i32 10, i32* %op0
  %op3 = load i32, i32* %op0
  %op4 = mul i32 5, %op3
  store i32 %op4, i32* %op1
  %op5 = alloca i32
  %op6 = load i32, i32* %op0
  %op7 = load i32, i32* %op1
  %op8 = add i32 %op6, %op7
  store i32 %op8, i32* %op5
  %op9 = load i32, i32* %op5
  %op10 = icmp sge i32 %op9, 60
  br i1 %op10, label %label11, label %label12
label11:                                                ; preds = %label_main_ENTRY
  ret void
label12:                                                ; preds = %label_main_ENTRY
  %op13 = alloca i32
  %op14 = load i32, i32* %op1
  %op15 = srem i32 %op14, 10
  store i32 %op15, i32* %op13
  %op16 = alloca float
  %op17 = load i32, i32* %op0
  %op18 = sitofp i32 %op17 to float
  %op19 = fdiv float %op18, 2.000000
  store float %op19, float* %op16
  %op20 = load float, float* %op16
  %op21 = load float, float* %op2
  %op22 = fcmp ne float %op20, %op21
  br i1 %op22, label %label24, label %label25
label23:                                                ; preds = %label25
  ret void
label24:                                                ; preds = %label12
  ret void
label25:                                                ; preds = %label12
  br label %label23
}
