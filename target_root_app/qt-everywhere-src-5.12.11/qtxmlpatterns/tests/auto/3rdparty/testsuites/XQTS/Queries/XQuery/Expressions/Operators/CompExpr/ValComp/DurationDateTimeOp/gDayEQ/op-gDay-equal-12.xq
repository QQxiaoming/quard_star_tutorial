(:*******************************************************:)
(:Test: op-gDay-equal-12                                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "gDay-equal" function used      :)
(:together with "fn:true"/or expression (ne operator).   :)
(:*******************************************************:)
 
(xs:gDay("---08Z") ne xs:gDay("---07Z")) or (fn:true())