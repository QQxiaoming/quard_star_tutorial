(:*******************************************************:)
(:Test: op-gMonth-equal-6                                :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 16, 2005                                    :)
(:Purpose: Evaluates The "gMonth-equal" function that    :)
(:return false and used together with fn:not(ne operator):)
(:*******************************************************:)
 
fn:not(xs:gMonth("--05Z") ne xs:gMonth("--05Z"))