(:*******************************************************:)
(:Test: op-dayTimeDuration-greater-than-4                :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "dayTimeDuration-greater-than" function that:)
(:return true and used together with fn:not (ge operator):)
(:*******************************************************:)
 
fn:not(xs:dayTimeDuration("P10DT11H") ge xs:dayTimeDuration("P10DT10H"))