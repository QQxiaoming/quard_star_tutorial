(:*******************************************************:)
(:Test: op-gMonth-equal-3                                :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "gMonth-equal" function that    :)
(:return true and used together with fn:not (eq operator):)
(:*******************************************************:)
 
fn:not((xs:gMonth("--12Z") eq xs:gMonth("--12Z")))