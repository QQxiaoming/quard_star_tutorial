(:*******************************************************:)
(:Test: op-dayTimeDuration-greater-than-10               :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "dayTimeDuration-greater-than" function used :)
(:together with "or" expression (ge operator).           :)
(:*******************************************************:)
 
(xs:dayTimeDuration("P10DT01H") ge xs:dayTimeDuration("P09DT06H")) or (xs:dayTimeDuration("P15DT01H") ge xs:dayTimeDuration("P02DT04H"))