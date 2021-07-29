(:*******************************************************:)
(:Test: op-dayTimeDuration-greater-than-8                :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "dayTimeDuration-greater-than" function used :)
(:together with "and" expression (ge operator).          :)
(:*******************************************************:)
 
(xs:dayTimeDuration("P10DT09H") ge xs:dayTimeDuration("P10DT01H")) and (xs:dayTimeDuration("P02DT04H") ge xs:dayTimeDuration("P09DT07H"))