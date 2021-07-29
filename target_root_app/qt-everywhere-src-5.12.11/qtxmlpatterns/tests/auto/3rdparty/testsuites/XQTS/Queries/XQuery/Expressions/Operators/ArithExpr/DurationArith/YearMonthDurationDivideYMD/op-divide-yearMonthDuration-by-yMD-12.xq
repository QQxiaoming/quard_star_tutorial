(:*******************************************************:)
(:Test: op-divide-yearMonthDuration-by-YMD-12            :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-yearMonthDuration-by-YMD" operator used :)
(:with a boolean expression and the "fn:true" function.   :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P10Y11M") div xs:yearMonthDuration("P05Y07M")) and (fn:true())