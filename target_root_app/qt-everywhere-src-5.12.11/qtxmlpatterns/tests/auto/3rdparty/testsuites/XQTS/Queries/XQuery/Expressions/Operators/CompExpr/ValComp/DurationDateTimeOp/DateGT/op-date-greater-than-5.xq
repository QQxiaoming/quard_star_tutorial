(:*******************************************************:)
(:Test: op-date-greater-than-5                           :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "date-greater-than" function that:)
(:return false and used together with fn:not (gt operator):)
(:*******************************************************:)
 
fn:not(xs:date("2000-11-09Z") gt xs:date("2000-11-10Z"))