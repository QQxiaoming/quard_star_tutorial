(:*******************************************************:)
(:Test: op-gDay-equal-14                                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "gDay-equal" function used      :)
(:together with "fn:false"/or expression (ne operator).  :)
(:*******************************************************:)
 
(xs:gDay("---09Z") ne xs:gDay("---09Z")) or (fn:false())