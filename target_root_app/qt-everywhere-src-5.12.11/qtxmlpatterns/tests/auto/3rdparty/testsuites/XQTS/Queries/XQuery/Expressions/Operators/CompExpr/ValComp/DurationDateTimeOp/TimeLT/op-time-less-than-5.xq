(:*******************************************************:)
(:Test: op-time-less-than-5                              :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "time-less-than" function that  :)
(:return false and used together with fn:not (lt operator):)
(:*******************************************************:)
 
fn:not(xs:time("13:00:00Z") lt xs:time("10:00:00Z"))