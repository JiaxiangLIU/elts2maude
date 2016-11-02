; ModuleID = 'Pure3Phase_true-termination.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %x = alloca i32, align 4
  %y = alloca i32, align 4
  %z = alloca i32, align 4
  store i32 0, i32* %retval
  %call = call i32 @__VERIFIER_nondet_int()
  store i32 %call, i32* %x, align 4
  %call1 = call i32 @__VERIFIER_nondet_int()
  store i32 %call1, i32* %y, align 4
  %call2 = call i32 @__VERIFIER_nondet_int()
  store i32 %call2, i32* %z, align 4
  br label %while.cond

while.cond:                                       ; preds = %if.end, %entry
  %0 = load i32, i32* %x, align 4
  %cmp = icmp sge i32 %0, 0
  br i1 %cmp, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %call3 = call i32 @__VERIFIER_nondet_int()
  %cmp4 = icmp ne i32 %call3, 0
  br i1 %cmp4, label %if.then, label %if.else

if.then:                                          ; preds = %while.body
  %1 = load i32, i32* %x, align 4
  %2 = load i32, i32* %y, align 4
  %add = add nsw i32 %1, %2
  store i32 %add, i32* %x, align 4
  br label %if.end

if.else:                                          ; preds = %while.body
  %3 = load i32, i32* %x, align 4
  %4 = load i32, i32* %z, align 4
  %add5 = add nsw i32 %3, %4
  store i32 %add5, i32* %x, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %5 = load i32, i32* %y, align 4
  %6 = load i32, i32* %z, align 4
  %add6 = add nsw i32 %5, %6
  store i32 %add6, i32* %y, align 4
  %7 = load i32, i32* %z, align 4
  %sub = sub nsw i32 %7, 1
  store i32 %sub, i32* %z, align 4
  br label %while.cond

while.end:                                        ; preds = %while.cond
  ret i32 0
}

declare i32 @__VERIFIER_nondet_int() #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.7.0 (tags/RELEASE_370/final)"}
