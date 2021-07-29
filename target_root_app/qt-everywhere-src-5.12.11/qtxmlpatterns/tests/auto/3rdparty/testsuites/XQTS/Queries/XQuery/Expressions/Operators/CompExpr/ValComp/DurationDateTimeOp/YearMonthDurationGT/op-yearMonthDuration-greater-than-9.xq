(:*******************************************************:)
(:Test: op-yearMonthDuration-greater-than-9               :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "yearMonthDuration-greater-than" function used:)
(:together with "or" expression (gt operator).           :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P10Y08M") gt xs:yearMonthDuration("P10Y07M")) or (xs:yearMonthDuration("P10Y09M") gt xs:yearMonthDuration("P10Y09M"))