(:*******************************************************:)
(:Test: op-divide-yearMonthDuration-by-YMD-5             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-yearMonthDuration-by-YMD" operator that  :)
(:is used as an argument to the fn:boolean function.     :)
(:*******************************************************:)
 
fn:boolean(xs:yearMonthDuration("P05Y08M") div xs:yearMonthDuration("P03Y06M"))