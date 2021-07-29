(:*******************************************************:)
(:Test: op-divide-yearMonthDuration-by-YMD-9             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-yearMonthDuration-by-YMD" operator used :)
(:together with and "and" expression.                    :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P01Y01M") div xs:yearMonthDuration("P02Y02M")) and (xs:yearMonthDuration("P02Y03M") div xs:yearMonthDuration("P04Y04M"))