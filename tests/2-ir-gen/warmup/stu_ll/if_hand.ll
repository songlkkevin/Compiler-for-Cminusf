define dso_local i32 @main() #0{
    %1 = alloca float
    store float 0x40163851E0000000, float* %1
    %2 = load float, float* %1
    %3 = sitofp i32 1 to float
    %4 = fcmp ugt float %2, %3
    br i1 %4, label %5, label %6
5:
    ret i32 233
6:
    ret i32  0
}