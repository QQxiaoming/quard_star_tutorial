(:*******************************************************:)
(:Test: op-divide-yearMonthDuration-by-YMD-2             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-yearMonthDuration-by-YMD" operator :)
(:used as part of a boolean expression (and operator) and the "fn:false" function. :)
(:*******************************************************:)

xs:yearMonthDuration("P10Y11M") div xs:yearMonthDuration("P12Y07M") and fn:false()