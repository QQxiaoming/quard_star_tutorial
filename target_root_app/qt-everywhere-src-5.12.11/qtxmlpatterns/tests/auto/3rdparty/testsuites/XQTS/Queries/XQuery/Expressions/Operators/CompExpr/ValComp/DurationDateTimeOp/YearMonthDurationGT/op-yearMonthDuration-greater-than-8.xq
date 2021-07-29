(:*******************************************************:)
(:Test: op-yearMonthDuration-greater-than-8              :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "yearMonthDuration-greater-than" function used  :)
(:together with "and" expression (ge operator).          :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P10Y09M") ge xs:yearMonthDuration("P10Y01M")) and (xs:yearMonthDuration("P02Y04M") ge xs:yearMonthDuration("P09Y07M"))