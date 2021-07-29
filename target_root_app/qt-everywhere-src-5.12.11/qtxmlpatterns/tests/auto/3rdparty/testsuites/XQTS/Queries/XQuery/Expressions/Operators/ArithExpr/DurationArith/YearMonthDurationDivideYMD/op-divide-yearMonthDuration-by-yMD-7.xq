(:*******************************************************:)
(:Test: op-divide-yearMonthDuration-by-YMD-7             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-yearMonthDuration-by-YMD" operator used  :)
(:as an argument to the "fn:string" function).           :)
(:*******************************************************:)
 
fn:string(xs:yearMonthDuration("P03Y08M") div xs:yearMonthDuration("P03Y08M"))