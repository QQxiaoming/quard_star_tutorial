(:*******************************************************:)
(:Test: op-dayTimeDuration-less-than-7                   :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "dayTimeDuration-less-than" function used  :)
(:together with "and" expression (lt operator).          :)
(:*******************************************************:)
 
(xs:dayTimeDuration("P10DT09H") lt xs:dayTimeDuration("P09DT10H")) and (xs:dayTimeDuration("P10DT01H") lt xs:dayTimeDuration("P08DT06H"))