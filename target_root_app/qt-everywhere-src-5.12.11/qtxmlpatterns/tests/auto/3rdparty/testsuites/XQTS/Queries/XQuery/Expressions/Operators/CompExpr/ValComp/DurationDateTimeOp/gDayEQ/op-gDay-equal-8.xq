(:*******************************************************:)
(:Test: op-gDay-equal-8                                  :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 16, 2005                                    :)
(:Purpose: Evaluates The "gDay-equal" function used      :)
(:together with "and" expression (ne operator).          :)
(:*******************************************************:)
 
(xs:gDay("---12Z") ne xs:gDay("---03Z")) and (xs:gDay("---05Z") ne xs:gDay("---08Z"))