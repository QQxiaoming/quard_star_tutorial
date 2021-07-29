(:*******************************************************:)
(:Test: op-gDay-equal-7                                  :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 16, 2005                                    :)
(:Purpose: Evaluates The "gDay-equal" function used      :)
(:together with "and" expression (eq operator).          :)
(:*******************************************************:)
 
(xs:gDay("---04Z") eq xs:gDay("---02Z")) and (xs:gDay("---01Z") eq xs:gDay("---12Z"))