(:*******************************************************:)
(:Test: op-yearMonthDuration-less-than-10                :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "yearMonthDuration-less-than" function used  :)
(:together with "or" expression (le operator).           :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P10Y01M") le xs:yearMonthDuration("P09Y06M")) or (xs:yearMonthDuration("P15Y01M") le xs:yearMonthDuration("P02Y04M"))