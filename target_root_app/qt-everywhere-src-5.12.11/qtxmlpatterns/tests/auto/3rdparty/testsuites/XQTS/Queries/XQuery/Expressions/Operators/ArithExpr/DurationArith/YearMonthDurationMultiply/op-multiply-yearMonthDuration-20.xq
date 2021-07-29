(:*******************************************************:)
(:Test: op-multiply-yearMonthDuration-20                 :)
(:Written By: Michael Kay                                :)
(:Date: 5 Feb 2009                                       :)
(:Purpose: Test rounding behaviour, FO.E12.              :)
(:*******************************************************:)

for $i in (-3.9, -3.5, -3.1, -0.9, -0.5, -0.1, +0.1, +0.5, +0.9, +3.1, +3.5, +3.9) 
return xs:yearMonthDuration("P1M") * $i