@PI = constant float 3.140000
declare i32 @getint()

declare i32 @getch()

declare i32 @getarray(i32*)

declare void @putint(i32)

declare void @putch(i32)

declare void @putarray(i32, i32*)

declare void @starttime()

declare void @stoptime()

define void @calculate_area(float %arg0) {
label_calculate_area_ENTRY:
  %op1 = alloca float
  store float %arg0, float* %op1
  %op2 = alloca float
  %op3 = load float, float* @PI
  %op4 = load float, float* %op1
  %op5 = fmul float %op3, %op4
  %op6 = load float, float* %op1
  %op7 = fmul float %op5, %op6
  store float %op7, float* %op2
  %op8 = load float, float* %op2
  %op9 = fcmp sge float %op8, 3.000000
  br i1 %op9, label %label12, label %label15
label10:                                                ; preds = %label15
  ret void
label11:                                                ; preds = %label15
  ret void
label12:                                                ; preds = %label_calculate_area_ENTRY
  %op13 = load float, float* %op2
  %op14 = fcmp sle float %op13, 3.200000
  br label %label15
label15:                                                ; preds = %label_calculate_area_ENTRY, %label12
  %op16 = phi i1 [ false, %label_calculate_area_ENTRY ], [ %op14, %label12 ]
  br i1 %op16, label %label10, label %label11
}
define void @main() {
label_main_ENTRY:
  %op0 = alloca float
  store float 1.000000, float* %op0
  %op1 = load float, float* %op0
  call void @calculate_area(float %op1)
  %op2 = alloca float
  store float 10.000000, float* %op2
  %op3 = load float, float* %op2
  %op4 = load float, float* %op0
  %op5 = fcmp ne float %op3, %op4
  br i1 %op5, label %label6, label %label8
label6:                                                ; preds = %label_main_ENTRY
  %op7 = load float, float* %op2
  call void @calculate_area(float %op7)
  br label %label8
label8:                                                ; preds = %label_main_ENTRY, %label6
  ret void
}
