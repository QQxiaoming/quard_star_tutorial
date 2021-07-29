(:*******************************************************:)
(:Test: op-gYear-equal-6                                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "gYear-equal" function that     :)
(:return false and used together with fn:not(ne operator):)
(:*******************************************************:)
 
fn:not(xs:gYear("2005Z") ne xs:gYear("2005Z"))