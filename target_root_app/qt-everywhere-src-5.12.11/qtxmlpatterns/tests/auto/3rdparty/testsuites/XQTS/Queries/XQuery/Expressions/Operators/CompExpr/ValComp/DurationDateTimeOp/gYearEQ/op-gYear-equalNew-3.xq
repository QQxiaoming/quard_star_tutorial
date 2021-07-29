(:*******************************************************:)
(:Test: op-gYear-equal-3                                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "gYear-equal" function that     :)
(:return true and used together with fn:not (eq operator):)
(:*******************************************************:)
 
fn:not((xs:gYear("1995Z") eq xs:gYear("1995Z")))