(:*******************************************************:)
(:Test: op-divide-yearMonthDuration-by-YMD-3             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-yearMonthDuration-by-YMD" operator as :)
(:part of a boolean expression (or operator) and the "fn:boolean" function. :)
(:*******************************************************:)
 
xs:yearMonthDuration("P20Y10M") div xs:yearMonthDuration("P19Y10M") or fn:false()