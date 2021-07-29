(:*******************************************************:)
(:Test: op-divide-yearMonthDuration-by-YMD-8             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-yearMonthDuration-by-YMD" operator that  :)
(:returns a negative value.                              :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P10Y01M") div xs:yearMonthDuration("-P10Y01M"))