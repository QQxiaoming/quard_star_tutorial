(:*******************************************************:)
(:Test: op-time-less-than-4                              :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "time-less-than" function that  :)
(:return true and used together with fn:not (le operator):)
(:*******************************************************:)
 
fn:not(xs:time("13:00:00Z") le xs:time("14:00:00Z"))