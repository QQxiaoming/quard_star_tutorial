(:*******************************************************:)
(:Test: op-divide-yearMonthDuration-3                    :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-yearMonthDuration" operator as :)
(:part of a boolean expression (or operator) and the "fn:boolean" function. :)
(:*******************************************************:)
 
fn:string((xs:yearMonthDuration("P20Y10M") div 2.0)) or fn:false()