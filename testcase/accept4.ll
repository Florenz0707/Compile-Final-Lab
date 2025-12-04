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
  store i32 1, i32* %op0
  %op1 = alloca i32
  store i32 10, i32* %op1
  %op2 = load i32, i32* %op0
  %op3 = icmp eq i32 %op2, 1
  br i1 %op3, label %label12, label %label15
label4:                                                ; preds = %label15
  %op5 = alloca float
  store float 5.500000, float* %op5
  %op6 = load float, float* %op5
  %op7 = fcmp slt float %op6, 5.000000
  br i1 %op7, label %label22, label %label19
label8:                                                ; preds = %label15
  %op9 = load i32, i32* %op1
  %op10 = icmp slt i32 %op9, 0
  br i1 %op10, label %label23, label %label25
label11:                                                ; preds = %label18, %label25
  ret i32 2
label12:                                                ; preds = %label_main_ENTRY
  %op13 = load i32, i32* %op1
  %op14 = icmp sge i32 %op13, 10
  br label %label15
label15:                                                ; preds = %label_main_ENTRY, %label12
  br i1 %, label %label4, label %label8
label16:                                                ; preds = %label22
  ret i32 0
label17:                                                ; preds = %label22
  ret i32 1
label18:
  br label %label11
label19:                                                ; preds = %label4
  %op20 = load float, float* %op5
  %op21 = fcmp sgt float %op20, 6.000000
  br label %label22
label22:                                                ; preds = %label4, %label19
  br i1 %, label %label16, label %label17
label23:                                                ; preds = %label8
  %op24 = sub i32 0, 1
  ret i32 %op24
label25:                                                ; preds = %label8
  br label %label11
}
