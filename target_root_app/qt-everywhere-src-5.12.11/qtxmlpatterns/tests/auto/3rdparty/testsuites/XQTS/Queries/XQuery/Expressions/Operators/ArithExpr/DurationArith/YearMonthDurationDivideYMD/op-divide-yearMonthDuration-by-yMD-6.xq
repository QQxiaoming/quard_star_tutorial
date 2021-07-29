(:*******************************************************:)
(:Test: op-divide-yearMonthDuration-by-YMD-6             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-yearMonthDuration-by-YMD" operator that :)
(:is used as an argument to the fn:number function.      :)
(:*******************************************************:)
 
fn:number(xs:yearMonthDuration("P02Y09M") div xs:yearMonthDuration("P02Y09M"))