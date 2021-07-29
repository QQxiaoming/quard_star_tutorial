(:*******************************************************:)
(:Test: op-dayTimeDuration-less-than-3                   :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 17, 2005                                    :)
(:Purpose: Evaluates The "dayTimeDuration-less-than" function that  :)
(:return true and used together with fn:not (lt operator):)
(:*******************************************************:)
 
fn:not((xs:dayTimeDuration("P13DT12H") lt xs:dayTimeDuration("P14DT11H")))