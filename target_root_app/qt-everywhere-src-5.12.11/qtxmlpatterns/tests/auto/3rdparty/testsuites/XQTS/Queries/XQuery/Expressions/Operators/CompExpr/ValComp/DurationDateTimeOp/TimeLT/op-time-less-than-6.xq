(:*******************************************************:)
(:Test: op-time-less-than-6                              :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "time-less-than" function that  :)
(:return false and used together with fn:not(le operator):)
(:*******************************************************:)
 
fn:not(xs:time("13:00:00Z") le xs:time("12:00:00Z"))