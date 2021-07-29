(:*******************************************************:)
(:Test: op-yearMonthDuration-less-than-8                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "yearMonthDuration-less-than" function used  :)
(:together with "and" expression (le operator).          :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P10Y09M") le xs:yearMonthDuration("P10Y01M")) and (xs:yearMonthDuration("P02Y04M") le xs:yearMonthDuration("P09Y07M"))