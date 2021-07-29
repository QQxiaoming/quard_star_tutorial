(:*******************************************************:)
(:Test: op-gDay-equal-10                                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "gDay-equal" function used      :)
(:together with "or" expression (ne operator).           :)
(:*******************************************************:)
 
(xs:gDay("---06Z") ne xs:gDay("---06Z")) or (xs:gDay("---08Z") ne xs:gDay("---09Z"))