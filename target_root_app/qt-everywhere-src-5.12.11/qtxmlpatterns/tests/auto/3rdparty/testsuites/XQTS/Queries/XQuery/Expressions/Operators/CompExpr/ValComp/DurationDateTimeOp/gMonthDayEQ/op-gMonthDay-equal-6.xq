(:*******************************************************:)
(:Test: op-gMonthDay-equal-6                             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 22, 2005                                    :)
(:Purpose: Evaluates The "gMonthDay-equal" function that :)
(:return false and used together with fn:not(ne operator):)
(:*******************************************************:)
 
fn:not(xs:gMonthDay("--05-10Z") ne xs:gMonthDay("--05-10Z"))