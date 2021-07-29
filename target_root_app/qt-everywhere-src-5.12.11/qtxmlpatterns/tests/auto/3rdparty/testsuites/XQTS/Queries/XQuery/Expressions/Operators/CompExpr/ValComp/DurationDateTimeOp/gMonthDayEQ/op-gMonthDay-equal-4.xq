(:*******************************************************:)
(:Test: op-gMonthDay-equal-4                             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 22, 2005                                    :)
(:Purpose: Evaluates The "gMonthDay-equal" function that :)
(:return true and used together with fn:not (ne operator):)
(:*******************************************************:)
 
fn:not(xs:gMonthDay("--05-01Z") ne xs:gMonthDay("--06-12Z"))