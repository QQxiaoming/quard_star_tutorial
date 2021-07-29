(:*******************************************************:)
(:Test: op-dayTimeDuration-greater-than-7                :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "dayTimeDuration-greater-than" function used :)
(:together with "and" expression (gt operator).          :)
(:*******************************************************:)
 
(xs:dayTimeDuration("P10DT09H") gt xs:dayTimeDuration("P09DT10H")) and (xs:dayTimeDuration("P10DT01H") gt xs:dayTimeDuration("P08DT06H"))