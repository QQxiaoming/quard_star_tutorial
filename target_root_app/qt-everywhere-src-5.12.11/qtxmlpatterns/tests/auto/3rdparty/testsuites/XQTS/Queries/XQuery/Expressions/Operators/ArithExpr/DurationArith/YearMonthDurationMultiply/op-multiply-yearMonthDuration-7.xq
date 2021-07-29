(:*******************************************************:)
(:Test: op-multiply-yearMonthDuration-7                  :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "multiply-yearMonthDuration" function used  :)
(:as an argument to the "fn:string" function).           :)
(:*******************************************************:)
 
fn:string(xs:yearMonthDuration("P03Y08M") * 2.0)