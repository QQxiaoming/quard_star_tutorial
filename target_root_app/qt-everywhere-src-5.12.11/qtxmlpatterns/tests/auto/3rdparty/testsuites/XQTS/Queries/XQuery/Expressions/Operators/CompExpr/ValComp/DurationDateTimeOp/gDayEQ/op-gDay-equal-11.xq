(:*******************************************************:)
(:Test: op-gDay-equal-11                                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "gDay-equal" function used      :)
(:together with "fn:true"/or expression (eq operator).   :)
(:*******************************************************:)
 
(xs:gDay("---03Z") eq xs:gDay("---01Z")) or (fn:true())