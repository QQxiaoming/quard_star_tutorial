(:*******************************************************:)
(:Test: op-date-less-than-5                              :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "date-less-than" function that  :)
(:return false and used together with fn:not (lt operator):)
(:*******************************************************:)
 
fn:not(xs:date("2000-12-25Z") lt xs:date("2000-11-25Z"))