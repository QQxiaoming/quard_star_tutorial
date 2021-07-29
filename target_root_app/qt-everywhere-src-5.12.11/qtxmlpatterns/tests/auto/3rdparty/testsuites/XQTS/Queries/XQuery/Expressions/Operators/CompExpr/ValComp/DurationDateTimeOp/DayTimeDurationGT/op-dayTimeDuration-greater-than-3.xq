(:*******************************************************:)
(:Test: op-dayTimeDuration-greater-than-3                :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 17, 2005                                    :)
(:Purpose: Evaluates The "dayTimeDuration-greater-than" function that:)
(:return true and used together with fn:not (gt operator):)
(:*******************************************************:)
 
fn:not((xs:dayTimeDuration("P15DT12H") gt xs:dayTimeDuration("P14DT11H")))