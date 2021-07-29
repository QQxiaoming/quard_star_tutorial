(:*******************************************************:)
(:Test: op-divide-yearMonthDuration-by-YMD-10            :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-yearMonthDuration-by-YMD" operator used  :)
(:together with an "or" expression.                      :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P05Y02M") div xs:yearMonthDuration("P03Y04M")) or (xs:yearMonthDuration("P05Y03M") div xs:yearMonthDuration("P01Y03M"))