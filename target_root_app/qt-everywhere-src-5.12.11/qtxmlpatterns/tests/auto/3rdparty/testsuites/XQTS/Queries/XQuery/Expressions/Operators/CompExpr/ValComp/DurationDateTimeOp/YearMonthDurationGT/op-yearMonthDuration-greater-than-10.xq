(:*******************************************************:)
(:Test: op-yearMonthDuration-greater-than-10             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "yearMonthDuration-greater-than" function used  :)
(:together with "or" expression (ge operator).           :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P10Y01M") ge xs:yearMonthDuration("P09Y06M")) or (xs:yearMonthDuration("P15Y01M") ge xs:yearMonthDuration("P02Y04M"))