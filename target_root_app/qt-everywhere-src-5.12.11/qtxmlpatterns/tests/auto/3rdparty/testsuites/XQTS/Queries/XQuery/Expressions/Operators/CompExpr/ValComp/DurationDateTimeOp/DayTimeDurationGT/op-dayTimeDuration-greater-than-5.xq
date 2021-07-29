(:*******************************************************:)
(:Test: op-dayTimeDuration-greater-than-5                :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "dayTimeDuration-greater-than" function that :)
(:return false and used together with fn:not (gt operator):)
(:*******************************************************:)
 
fn:not(xs:dayTimeDuration("P08DT10H") gt xs:dayTimeDuration("P9DT09H"))