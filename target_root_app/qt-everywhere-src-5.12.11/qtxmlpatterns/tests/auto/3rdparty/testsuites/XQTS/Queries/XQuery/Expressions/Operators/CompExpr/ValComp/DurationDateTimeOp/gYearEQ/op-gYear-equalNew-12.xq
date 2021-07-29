(:*******************************************************:)
(:Test: op-gYear-equal-12                                :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "gYear-equal" function used     :)
(:together with "fn:true"/or expression (ne operator).   :)
(:*******************************************************:)
 
(xs:gYear("2000Z") ne xs:gYear("2000Z")) or (fn:true())