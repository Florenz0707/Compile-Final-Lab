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
  %op0 = alloca float
  store float 0.050000, float* %op0
  %op1 = alloca float
  store float 1000.000000, float* %op1
  %op2 = alloca float
  %op3 = alloca i32
  store i32 5, i32* %op3
  %op4 = load float, float* %op1
  %op5 = load float, float* %op0
  %op6 = fmul float %op4, %op5
  %op7 = load i32, i32* %op3
  %op8 = sitofp i32 %op7 to float
  %op9 = fmul float %op6, %op8
  store float %op9, float* %op2
  %op10 = load float, float* %op2
  %op11 = fcmp eq float %op10, 250.000000
  br i1 %op11, label %label12, label %label19
label12:                                                ; preds = %label_main_ENTRY
  %op13 = alloca float
  %op14 = load float, float* %op1
  %op15 = load float, float* %op2
  %op16 = fadd float %op14, %op15
  store float %op16, float* %op13
  %op17 = load float, float* %op13
  %op18 = fcmp sle float %op17, 1250.000000
  br i1 %op18, label %label22, label %label23
label19:                                                ; preds = %label_main_ENTRY, %label23
  %op20 = load float, float* %op1
  %op21 = fcmp slt float %op20, 999.989990
  br i1 %op21, label %label29, label %label26
label22:                                                ; preds = %label12
  ret void
label23:                                                ; preds = %label12
  br label %label19
label24:                                                ; preds = %label29
  ret void
label25:                                                ; preds = %label29
  ret void
label26:                                                ; preds = %label19
  %op27 = load i32, i32* %op3
  %op28 = icmp ne i32 %op27, 5
  br label %label29
label29:                                                ; preds = %label19, %label26
  br i1 %, label %label24, label %label25
}
