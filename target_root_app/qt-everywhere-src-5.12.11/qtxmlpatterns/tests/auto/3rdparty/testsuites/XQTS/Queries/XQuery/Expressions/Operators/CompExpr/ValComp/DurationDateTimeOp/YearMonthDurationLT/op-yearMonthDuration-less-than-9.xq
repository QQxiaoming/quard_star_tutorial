(:*******************************************************:)
(:Test: op-yearMonthDuration-less-than-9                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "yearMonthDuration-less-than" function used:)
(:together with "or" expression (lt operator).           :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P10Y08M") lt xs:yearMonthDuration("P10Y07M")) or (xs:yearMonthDuration("P10Y09M") lt xs:yearMonthDuration("P10Y09M"))