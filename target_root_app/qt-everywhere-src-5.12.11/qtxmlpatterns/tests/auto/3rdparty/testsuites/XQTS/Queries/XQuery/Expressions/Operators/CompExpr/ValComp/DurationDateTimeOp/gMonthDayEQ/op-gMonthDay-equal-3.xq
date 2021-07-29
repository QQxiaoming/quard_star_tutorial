(:*******************************************************:)
(:Test: op-gMonthDay-equal-3                             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 22, 2005                                    :)
(:Purpose: Evaluates The "gMonthDay-equal" function that :)
(:return true and used together with fn:not (eq operator):)
(:*******************************************************:)
 
fn:not((xs:gMonthDay("--12-10Z") eq xs:gMonthDay("--12-10Z")))