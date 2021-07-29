(:*******************************************************:)
(:Test: op-yearMonthDuration-greater-than-7              :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "yearMonthDuration-greater-than" function used :)
(:together with "and" expression (gt operator).          :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P10Y09M") gt xs:yearMonthDuration("P09Y10M")) and (xs:yearMonthDuration("P10Y01M") gt xs:yearMonthDuration("P08Y06M"))