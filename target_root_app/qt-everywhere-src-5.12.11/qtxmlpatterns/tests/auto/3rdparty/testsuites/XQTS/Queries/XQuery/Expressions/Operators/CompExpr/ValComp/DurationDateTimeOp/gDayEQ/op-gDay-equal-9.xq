(:*******************************************************:)
(:Test: op-gDay-equal-9                                  :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "gDay-equal" function used      :)
(:together with "or" expression (eq operator).           :)
(:*******************************************************:)
 
(xs:gDay("---02Z") eq xs:gDay("---02Z")) or (xs:gDay("---06Z") eq xs:gDay("---06Z"))