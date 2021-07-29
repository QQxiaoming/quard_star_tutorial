(:*******************************************************:)
(:Test: op-gDay-equal-5                                  :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "gDay-equal" function that      :)
(:return false and used together with fn:not (eq operator):)
(:*******************************************************:)
 
fn:not(xs:gDay("---11Z") eq xs:gDay("---10Z"))