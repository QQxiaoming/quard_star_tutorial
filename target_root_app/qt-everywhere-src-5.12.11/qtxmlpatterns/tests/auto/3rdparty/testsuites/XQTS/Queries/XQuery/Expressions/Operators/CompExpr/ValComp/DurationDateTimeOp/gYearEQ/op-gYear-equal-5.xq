(:*******************************************************:)
(:Test: op-gYear-equal-5                                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "gYear-equal" function that     :)
(:return false and used together with fn:not (eq operator):)
(:*******************************************************:)
 
fn:not(xs:gYear("2000Z") eq xs:gYear("2001Z"))