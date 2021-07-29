(:*******************************************************:)
(:Test: op-dayTimeDuration-less-than-8                   :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "dayTimeDuration-less-than" function used  :)
(:together with "and" expression (le operator).          :)
(:*******************************************************:)
 
(xs:dayTimeDuration("P10DT09H") le xs:dayTimeDuration("P10DT01H")) and (xs:dayTimeDuration("P02DT04H") le xs:dayTimeDuration("P09DT07H"))