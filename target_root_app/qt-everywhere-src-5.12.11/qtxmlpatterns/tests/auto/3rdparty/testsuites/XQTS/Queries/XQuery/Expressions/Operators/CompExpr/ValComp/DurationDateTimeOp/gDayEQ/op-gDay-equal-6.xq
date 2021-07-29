(:*******************************************************:)
(:Test: op-gDay-equal-6                                  :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 16, 2005                                    :)
(:Purpose: Evaluates The "gDay-equal" function that      :)
(:return false and used together with fn:not(ne operator):)
(:*******************************************************:)
 
fn:not(xs:gDay("---05Z") ne xs:gDay("---05Z"))