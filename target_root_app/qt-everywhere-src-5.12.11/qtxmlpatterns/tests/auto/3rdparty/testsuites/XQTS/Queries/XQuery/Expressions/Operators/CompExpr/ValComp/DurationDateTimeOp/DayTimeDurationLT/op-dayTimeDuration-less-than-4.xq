(:*******************************************************:)
(:Test: op-dayTimeDuration-less-than-4                   :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "dayTimeDuration-less-than" function that  :)
(:return true and used together with fn:not (le operator):)
(:*******************************************************:)
 
fn:not(xs:dayTimeDuration("P10DT110H") le xs:dayTimeDuration("P10DT11H"))