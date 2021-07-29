(:*******************************************************:)
(:Test: op-divide-yearMonthDuration-6                    :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-yearMonthDuration" operator that :)
(:is used as an argument to the fn:number function.      :)
(:*******************************************************:)
 
fn:number(xs:yearMonthDuration("P02Y09M") div 2.0)