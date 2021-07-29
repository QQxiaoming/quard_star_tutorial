(:*******************************************************:)
(:Test: op-yearMonthDuration-less-than-7                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "yearMonthDuration-less-than" function used  :)
(:together with "and" expression (lt operator).          :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P10Y09M") lt xs:yearMonthDuration("P09Y10M")) and (xs:yearMonthDuration("P10Y01M") lt xs:yearMonthDuration("P08Y06M"))