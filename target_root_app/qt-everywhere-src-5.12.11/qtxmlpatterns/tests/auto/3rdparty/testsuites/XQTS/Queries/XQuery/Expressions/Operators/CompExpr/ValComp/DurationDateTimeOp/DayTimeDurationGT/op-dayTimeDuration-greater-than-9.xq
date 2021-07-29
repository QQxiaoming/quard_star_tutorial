(:*******************************************************:)
(:Test: op-dayTimeDuration-greater-than-9                :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "dayTimeDuration-greater-than" function used:)
(:together with "or" expression (gt operator).           :)
(:*******************************************************:)
 
(xs:dayTimeDuration("P10DT08H") gt xs:dayTimeDuration("P10DT07H")) or (xs:dayTimeDuration("P10DT09H") gt xs:dayTimeDuration("P10DT09H"))