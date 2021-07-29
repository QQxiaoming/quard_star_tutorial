(:*******************************************************:)
(:Test: op-dayTimeDuration-less-than-10                  :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "dayTimeDuration-less-than" function used  :)
(:together with "or" expression (le operator).           :)
(:*******************************************************:)
 
(xs:dayTimeDuration("P10DT01H") le xs:dayTimeDuration("P09DT06H")) or (xs:dayTimeDuration("P15DT01H") le xs:dayTimeDuration("P02DT04H"))