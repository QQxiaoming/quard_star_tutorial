(:*******************************************************:)
(:Test: op-time-greater-than-3                           :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "time-greater-than" function that:)
(:return true and used together with fn:not (gt operator):)
(:*******************************************************:)
 
fn:not((xs:time("14:00:00Z") gt xs:time("13:00:00Z")))