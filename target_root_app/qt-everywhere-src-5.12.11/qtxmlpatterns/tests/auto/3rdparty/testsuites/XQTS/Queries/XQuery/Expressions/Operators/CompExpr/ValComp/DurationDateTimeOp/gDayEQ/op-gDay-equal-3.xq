(:*******************************************************:)
(:Test: op-gDay-equal-3                                  :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "gDay-equal" function that      :)
(:return true and used together with fn:not (eq operator):)
(:*******************************************************:)
 
fn:not((xs:gDay("---12Z") eq xs:gDay("---12Z")))