(:*******************************************************:)
(:Test: op-time-greater-than-4                           :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "time-greater-than" function that:)
(:return true and used together with fn:not (ge operator):)
(:*******************************************************:)
 
fn:not(xs:time("13:00:00Z") ge xs:time("10:00:00Z"))