(:*******************************************************:)
(:Test: op-gMonth-equal-5                                :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "gMonth-equal" function that    :)
(:return false and used together with fn:not (eq operator):)
(:*******************************************************:)
 
fn:not(xs:gMonth("--11Z") eq xs:gMonth("--10Z"))