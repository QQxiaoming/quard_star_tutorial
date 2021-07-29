(:*******************************************************:)
(:Test: op-dayTimeDuration-less-than-6                   :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "dayTimeDuration-less-than" function that  :)
(:return false and used together with fn:not(le operator):)
(:*******************************************************:)
 
fn:not(xs:dayTimeDuration("P10DT09H") le xs:dayTimeDuration("P09DT09H"))