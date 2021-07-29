(:*******************************************************:)
(:Test: op-dayTimeDuration-less-than-5                   :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "dayTimeDuration-less-than" function that  :)
(:return false and used together with fn:not (lt operator):)
(:*******************************************************:)
 
fn:not(xs:dayTimeDuration("P10DT10H") lt xs:dayTimeDuration("P9DT09H"))