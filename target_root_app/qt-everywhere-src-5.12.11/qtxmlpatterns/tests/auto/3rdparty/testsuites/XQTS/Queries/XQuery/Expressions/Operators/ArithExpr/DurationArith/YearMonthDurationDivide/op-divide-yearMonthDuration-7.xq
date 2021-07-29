(:*******************************************************:)
(:Test: op-divide-yearMonthDuration-7                    :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-yearMonthDuration" operator used  :)
(:as an argument to the "fn:string" function).           :)
(:*******************************************************:)
 
fn:string(xs:yearMonthDuration("P03Y08M") div 2.0)