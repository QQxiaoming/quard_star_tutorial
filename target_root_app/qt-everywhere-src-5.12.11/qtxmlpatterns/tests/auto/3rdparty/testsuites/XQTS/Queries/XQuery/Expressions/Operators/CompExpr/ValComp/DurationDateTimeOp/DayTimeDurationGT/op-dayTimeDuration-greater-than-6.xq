(:*******************************************************:)
(:Test: op-dayTimeDuration-greater-than-6                :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "dayTimeDuration-greater-than" function that :)
(:return false and used together with fn:not(ge operator):)
(:*******************************************************:)
 
fn:not(xs:dayTimeDuration("P07DT09H") ge xs:dayTimeDuration("P09DT09H"))