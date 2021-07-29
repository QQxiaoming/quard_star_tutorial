(:*******************************************************:)
(:Test: op-gDay-equal-13                                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "gDay-equal" function used      :)
(:together with "fn:false"/or expression (eq operator).  :)
(:*******************************************************:)
 
(xs:gDay("---05Z") eq xs:gDay("---05Z")) or (fn:false())